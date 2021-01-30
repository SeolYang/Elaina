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
      RenderPass(const StringType& name, size_t distributionGroup = 0) :
         Name(name),
         RefCount(0),
         DistributionGroup(distributionGroup)
      {
      }

      virtual ~RenderPass() = default;

      StringType GetName() const { return Name; }

      size_t GetRefCount() const { return RefCount; }
      bool IsNeedToCull() const { return (RefCount == 0); }

      size_t GetDistributionGroup() const { return DistributionGroup; }

   protected:
      /**
      * @brief Create resource handles 
      */
      virtual void Setup(RenderPassBuilder& builder) = 0;

      /** 
      * @brief Perfom actual rendering operations
      */
      virtual void Execute() = 0;

   private:
      /** 
      * @brief Calculate Reference count
      * �ڽ��� ���� ���ҽ��� ����� ���ҽ����Լ��� �����ȴ�
      * (���ҽ��� RenderPass�� �����ؾ��ϴ� ���; RefCount = Resource�� ��)
      */
      void OnCompile() { RefCount = (Creates.size() + Writes.size()); }

   private:
      StringType Name;

      std::vector<FrameResourceBase*> Creates;
      std::vector<FrameResourceBase*> Reads;
      std::vector<FrameResourceBase*> Writes;
      size_t RefCount;

      size_t DistributionGroup;

      friend FrameGraph;
      friend RenderPassBuilder;

   };
}