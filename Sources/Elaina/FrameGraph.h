#pragma once
#include <algorithm>
#include <stack>
#include <fstream>
#include <Elaina/FrameResource.h>
#include <Elaina/RenderPass.h>
#include <Elaina/CallbackRenderPass.h>
#include <Elaina/RenderPassBuilder.h>

namespace Elaina
{
   /** Execute 단계에서 실제로 실행될 렌더 패스와 할당 또는 할당 해제될 리소스들의 리스트 */
   struct RenderPhase
   {
      RenderPhase() : RenderPass(nullptr)
      {
      }

      Elaina::RenderPass* RenderPass;
      std::vector<FrameResourceBase*> ToRealize;
      std::vector<FrameResourceBase*> ToDerealize;
   };

   class FrameGraph
   {
   public:
      FrameGraph() = default;
      virtual ~FrameGraph()
      {
         Clear();
      }

      /** Setup */
      template <typename RenderPassType, typename... Args>
      RenderPassType* AddRenderPass(Args&&... args)
      {
         auto* newRenderPass = new RenderPassType(args...);
         RenderPasses.push_back(newRenderPass);

         RenderPassBuilder builder(this, newRenderPass);
         newRenderPass->Setup(builder);

         return newRenderPass;
      }

      template <typename DataType, typename... Args>
      CallbackRenderPass<DataType>* AddCallbackPass(Args&&... args)
      {
         auto* newRenderPass = new CallbackRenderPass<DataType>(args...);
         RenderPasses.push_back(newRenderPass);

         RenderPassBuilder builder(this, newRenderPass);
         newRenderPass->Setup(builder);

         return newRenderPass;
      }

      template <typename DescriptorType, typename ActualType>
      auto AddExternalPermanentResource(const StringType& name, const DescriptorType& descriptor, ActualType* actual)
      {
         auto newResource = new FrameResource<DescriptorType, ActualType>(name, descriptor, actual);
         Resources.push_back(newResource);
         return newResource;
      }

      /** Compile */
      void Compile()
      {
         /** Calculate Reference Count */
         for (auto renderPass : RenderPasses)
         {
            renderPass->OnCompile();
         }
         for (auto resource : Resources)
         {
            resource->OnCompile();
         }

         /** Culling resources and render pass */
         std::stack<FrameResourceBase*> expiredResources;
         for (auto resource : Resources)
         {
            if (resource->IsExpired())
            {
               expiredResources.push(resource);
            }
         }

         while (!expiredResources.empty())
         {
            auto expiredResource = expiredResources.top();
            expiredResources.pop();

            auto creator = expiredResource->GetCreator();
            if (!creator->IsNeedToCull())
            {
               --creator->RefCount;
            }

            /** Render pass expired! */
            if (creator->IsNeedToCull())
            {
               /** Cull 처리 될 render pass가 참조하던 리소스의 레퍼런스 카운트를 감소시켜준다 */
               for (auto readResource : creator->Reads)
               {
                  if(readResource->RefCount > 0) /** Do not include external permanent */
                  {
                     --readResource->RefCount;
                  }

                  if (readResource->IsExpired())
                  {
                     expiredResources.push(readResource);
                  }
               }
            }

            for (auto writer : expiredResource->Writers)
            {
               if (!writer->IsNeedToCull())
               {
                  --writer->RefCount;
               }

               if (writer->IsNeedToCull())
               {
                  /** Cull 처리 될 render pass가 참조하던 리소스의 레퍼런스 카운트를 감소시켜준다 */
                  for (auto readResource : creator->Reads)
                  {
                     if (readResource->RefCount > 0) /** Do not include external permanent */
                     {
                        --readResource->RefCount;
                     }

                     if (readResource->IsExpired())
                     {
                        expiredResources.push(readResource);
                     }
                  }
               }
            }
         }

         /** Construct render phases (Find Realized/Derealized resources each render pass) */
         Phases.clear();
         for (auto targetRenderPass : RenderPasses)
         {
            if (!targetRenderPass->IsNeedToCull())
            {
               RenderPhase newPhase;
               newPhase.RenderPass = targetRenderPass;

               /** Always realize create resource in its own render pass. */
               for (auto resource : targetRenderPass->Creates)
               {
                  newPhase.ToRealize.push_back(resource);
                  if (resource->Readers.empty() && resource->Writers.empty())
                  {
                     /** Immediately derealize own resource after render pass excuted if there a no more any referencing by other render pass. */
                     newPhase.ToDerealize.push_back(resource);
                  }
               }

               /** For resources which read and write by current render pass. */
               auto readsAndWrites = targetRenderPass->Reads;
               readsAndWrites.insert(
                  readsAndWrites.end(),
                  targetRenderPass->Writes.begin(), targetRenderPass->Writes.end());

               /** Greedy selection */
               for (auto resource : readsAndWrites)
               {
                  if (resource->IsTransient())
                  {
                     bool bFoundLastReader = false;
                     size_t lastIndex = 0;
                     if (!resource->Readers.empty())
                     {
                        for (auto renderPassItr = RenderPasses.begin(); renderPassItr != RenderPasses.end(); ++renderPassItr)
                        {
                           if ((*renderPassItr) == resource->Readers.back())
                           {
                              bFoundLastReader = true;
                              lastIndex = std::distance(RenderPasses.begin(), renderPassItr);
                           }
                        }
                     }

                     bool bFoundLastWriters = false;
                     if (!resource->Writers.empty())
                     {
                        for (auto renderPassItr = RenderPasses.begin(); renderPassItr != RenderPasses.end(); ++renderPassItr)
                        {
                           if ((*renderPassItr) == resource->Writers.back())
                           {
                              bFoundLastWriters = true;
                              lastIndex = std::max(
                                 lastIndex,
                                 (size_t)std::distance(RenderPasses.begin(), renderPassItr));
                           }
                        }
                     }

                     /** @TODO MORE OPTIMIZE(한번만 어느시점에서 최종적으로 deserialize 될지는 한번만 계산해도됨 */
                     bool bNeedToDerealizeOnCurrent = targetRenderPass == RenderPasses[lastIndex];
                     if ((bFoundLastWriters || bFoundLastReader) && bNeedToDerealizeOnCurrent)
                     {
                        newPhase.ToDerealize.push_back(resource);
                     }
                  }
               }

               Phases.push_back(std::move(newPhase));
            }
         }
      }

