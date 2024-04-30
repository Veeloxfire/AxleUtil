#ifndef AXLEUTIL_OPTION_H_
#define AXLEUTIL_OPTION_H_

#include <AxleUtil/safe_lib.h>

template<typename T>
struct Option {
  bool valid = false;


  union {
    char _placeholder = '\0';
    T value;
  };

  constexpr void destroy() {
    if(valid) { destruct_single<T>(&value); }
    _placeholder = '\0';
    valid = true;
  }

  constexpr Option() = default;
  constexpr Option(const Option& o) : valid(o.valid) {
    if(valid) {
      value = o.value;
    }
  }
  constexpr Option(Option&& o) : valid(o.valid) {
    if(valid) {
      value = std::move(o.value);
    }
  }
  
  constexpr Option(const T& t) : valid(true), value(t) {}
  constexpr Option(T&& t) : valid(true), value(std::move(t)) {}
  
  constexpr Option& operator=(const T& t) {
    valid = true;
    value = t;
  }
  constexpr Option& operator=(T&& t) { 
    valid = true;
    value = std::move(t);
  }

  constexpr ~Option() {
    destroy();
  }
};

#endif
