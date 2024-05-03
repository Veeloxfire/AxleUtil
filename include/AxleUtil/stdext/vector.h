#ifndef AXLEUTIL_STDEXT_VECTOR_H_
#define AXLEUTIL_STDEXT_VECTOR_H_

#include <vector>

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/formattable.h>

namespace Axle {
  template<typename T> 
  struct Viewable<std::vector<T>> {
    using ViewT = T;

    template<typename U> constexpr static ViewArr<U> view(std::vector<T>& vec) {
      return {vec.data(), vec.size()};
    }
  };

  template<typename T>
  struct Viewable<const std::vector<T>> {
    using ViewT = const T;

    template<typename U>
    constexpr static ViewArr<U> view(const std::vector<T>& vec) {
      return {vec.data(), vec.size()};
    }
  };
}

#endif
