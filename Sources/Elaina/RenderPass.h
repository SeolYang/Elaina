#pragma once
#include <vector>
#include <functional>
#include <Elaina/Elaina.config.h>

namespace Elaina
{
   class FrameResourceBase;
   class FrameGraph;
   class RenderPassBuilder;

   class RenderPass
   {
   public:
      RenderPass(const StringType& name) :
         Name(name),
         RefCount(0)
      {
      }

      virtual ~RenderPass() = default;

      StringType GetName() const { return Name; }

      size_t GetReferenceCount() const { return RefCount; }
      bool IsCulled() const { return (RefCount == 0); }

   protected:
      /** Create resource handles */
      virtual void Setup(RenderPassBuilder& builder) = 0;
      /** Perfom actual rendering operations */
      virtual void Execute() = 0;

   private:
      /** Calculate Reference count */
      void OnCompile() { RefCount = (Creates.size() + Writes.size()); }

   private:
      StringType Name;

      std::vector<FrameResourceBase*> Creates;
      std::vector<FrameResourceBase*> Reads;
      std::vector<FrameResourceBase*> Writes;
      size_t RefCount;

      friend FrameGraph;
      friend RenderPassBuilder;

   };
}