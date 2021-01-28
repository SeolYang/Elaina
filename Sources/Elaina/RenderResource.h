#pragma once
#include <vector>
#include <Elaina/Elaina.config.h>

namespace Elaina
{
   class RenderPass;
   class FrameGraph;
   class FrameGraphBuilder;

   template <typename DescriptorType, typename ActualType>
   class RenderResource
   {
   public:
      /** Deferred-created resources */
      explicit RenderResource(const StringType& name, RenderPass* creator, const DescriptorType& descriptor) :
         Descriptor(descriptor),
         RenderResource(name, creator)
      {
      }

      /** External Permanent resources */
      explicit RenderResource(const StringType& name, DescriptorType& description, ActualType* actual) :
         Descriptor(descriptor),
         Actual(actual),
         RenderResource(name, nullptr)
      {
      }

      virtual ~RenderResource()
      {
         /** Delete 'actual' only if it is transient resource and deferred created(or realized). */
         if (IsTransient())
         {
            SafeDelete(Actual);
         }
      }

      RenderPass* GetCreator() const { return Creator; }
      std::vector<RenderPass*>& GetReaders() { return Readers; }
      std::vector<RenderPass*>& GetWriters() { return Writers; }

      size_t GetID() const { return Identifier; }
      StringType GetName() const { return Name; }

      size_t GetRefCount() const { return RefCount; }
      size_t Referencing() { ++RefCount; return RefCount; }
      size_t DeReferencing() { --RefCount; return RefCount; }

      DescriptorType GetDescriptor() const { return Descriptor; }
      ActualType* GetActual() const { return Actual; }

      bool IsTransient() const { return (Creator != nullptr); }
      bool IsRealized() const { return (Actual != nullptr); }
      bool IsExternalPermanent() const { return (!IsTransient() && IsRealized()); }

   private:
      explicit Resource(const StringType& name, RenderPass* creator) :
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
            // @todo 실제 리소스 오브젝트(ex. ID3D11Texture2D / ID3D11RenderTarget 등등.. 또는 래핑 클래스들)
            RealizeImpl();
         }
      }

      void DeRealize()
      {
         if (IsTransient() && IsRealized())
         {
            SafeDelete(Actual);
         }
      }

   private:
      size_t      Identifier;
      StringType  Name;

      RenderPass* Creator;
      std::vector<RenderPass*> Readers;
      std::vector<RenderPass*> Writers;
      size_t RefCount;

      DescriptorType Descriptor;
      ActualType* Actual;

   };
}