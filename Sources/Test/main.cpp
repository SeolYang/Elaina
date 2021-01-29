#include <Elaina/FrameGraph.h>
#include <Elaina/FrameResource.h>

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

int main()
{
   auto* realized = Elaina::Realize<IntDescriptor, int>({ 5 });
   Elaina::SafeDelete(realized);

   // auto* missingRealized = Elaina::Realize<FloatDescriptor, float>({ 3.0f }); /** Missing impelemntation example */
   // Elaina::SafeDelete(missingRealized);

   return 0;
}