#pragma once
#include <vector>
#include <functional>
#include <Elaina/Elaina.config.h>

namespace Elaina
{
   class FrameResource;
   class FrameGraph;
   class FrameGraphBuilder;

   class RenderPass
   {
   public:
      RenderPass(const StringType& name) :
         Name(name),
         RefCount(0)
      {
      }

      virtual ~RenderPass()
      {
         for (FrameResource* ownResource : Creates)
         {
            SafeDelete(ownResource);
         }
      }

      StringType GetName() const { return Name; }

      size_t GetReferenceCount() const { return RefCount; }
      bool IsCulled() const { return (RefCount == 0); }

   protected:
      /** Create resource handles */
      virtual void Setup(FrameGraphBuilder& builder) = 0;
      /** Perfom actual rendering operations */
      virtual void Execute() const = 0;

   private:
      /** Calculate Reference count */
      void OnCompile() { RefCount = (Creates.size() + Writes.size()); }

   private:
      StringType Name;

      std::vector<FrameResource*> Creates;
      std::vector<FrameResource*> Reads;
      std::vector<FrameResource*> Writes;
      size_t RefCount;

      friend FrameGraph;
      friend FrameGraphBuilder;

   };
}