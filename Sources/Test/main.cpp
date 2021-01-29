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

struct FloatIntAddPassData
{
   IntFrameResource* IntegerInput;
   FloatFrameResource* FloatInput;
   FloatFrameResource* Output;
};

int main()
{
#ifdef _DEBUG
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

   Elaina::FrameGraph frameGraph;

   float* output = new float();
   auto externalPermanentResource =
      frameGraph.AddExternalPermanentResource("Output", FloatDescriptor(), output);

   auto renderPass = frameGraph.AddCallbackPass<FloatIntAddPassData>(
      /** Render Pass Name */
      "AddPass",
      /** Setup callback */
      [&](Elaina::RenderPassBuilder& builder, FloatIntAddPassData& data)
      {
         data.IntegerInput = builder.Create<IntFrameResource>("Integer0", IntDescriptor{ 3 });
         data.FloatInput = builder.Create<FloatFrameResource>("Float0", FloatDescriptor{ 2.0f });
         data.Output = builder.Write(externalPermanentResource);
      },
      /** Execute callback  */
      [](const FloatIntAddPassData& data)
      {
         auto IntegerInputActual = data.IntegerInput->GetActual();
         auto FloatInputActual = data.FloatInput->GetActual();
         auto OutputActual = data.Output->GetActual();
         (*OutputActual) = static_cast<float>(*IntegerInputActual) + (*FloatInputActual);
      });

   auto data = renderPass->GetData();

   frameGraph.Compile();
   frameGraph.Execute();

   std::cout << (*data.Output->GetActual()) << std::endl;

   Elaina::SafeDelete(output);
   return 0;
}