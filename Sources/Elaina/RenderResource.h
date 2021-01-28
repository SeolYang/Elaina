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
      explicit RenderResource(const StringType& name, const RenderPass* creator, const DescriptorType& description) :
         Description(description),
         RenderResource(name, creator)
      {
      }

      /** External Permanent resources */
      explicit RenderResource(const StringType& name, const DescriptorType& description, ActualType* actual) :
         Description(description),
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

      const RenderPass* GetCreator() const { return Creator; }
      std::vector<const RenderPass*>& GetReaders() { return Readers; }
      std::vector<const RenderPass*>& GetWriters() { return Writers; }

      size_t GetID() const { return Identifier; }
      StringType GetName() const { return Name; }

      size_t GetRefCount() const { return RefCount; }
      size_t Referencing() { ++RefCount; return RefCount; }
      size_t DeReferencing() { --RefCount; return RefCount; }

      Description GetDescription() const { return Description; }
      ActualType* GetActual() const { return Actual; }

      bool IsTransient() const { return (Creator != nullptr); }
      bool IsRealized() const { return (Actual != nullptr); }
      bool IsExternalPermanent() const { return (!IsTransient() && IsRealized()); }

   private:
      explicit Resource(const StringType& name, const RenderPass* creator) :
         Name(name),
         Creator(creator),
         RefCount(0),
         Description(DescriptorType()),
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

      const RenderPass* Creator;
      std::vector<const RenderPass*> Readers;
      std::vector<const RenderPass*> Writers;
      size_t RefCount;

      DescriptorType Description;
      ActualType* Actual;

   };
}