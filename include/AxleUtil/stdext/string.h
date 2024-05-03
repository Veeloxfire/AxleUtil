#ifndef AXLEUTIL_STDEXT_STRING_H_
#define AXLEUTIL_STDEXT_STRING_H_

#include <string>
#include <AxleUtil/formattable.h>

namespace Axle {
  template<typename T> 
  struct Viewable<std::basic_string<T>> {
    using ViewT = T;

    template<typename U> constexpr static ViewArr<U> view(std::basic_string<T>& str) {
      return {str.data(), str.size()};
    }
  };

  template<typename T>
  struct Viewable<const std::basic_string<T>> {
    using ViewT = const T;

    template<typename U>
    constexpr static ViewArr<U> view(const std::basic_string<T>& str) {
      return {str.data(), str.size()};
    }
  };
}

namespace Axle::Format {
  template<>
  struct FormatArg<std::string> {
    template<Formatter F>
    static void load_string(F& f, const std::string& str) {
      f.load_string(str.c_str(), str.length());
    }
  };
}

#endif
