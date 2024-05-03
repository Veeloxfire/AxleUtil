#ifndef AXLEUTIL_STDEXT_COMPARE_H_
#define AXLEUTIL_STDEXT_COMPARE_H_

#include <compare>

#include <AxleUtil/formattable.h>

namespace Axle::Format {
  template<>
  struct FormatArg<std::strong_ordering> {
    template<Formatter F>
    constexpr static void load_string(F& res, const std::strong_ordering& so) {
      if(so == std::strong_ordering::equal) {
        res.load_string_lit("std::strong_ordering::equal");
      }
      else if(so == std::strong_ordering::less) {
        res.load_string_lit("std::strong_ordering::less");
      }
      else if(so == std::strong_ordering::greater) {
        res.load_string_lit("std::strong_ordering::less");
      }
      else {
        INVALID_CODE_PATH("Impossible std::strong_ordering value");
      }
    }
  }; 
}

#endif
