#pragma once
#include <vector>
#include <Elaina/Elaina.config.h>
#include <Elaina/Realize.h>

namespace Elaina
{
   class RenderPass;
   class FrameGraph;
   class FrameGraphBuilder;

   template <typename DescriptorType, typename ActualType>
   class FrameResource
   {
   public:
      /** Deferred-created resources */
      explicit FrameResource(const StringType& name, RenderPass* creator, const DescriptorType& descriptor) :
         Descriptor(descriptor),
         FrameResource(name, creator)
      {
      }

      /** External Permanent resources */
      explicit FrameResource(const StringType& name, DescriptorType& descriptor, ActualType* actual) :
         Descriptor(descriptor),
         Actual(actual),
         FrameResource(name, nullptr)
      {
      }

      virtual ~FrameResource()
      {
         /** Delete 'actual' only if it is transient resource and deferred created(or realized). */
         DeRealize();
      }

      RenderPass* GetCreator() const { return Creator; }

      size_t GetID() const { return Identifier; }
      StringType GetName() const { return Name; }

      size_t GetRefCount() const { return RefCount; }

      DescriptorType GetDescriptor() const { return Descriptor; }
      ActualType* GetActual() const { return Actual; }

      bool IsTransient() const { return (Creator != nullptr); }
      bool IsRealized() const { return (Actual != nullptr); }
      bool IsExternalPermanent() const { return (!IsTransient() && IsRealized()); }

   private:
      explicit FrameResource(const StringType& name, RenderPass* creator) :
         Name(name),
         Creator(creator),
         RefCount(0),
         Descriptor(DescriptorType()),
         Actual(nullptr)
      {
         static size_t IdentifierCounter = 0;
         Identifier = IdentifierCounter;
         ++IdentifierCounter;
      }

      void Realize()
      {
         if (IsTransient() && !IsRealized())
         {
            Actual = Elaina::Realize<DescriptorType, ActualType>(Descriptor);
         }
      }

      void DeRealize()
      {
         if (IsTransient() && IsRealized())
         {
            SafeDelete(Actual);
         }
      }

      /** Calculate Reference Count */
      void OnCompile()
      {
         RefCount = Readers.size();
      }

   protected:
      DescriptorType Descriptor;
      ActualType* Actual;

   private:
      size_t      Identifier;
      StringType  Name;

      RenderPass* Creator;
      std::vector<RenderPass*> Readers;
      std::vector<RenderPass*> Writers;
      size_t RefCount;

      friend FrameGraph;

   };
}