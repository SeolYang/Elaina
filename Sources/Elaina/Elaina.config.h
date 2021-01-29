#pragma once
#include <string>

namespace Elaina
{
  //using StringType = std::wstring;
  using StringType = std::string;

  template <typename Type>
  static void SafeDelete(Type& pointer)
  {
     if (pointer != nullptr)
     {
        delete pointer;
        pointer = nullptr;
     }
  }

  template <typename Type>
  static void SafeDeleteArray(Type& pointer)
  {
     if (pointer != nullptr)
     {
        delete[] pointer;
        pointer = nullptr;
     }
  }
}