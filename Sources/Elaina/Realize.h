#pragma once
#include <Elaina/Elaina.config.h>

namespace Elaina
{
   template <typename DescriptorType, typename ActualType>
   struct MissingRealizeImplementation : std::false_type
   {
      /** static assert�� �̿��� ���� ��Ȯ�� compile time error */
   };

   template <typename DescriptorType, typename ActualType>
   ActualType* Realize(const DescriptorType& descriptor)
   {
      static_assert(MissingRealizeImplementation<DescriptorType, ActualType>::value, "Missing realize implementation for given descriptor type and actual type!");
      return nullptr;
   }
}