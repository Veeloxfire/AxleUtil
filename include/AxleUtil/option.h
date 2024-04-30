#ifndef AXLEUTIL_OPTION_H_
#define AXLEUTIL_OPTION_H_

#include <AxleUtil/safe_lib.h>

namespace Axle {
template<typename T>
struct Option {
  bool valid = false;


  union {
    char _placeholder = '\0';
    T value;
  };

  constexpr void destroy() {
    if(valid) { Axle::destruct_single<T>(&value); }
    _placeholder = '\0';
    valid = false;
  }

  constexpr Option() = default;
  constexpr Option(const Option& o) : valid(o.valid) {
    if(valid) {
      new (&value) T(value);
    }
  }
  constexpr Option(Option&& o) : valid(o.valid) {
    if(valid) {
      new(&value) T(std::move(o.value));
    }
  }
  
  constexpr Option(const T& t) : valid(true), value(t) {}
  constexpr Option(T&& t) : valid(true), value(std::move(t)) {}
  
  constexpr Option& operator=(const T& t) {
    if(!valid) construct_single<T>(&value);
    valid = true;
    value = t;

    return *this;
  }
  constexpr Option& operator=(T&& t) { 
    if(!valid) construct_single<T>(&value);
    valid = true;
    value = std::move(t);

    return *this;
  }

  constexpr Option& operator=(const Option& t) {
    if(t.valid) {
      if(!valid) construct_single<T>(&value);
      valid = true;
      value = t.t;
    } else {
      destroy();
    }

    return *this;
  }

  constexpr Option& operator=(Option&& t) {
    if(t.valid) {
      if(!valid) construct_single<T>(&value);
      valid = true;
      value = t.t;
    } else {
      destroy();
    }

    return *this;
  }

  constexpr ~Option() {
    destroy();
  }
};
}

#endif
