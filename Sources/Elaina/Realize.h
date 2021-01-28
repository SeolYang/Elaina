#pragma once
#include <Elaina/Elaina.config.h>

namespace Elaina
{
   template <typename DescriptorType, typename ActualType>
   struct MissingRealizeImplementation : std::false_type
   {
      /** static assert를 이용한 더욱 정확한 compile time error */
   };

   template <typename DescriptorType, typename ActualType>
   ActualType* Realize(const DescriptorType& descriptor)
   {
      static_cast(MissingRealizeImplementation<DescriptorType, ActualType>::value, "주어진 타입에 대한 Realize의 구현을 찾을 수 없습니다.");
      return nullptr;
   }
}