      /** Execute */
      void Execute()
      {
         for (auto phase : Phases)
         {
            /* Realize resource */
            for (auto resource : phase.ToRealize)
            {
               resource->Realize();
            }

            phase.RenderPass->Execute();

            /* Derealize Resource*/
            for (auto resource : phase.ToDerealize)
            {
               resource->Derealize();
            }
         }
      }

      void Clear()
      {
         for (auto renderPass : RenderPasses)
         {
            SafeDelete(renderPass);
         }

         for (auto resource : Resources)
         {
            SafeDelete(resource);
         }

         RenderPasses.clear();
         Resources.clear();
         Phases.clear();
      }

      /** 
      * @param filePath Output path
      * @param bgColor  Background color; "#rgb" | "H S V" | "ColorNmae" (ex. White is "#ffffff" as hexadecimal or "0.0 0.0 1.0" or "White" (https://graphviz.org/doc/info/colors.html)
      * @param fontName Output font name (https://graphviz.org/doc/info/attrs.html#d:fontname)
      * @param fontSize Output font size
      * @param renderPassNodeColor     RenderPass node color
      * @param transientResNodeColor   Trasient Resource node color
      * @param externalResNodeColor    External permanent Resource node color
      * @param createRefEdgeColor      RenderPass->Resource(Create)
      * @param writeRefEdgeColor       RenderPass->Resource(Write)
      * @param readRefEdgeColor        Resource->RenderPass(Read)
      * @graphviz https://sketchviz.com/graphviz-examples
      */
      void ExportVisualization(
         const std::string& filePath,
         bool bLeftToRight,
         const std::string& bgColor,
         const std::string& fontName, double fontSize,
         const std::string& renderPassNodeColor,
         const std::string& transientResNodeColor,
         const std::string& externalResNodeColor,
         const std::string& createRefEdgeColor,
         const std::string& writeRefEdgeColor,
         const std::string& readRefEdgeColor)
      {
         std::ofstream stream(filePath);

         stream << "diagraph FrameGraph \n{\n";

         stream << "rankdir = " << (bLeftToRight ? "LR" : "TB") << std::endl;;
         stream << "bgcolor = " << bgColor << std::endl;
         stream << "node [shape=rectangle, fontname=\"" << fontName << "\", fontsize=" << fontSize << "]" << std::endl << std::endl;

         /** Export Render Passes as graph nodes */
         for (auto renderPass : RenderPasses)
         {
            stream << "\"" << renderPass->GetName() <<
               "\" [label=\"" << renderPass->GetName() << std::endl
               << "Refs : " << renderPass->GetReferenceCount() <<
               "\", style=filled, fillcolor=" << renderPassNodeColor << "]" << std::endl;
         }
         stream << std::endl;

         /** Export Resources as graph nodes */
         for (auto resource : Resources)
         {
            stream << "\"" << resource->GetName() <<
               "\" [label=\"" << resource->GetName() << std::endl
               << "ID : " << resource->GetID() << std::endl
               << "Refs : " << resource->GetRefCount() 
               << "\", style=filled, fillcolor=" << (resource->IsTransient() ? transientResNodeColor : externalResNodeColor) << "]" << std::endl;
         }

         /** Export resource referencing relation as directed graph edge */
         /** RenderPass->Resource Referencing relations */
         for (auto renderPass : RenderPasses)
         {
            /** Create Resource */
            stream << "\"" << renderPass->GetName() << "\" -> { ";
            for (auto resource : renderPass->Creates)
            {
               stream << "\"" << resource->GetName() << "\" ";
            }
            stream << "} [color=" << createRefEdgeColor << "]" << std::endl;

            /** Write Resource */
            stream << "\"" << renderPass->GetName() << "\" -> { ";
            for (auto resource : renderPass->Writes)
            {
               stream << "\"" << resource->GetName() << "\" ";
            }
            stream << "} [color=" << writeRefEdgeColor << "]" << std::endl;
         }
         stream << std::endl;

         /** Resource->RenderPass Referencing relations */
         for (auto resource : Resources)
         {
            stream << "\"" << resource->GetName() << "\" -> { ";
            for (auto renderPass : resource->Readers)
            {
               stream << "\"" << renderPass->GetName() << "\" ";
            }
            stream << "} [color=" << readRefEdgeColor << "]" << std::endl;
         }

         stream << "}"; // End of diagraph FrameGraph
      }

   private:
      std::vector<RenderPass*> RenderPasses;
      std::vector<FrameResourceBase*> Resources;
      std::vector<RenderPhase> Phases;

      friend RenderPassBuilder;

   };

   template <typename ResourceType, typename DescriptorType>
   ResourceType* RenderPassBuilder::Create(const StringType& name, const DescriptorType& descriptor)
   {
      ResourceType* newResource = new ResourceType(name, TargetRenderPass, descriptor);
      TargetFrameGraph->Resources.push_back(newResource);
      TargetRenderPass->Creates.push_back(newResource);
      return newResource;
   }
}