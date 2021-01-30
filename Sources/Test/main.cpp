#include <iostream>
#include <Elaina/FrameGraph.h>
#include <Elaina/FrameResource.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

struct IntDescriptor
{
   int Value = 0;
};

using IntFrameResource = Elaina::FrameResource<IntDescriptor, int>;

struct FloatDescriptor
{
   float Value = 0.0f;
};

using FloatFrameResource = Elaina::FrameResource<FloatDescriptor, float>;

template<>
int* Elaina::Realize(const IntDescriptor& descriptor)
{
   return new int(descriptor.Value);
}

template<>
float* Elaina::Realize(const FloatDescriptor& descriptor)
{
   return new float(descriptor.Value);
}

struct DataCreatePassData
{
   IntFrameResource* IntegerOutput;
   FloatFrameResource* FloatOutput;
};

struct FloatIntBinaryOpPassData
{
   IntFrameResource* IntegerInput;
   FloatFrameResource* FloatInput;
   FloatFrameResource* Output;
};

struct TwoTimesPassData
{
   IntFrameResource* IntegerInput;
   FloatFrameResource* FloatInput;
   IntFrameResource* IntegerOutput;
   FloatFrameResource* FloatOutput;
};

int main()
{
#ifdef _DEBUG
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

   Elaina::FrameGraph frameGraph;

   auto createPass = frameGraph.AddCallbackPass<DataCreatePassData>(
      "CreateResourcePass",
      [](Elaina::RenderPassBuilder& builder, DataCreatePassData& data)
      {
         data.IntegerOutput = builder.Create<IntFrameResource>("Integer0", IntDescriptor{ 5 });
         data.FloatOutput = builder.Create<FloatFrameResource>("Float0", FloatDescriptor{ 3.0f });
      },
      [](const DataCreatePassData& data)
      {
         /** Nothing to do here */
      }, 0); /** Distribution Group : 0 */

   auto createdData = createPass->GetData();

   float* addOutput = new float();
   auto addOutputResource =
      frameGraph.AddExternalPermanentResource("Add Pass Output", FloatDescriptor(), addOutput);

   auto addPass = frameGraph.AddCallbackPass<FloatIntBinaryOpPassData>(
      /** Render Pass Name */
      "AddPass",
      /** Setup callback */
      [&](Elaina::RenderPassBuilder& builder, FloatIntBinaryOpPassData& data)
      {
         data.IntegerInput = builder.Read(createdData.IntegerOutput);
         data.FloatInput = builder.Read(createdData.FloatOutput);
         data.Output = builder.Write(addOutputResource);
      },
      /** Execute callback  */
      [](const FloatIntBinaryOpPassData& data)
      {
         auto IntegerInputActual = data.IntegerInput->GetActual();
         auto FloatInputActual = data.FloatInput->GetActual();
         auto OutputActual = data.Output->GetActual();
         (*OutputActual) = static_cast<float>(*IntegerInputActual) + (*FloatInputActual);
      }, 1);

   float* subOutput = new float();
   auto subOutputResource =
      frameGraph.AddExternalPermanentResource("Sub Pass Output", FloatDescriptor(), subOutput);

   auto subPass = frameGraph.AddCallbackPass<FloatIntBinaryOpPassData>(
      /** Render Pass Name */
      "SubtractPass",
      /** Setup callback */
      [&](Elaina::RenderPassBuilder& builder, FloatIntBinaryOpPassData& data)
      {
         data.IntegerInput = builder.Read(createdData.IntegerOutput);
         data.FloatInput = builder.Read(createdData.FloatOutput);
         data.Output = builder.Write(subOutputResource);
      },
      /** Execute callback  */
         [](const FloatIntBinaryOpPassData& data)
      {
         auto IntegerInputActual = data.IntegerInput->GetActual();
         auto FloatInputActual = data.FloatInput->GetActual();
         auto OutputActual = data.Output->GetActual();
         (*OutputActual) = static_cast<float>(*IntegerInputActual) - (*FloatInputActual);
      }, 1);

   auto addPassData = addPass->GetData();
   auto subPassData = subPass->GetData();

   auto twoTimesPass = frameGraph.AddCallbackPass<TwoTimesPassData>(
      "TwoTimesPass",
      [&](Elaina::RenderPassBuilder& builder, TwoTimesPassData& data)
      {
         /** Read/Write a resource in a pass */
         data.IntegerInput = builder.Read(createdData.IntegerOutput);
         data.FloatInput = builder.Read(createdData.FloatOutput);
         data.IntegerOutput = builder.Create<IntFrameResource>("Integer1", IntDescriptor{ 0 });
         data.FloatOutput = builder.Create<FloatFrameResource>("Float1", FloatDescriptor{ 0.0f });
      },
      [](const TwoTimesPassData& data)
      {
         auto integerInputActual = data.IntegerInput->GetActual();
         auto integerOutputActual = data.IntegerOutput->GetActual();
         auto floatIntputActual = data.FloatInput->GetActual();
         auto floatOutputActual = data.FloatOutput->GetActual();

         (*integerOutputActual) = (*integerInputActual) * 2;
         (*floatOutputActual) = (*floatIntputActual) * 2.0f;
      }, 0);

   frameGraph.Compile();
   frameGraph.Execute();

   std::cout << "Add Pass result : " << (*addPassData.Output->GetActual()) << std::endl;
   std::cout << "Sub Pass result : " << (*subPassData.Output->GetActual()) << std::endl;
   frameGraph.ExportVisualization({ "FrameGraph.dot", "nanumgothic bold" });
   frameGraph.Clear();

   Elaina::SafeDelete(addOutput);
   Elaina::SafeDelete(subOutput);
   return 0;
}