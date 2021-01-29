#pragma once
#include <Elaina/RenderPass.h>

namespace Elaina
{
   template <typename DataType>
   class CallbackRenderPass : public RenderPass
   {
      using SetupCallbackType = std::function<void(FrameGraphBuilder&, DataType&)>;
      using ExecuteCallbackType = std::function<void(const DtaType&)>;

   public:
      explicit CallbackRenderPass(const StringType& name, const SetupCallback& setupCallback, const ExecuteCallbackType& executeCallback) :
         SetupCallback(setupCallback),
         ExecuteCallback(executeCallback),
         RenderPass(name)
      {
      }

      virtual ~CallbackRenderPass() = default;

      const DataType& GetData() const { return Data; }

   protected:
      virtual void Setup(FrameGraphBuilder& builder) override
      {
         SetupCallback(builder, Data);
      }

      virtual void Execute() const override
      {
         ExecuteCallback(Data);
      }

   protected:
      DataType Data;
      const SetupCallbackType SetupCallback;
      const ExecuteCallbackType ExecuteCallback;

   };
}