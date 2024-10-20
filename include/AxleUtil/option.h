#ifndef AXLEUTIL_OPTION_H_
#define AXLEUTIL_OPTION_H_

#include <AxleUtil/safe_lib.h>
#include <utility>

namespace Axle {
struct InPlaceT {};

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
  constexpr Option(const Option& o) : valid(o.valid), value(o.value) {}
  constexpr Option(Option&& o) : valid(o.valid), value(std::move(o.value)) {
    o.destroy();
  }
  
  constexpr Option(const T& t) : valid(true), value(t) {}
  constexpr Option(T&& t) : valid(true), value(std::move(t)) {}

  template<typename ... Args>
  constexpr Option(InPlaceT, Args&& ... args) : valid(true), value(std::forward<Args>(args)...) {}
  
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
      value = t.value;
    } else {
      destroy();
    }

    return *this;
  }

  constexpr Option& operator=(Option&& t) {
    if(t.valid) {
      if(!valid) construct_single<T>(&value);
      valid = true;
      value = std::move(t.value);
      t.destroy();
    } else {
      destroy();
    }

    return *this;
  }

  constexpr void set_value(const T& t) {
    if(valid) {
      value = t;
    }
    else {
      new (&value) T(t);
    }

    valid = true;
  }

  constexpr void set_value(T&& t) {
    if(valid) {
      value = std::move(t);
    }
    else {
      new (&value) T(std::move(t));
    }

    valid = true;
  }

  constexpr ~Option() {
    destroy();
  }
};
}

#endif
