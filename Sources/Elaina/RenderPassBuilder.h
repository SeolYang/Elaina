#pragma once
#include <Elaina/Elaina.config.h>

namespace Elaina
{
   class FrameGraph;
   class FrameResourceBase;
   class RenderPass;
   class RenderPassBuilder
   {
   public:
      RenderPassBuilder(FrameGraph* targetFrameGraph, RenderPass* targetRenderPass) :
         TargetFrameGraph(targetFrameGraph),
         TargetRenderPass(targetRenderPass)
      {
      }

      ~RenderPassBuilder() = default;

      template <typename ResourceType, typename DescriptorType>
      ResourceType* Create(const StringType& name, const DescriptorType& descriptor);

      template <typename ResourceType>
      ResourceType* Read(ResourceType* resource);

      template <typename ResourceType>
      ResourceType* Write(ResourceType* resource);

   private:
      FrameGraph* TargetFrameGraph;
      RenderPass* TargetRenderPass;

   };
}