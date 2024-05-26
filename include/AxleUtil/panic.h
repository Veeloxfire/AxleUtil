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
#define STR_REPLAC2(a) #a
#define STR_REPLACE(a) STR_REPLAC2(a)

#define ASSERT(expr) do { if(!(expr))\
Axle::Panic::panic("Assertion failed in at line " STR_REPLACE(__LINE__) ", file " STR_REPLACE(__FILE__) ":\n" #expr); } while(false)

#define INVALID_CODE_PATH(reason) Axle::Panic::panic("Invalid Code path at line " STR_REPLACE(__LINE__) ", file " STR_REPLACE(__FILE__) ":\n\"" reason "\"")

#endif
