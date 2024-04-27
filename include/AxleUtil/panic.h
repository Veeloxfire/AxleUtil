#ifndef AXLETEST_CLIENT_H_
#define AXLETEST_CLIENT_H_

#include <AxleUtil/primitives.h>

namespace Axle {
  template<typename T>
  struct ViewArr;
}

namespace Axle::Panic {
  using CallbackFn = void(*)(const void* data, const Axle::ViewArr<const char>& message);

  void set_panic_callback(const void* data, CallbackFn callback) noexcept;

  void default_panic_callback(const void* data, const Axle::ViewArr<const char>& message) noexcept;

  [[noreturn]] void panic(const char* message, usize size) noexcept;
  
  template<usize N>
  [[noreturn]] void panic(const char (&message)[N]) noexcept {
    panic(message, N - 1);
  }
}
#endif
