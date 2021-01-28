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
      static_cast(MissingRealizeImplementation<DescriptorType, ActualType>::value, "�־��� Ÿ�Կ� ���� Realize�� ������ ã�� �� �����ϴ�.");
      return nullptr;
   }
}