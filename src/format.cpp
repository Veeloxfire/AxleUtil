#include <AxleUtil/format.h>
#include <AxleUtil/tracing_wrapper.h>

#include <charconv>

namespace Axle {
Format::FloatStr Format::format_float(float f) {
  FloatStr dstr = {};
  std::to_chars_result fcr = std::to_chars(dstr.str, dstr.str + dstr.MAX, f);
  ASSERT(fcr.ec == std::errc{});
  const std::ptrdiff_t diff = fcr.ptr - dstr.str;
  ASSERT(diff > 0);
  dstr.count = static_cast<usize>(diff);
  return dstr;
}

Format::DoubleStr Format::format_double(double f) {
  DoubleStr dstr = {};
  std::to_chars_result fcr = std::to_chars(dstr.str, dstr.str + dstr.MAX, f);
  ASSERT(fcr.ec == std::errc{});
  const std::ptrdiff_t diff = fcr.ptr - dstr.str;
  ASSERT(diff > 0);
  dstr.count = static_cast<usize>(diff);
  return dstr;
}

OwnedArr<char> format_type_set(const ViewArr<const char>& format_in, const size_t prepend_spaces, const size_t max_width) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  ASSERT(max_width > 0);
  
  Array<char> result = {};
  result.reserve_extra(format_in.size);

  const char* format_i = format_in.begin();
  const char* const format_end = format_in.end();

  const auto prepend = [&] {
    result.reserve_extra(prepend_spaces);
    for (size_t i = 0; i < prepend_spaces; i++) {
      result.insert(' ');
    }
  };

  const auto get_content = [&] {
    const char* start = format_i;
    usize size = 0;

    while (format_i != format_end
        && *format_i != ' '
        && *format_i != '\n'
        && *format_i != '\r'
        && *format_i != '\t') {
      format_i++;
      size++;
    }

    return ViewArr<const char>{ start, size };
  };

  struct Gap {
    u32 num_newlines;
    Axle::ViewArr<const char> remaining_gap;
  };

  const auto get_gap = [&] {
    const char* start = format_i;
    usize size = 0;
    u32 num_newlines = 0;

    while (format_i != format_end
        && (*format_i == ' '
        || *format_i == '\n'
        || *format_i == '\r'
        || *format_i == '\t')) {
      if (*format_i == '\n') {
        num_newlines += 1;

        format_i++;
        size = 0;
      }
      else {
        format_i++;
        size++;
      }
    }

    return Gap{ num_newlines, { start, size } };
  };

  prepend();

  size_t curr_length = 0;

  while (true) {
    auto gap = get_gap();

    if (format_i == format_end) break;

    if (gap.num_newlines > 0) {
      result.reserve_extra(gap.num_newlines);
      for (u32 i = 0; i < gap.num_newlines; ++i) {
        result.insert('\n');
      }

      prepend();
      curr_length = 0;
    }

    ASSERT(format_i != format_end);
    auto content = get_content();
    ASSERT(content.size > 0);

    if (gap.remaining_gap.size + content.size + curr_length + prepend_spaces > max_width) {
      if (curr_length > 0) {
        result.insert('\n');
        prepend();
      }
      result.concat(content);
      curr_length = content.size;
    }
    else {
      result.concat(gap.remaining_gap);
      result.concat(content);
      curr_length += gap.remaining_gap.size + content.size;
    }

    if (format_i == format_end) break;
  }

  return bake_arr(std::move(result));
}
}
