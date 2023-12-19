#ifndef AXLEUTIL_FORMATTABLE_H_
#define AXLEUTIL_FORMATTABLE_H_

#include <AxleUtil/safe_lib.h>

#ifdef AXLE_TRACE
#include <Tracer/trace.h>
#endif

namespace Axle {
//For printing character as it appears in code
struct DisplayChar {
  char c;
};

struct DisplayString {
  const char* arr;
  usize size;
};

struct PrintPtr {
  const void* ptr;
};

struct MagicNumber {
  uint16_t num;
};

struct ByteArray {
  const u8* ptr;
  usize size;
};

template<typename T>
struct PrintList {
  const T* arr;
  usize size;
};

template<typename T, typename L>
struct PrintListCF {
  const T* arr;
  usize size;

  const L& format;
};


namespace Format {
  template<typename T>  
  struct Hex {
    const T& t;
  };

  template<typename T>
  Hex(const T& t) -> Hex<T>;

  template<typename F>
  concept Formatter = requires(F & f, char c, const char* ptr, usize n,
                               const char(&arr1)[1], const char(&arr2)[10], const char(&arr3)[100])
  {
    {f.load_char(c)};
    {f.load_string(ptr, n)};
    {f.load_string_lit(arr1)};
    {f.load_string_exact(arr1)};
    {f.load_string_lit(arr2)};
    {f.load_string_exact(arr2)};
    {f.load_string_lit(arr3)};
    {f.load_string_exact(arr3)};
  };

  template<typename T>
  struct FormatArg {
    template<typename U>
    struct TemplateFalse {
      constexpr static bool VAL = false;
    };

    static_assert(TemplateFalse<T>::VAL, "Attempted to use unspecialized format arg");
  };

  template<>
  struct FormatArg<bool> {
    template<Formatter F>
    constexpr static void load_string(F& res, bool b) {
      if (b) {
        res.load_string_lit("true");
      }
      else {
        res.load_string_lit("false");
      }
    }
  };

  template<>
  struct FormatArg<char> {
    template<Formatter F>
    constexpr static void load_string(F& res, char c) {
      res.load_char(c);
    }
  };

  template<>
  struct FormatArg<DisplayChar> {
    template<Formatter F>
    constexpr static void load_string(F& res, DisplayChar c) {
      switch (c.c) {
        case '\n': {
            res.load_string_lit("\\n");
            break;
          }
        case '\r': {
            res.load_string_lit("\\r");
            break;
          }
        case '\f': {
            res.load_string_lit("\\f");
            break;
          }
        case '\t': {
            res.load_string_lit("\\t");
            break;
          }
        case '\0': {
            res.load_string_lit("\\0");
            break;
          }
        default:
          res.load_char(c.c);
          break;
      }
    }
  };

  template<>
  struct FormatArg<DisplayString> {
    template<Formatter F>
    constexpr static void load_string(F& res, const DisplayString& ds) {
      for (usize i = 0; i < ds.size; ++i) {
        FormatArg<DisplayChar>::load_string(res, DisplayChar{ ds.arr[i] });
      }
    }
  };

  constexpr char hex_char(int c) {
    if (0 <= c && c <= 9) {
      return static_cast<char>(c + '0');
    }
    else if (0xa <= c && c <= 0xf) {
      return static_cast<char>((c - 0xa) + 'a');
    }
    return '\0';
  };

  template<>
  struct FormatArg<MagicNumber> {
    template<Formatter F>
    constexpr static void load_string(F& res, MagicNumber c) {
      char chars[4] = {};

      chars[0] = hex_char(((int)c.num & 0x00f0) >> 4);
      chars[1] = hex_char(((int)c.num & 0x000f));
      chars[2] = hex_char(((int)c.num & 0xf000) >> 12);
      chars[3] = hex_char(((int)c.num & 0x0f00) >> 8);

      res.load_string_exact(chars);
    }
  };

  template<>
  struct FormatArg<ViewArr<const char>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const ViewArr<const char>& str) {
      res.load_string(str.data, str.size);
    }
  };

  template<>
  struct FormatArg<ViewArr<char>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const ViewArr<char>& str) {
      res.load_string(str.data, str.size);
    }
  };

  template<Formatter F>
  constexpr static void load_unsigned(F& res, uint64_t u) {
    if (u == 0) {
      return res.load_char('0');
    }

    constexpr usize MAX = 20;
    usize i = MAX;
    char arr[MAX + 1] = {};

    while (u > 0) {
      ASSERT(i > 0);
      auto d = u % 10;
      arr[i - 1] = static_cast<char>('0' + d);
      i -= 1;
      u /= 10;
    }

    return res.load_string(arr + i, MAX - i);
  }

  template<Formatter F>
  constexpr static void load_unsigned_hex(F& res,
                                          uint64_t u,
                                          const usize bytes) {
    constexpr size_t MAX_LEN = 16;
    ASSERT(bytes * 2 <= MAX_LEN);
    ASSERT(bytes >= 1);

    char string_res[2 + MAX_LEN] = {
      '0', 'x',
      '0', '0', '0', '0', '0', '0', '0', '0',
      '0', '0', '0', '0', '0', '0', '0', '0'
    };

    usize end_len = 2 + (bytes * 2);

    for (u32 i = 0; i < (end_len - 2); i++) {
      u8 digit = u & 0xF;
      u >>= 4;

      if (digit >= 10) {
        ASSERT(digit < 16);
        string_res[(end_len - 1) - i] = 'A' + (digit - 10);
      }
      else {
        string_res[(end_len - 1) - i] = '0' + digit;
      }
    }

    res.load_string(string_res, end_len);
  }

  template<>
  struct FormatArg<PrintPtr> {
    template<Formatter F>
    constexpr static void load_string(F& res, PrintPtr ptr) {
      if (ptr.ptr == nullptr) {
        return res.load_string_lit("nullptr");
      }
      else {
        return load_unsigned_hex(res, (uintptr_t)ptr.ptr, 8);
      }
    }
  };

  template<>
  struct FormatArg<unsigned char> {
    template<Formatter F>
    constexpr static void load_string(F& res, unsigned char u) {
      return load_unsigned(res, u);
    }
  };

  template<>
  struct FormatArg<unsigned short> {
    template<Formatter F>
    constexpr static void load_string(F& res, unsigned short u) {
      return load_unsigned(res, u);
    }
  };

  template<>
  struct FormatArg<unsigned int> {
    template<Formatter F>
    constexpr static void load_string(F& res, unsigned int u) {
      return load_unsigned(res, u);
    }
  };

  template<>
  struct FormatArg<unsigned long> {
    template<Formatter F>
    constexpr static void load_string(F& res, unsigned long u) {
      return load_unsigned(res, u);
    }
  };

  template<>
  struct FormatArg<unsigned long long> {
    template<Formatter F>
    constexpr static void load_string(F& res, unsigned long long u) {
      return load_unsigned(res, u);
    }
  };

  template<>
  struct FormatArg<signed char> {
    template<Formatter F>
    constexpr static void load_string(F& res, signed char i) {
      if (i < 0) {
        res.load_char('-');
      }

      return load_unsigned(res, absolute(i));
    }
  };

  template<>
  struct FormatArg<signed short> {
    template<Formatter F>
    constexpr static void load_string(F& res, signed short i) {
      if (i < 0) {
        res.load_char('-');
      }

      return load_unsigned(res, absolute(i));
    }
  };


  template<>
  struct FormatArg<signed int> {
    template<Formatter F>
    constexpr static void load_string(F& res, signed int i) {
      if (i < 0) {
        res.load_char('-');
      }

      return load_unsigned(res, absolute(i));
    }
  };

  template<>
  struct FormatArg<signed long> {
    template<Formatter F>
    constexpr static void load_string(F& res, signed long i) {
      if (i < 0) {
        res.load_char('-');
      }

      return load_unsigned(res, absolute((int64_t)i));
    }
  };

  template<>
  struct FormatArg<signed long long> {
    template<Formatter F>
    constexpr static void load_string(F& res, signed long long i) {
      if (i < 0) {
        res.load_char('-');
      }

      return load_unsigned(res, absolute(i));
    }
  };

  struct FloatStr {
    static constexpr usize MAX = 15;
    char str[MAX];
    usize count;
  };
  FloatStr format_float(float f);

  template<>
  struct FormatArg<float> {
    template<Formatter F>
    static void load_string(F& res, float f) {
      FloatStr fstr = format_float(f);
      res.load_string(fstr.str, fstr.count);
    }
  };

  struct DoubleStr {
    static constexpr usize MAX = 24;
    char str[MAX];
    usize count;
  };
  DoubleStr format_double(double f);

  template<>
  struct FormatArg<double> {
    template<Formatter F>
    static void load_string(F& res, double f) {
      DoubleStr fstr = format_double(f);
      res.load_string(fstr.str, fstr.count);
    }
  };

  template<>
  struct FormatArg<Hex<u8>> {
    template<Formatter F>
    constexpr static void load_string(F& res, Hex<u8> hu8) {
      return load_unsigned_hex(res, hu8.t, 1);  
    }
  };

  template<>
  struct FormatArg<Hex<u16>> {
    template<Formatter F>
    constexpr static void load_string(F& res, Hex<u16> hu16) { 
      return load_unsigned_hex(res, hu16.t, 2);
    }
  };

  template<>
  struct FormatArg<Hex<u32>> {
    template<Formatter F>
    constexpr static void load_string(F& res, Hex<u32> hu32) { 
      return load_unsigned_hex(res, hu32.t, 4);
    }
  };

  template<>
  struct FormatArg<Hex<u64>> {
    template<Formatter F>
    constexpr static void load_string(F& res, Hex<u64> hu64) { 
      return load_unsigned_hex(res, hu64.t, 8);
    }
  };

  template<>
  struct FormatArg<ByteArray> {
    template<Formatter F>
    constexpr static void load_string(F& res, const ByteArray& arr) {
      char as_string[] = ", 0x00";

      const u8* i = arr.ptr;
      const u8* end = i + arr.size;

      if (i < end) {
        as_string[4] = hex_char(((*i) >> 4) & 0xf);
        as_string[5] = hex_char((*i) & 0x0f);

        res.load_string(as_string + 2, 4);
        i += 1;

        while (i < end) {
          as_string[4] = hex_char(((*i) >> 4) & 0xf);
          as_string[5] = hex_char((*i) & 0x0f);
          res.load_string(as_string, 6);
          i += 1;
        }
      }
    }
  };

  template<typename T>
  struct FormatArg<PrintList<T>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const PrintList<T>& arr) {
      usize i = 0;
      if (i < arr.size) {
        FormatArg<T>::load_string(res, arr.arr[i]);
        ++i;
        for (; i < arr.size; ++i) {
          res.load_string_lit(", ");
          FormatArg<T>::load_string(res, arr.arr[i]);
        }
      }
    }
  };


  template<typename T, typename L>
  struct FormatArg<PrintListCF<T, L>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const PrintListCF<T, L>& arr) {
      usize i = 0;
      if (i < arr.size) {
        arr.format(res, arr.arr[i]);
        ++i;
        for (; i < arr.size; ++i) {
          res.load_string_lit(", ");
          arr.format(res, arr.arr[i]);
        }
      }
    }
  };



  template<usize N>
  struct FormatArg<const char[N]> {
    template<Formatter F>
    constexpr static void load_string(F& res, const char(&arr)[N]) {
      res.load_string_lit(arr);
    }
  };

  template<usize N>
  struct FormatArg<char[N]> {
    template<Formatter F>
    constexpr static void load_string(F& res, const char(&arr)[N]) {
      res.load_string_lit(arr);
    }
  };

  struct FormatString {
    const char* arr;
    usize len;

    template<usize N>
    constexpr FormatString(const char(&_arr)[N]) : arr(_arr), len(N) {}

    constexpr FormatString(const char* _arr, usize _len) : arr(_arr), len(_len) {}
  };

  template<Formatter F>
  struct FormatDispatch {
    FormatString format_string;

    F& result;
  };

  template<Formatter F, typename T>
  constexpr FormatDispatch<F>& operator<<(FormatDispatch<F>& f, const T& t) {
    const char* string = f.format_string.arr;

    while (true) {
      if (f.format_string.len == 0 || f.format_string.arr[0] == '\0') {
        INVALID_CODE_PATH("Found too many arguments in format");
      }
      else if (f.format_string.arr[0] == '{') {
        if(f.format_string.arr[1] == '{') {
          const size_t num_chars = f.format_string.arr - string;
          if(num_chars > 0) {
            f.result.load_string(string, num_chars);
          }

          f.format_string.arr += 1;
          f.format_string.len -= 1;
          string = f.format_string.arr;
        }
        else if(f.format_string.arr[1] == '}') {
          const size_t num_chars = f.format_string.arr - string;
          if (num_chars > 0) {
            f.result.load_string(string, num_chars);
          }

          FormatArg<T>::load_string(f.result, t);

          f.format_string.arr += 2;
          f.format_string.len -= 2;
          return f;
        }
        else {
          INVALID_CODE_PATH("Invalid format string: '{' not followed by '{' or '}'");
        }
      }
      else if(f.format_string.arr[0] == '}') {
        if(f.format_string.arr[1] == '}') {
          const size_t num_chars = f.format_string.arr - string;
          if(num_chars > 0) {
            f.result.load_string(string, num_chars);
          }

          f.format_string.arr += 1;
          f.format_string.len -= 1;
          string = f.format_string.arr;
        }
        else {
          INVALID_CODE_PATH("Invalid format string: '}' not followed by '}'");
        }
      }

      f.format_string.arr += 1;
      f.format_string.len -= 1;
    }
  }

  //Doesnt null terminate!
  template<Formatter F, typename ... T>
  constexpr void format_to_formatter(F& result, FormatString format, const T& ... ts) {
  #ifdef AXLE_TRACING
    TRACING_FUNCTION();
  #endif

    if constexpr (sizeof...(T) > 0) {
      FormatDispatch<F> f = { format, result };

      (f << ... << ts);
      format = f.format_string;
    }

    const char* string = format.arr;

    while (true) {
      if (format.len == 0 || format.arr[0] == '\0') {
        const size_t num_chars = format.arr - string;
        if (num_chars > 0) {
          result.load_string(string, num_chars);
        }
        return;
      }
      else if (format.arr[0] == '{') {
        if(format.arr[1] == '{') {
          const size_t num_chars = format.arr - string;
          if(num_chars > 0) {
            result.load_string(string, num_chars);
          }

          format.arr += 1;
          format.len -= 1;
          string = format.arr;
        }
        else if(format.arr[1] == '}') {
          INVALID_CODE_PATH("Expected extra arguments in format");
          break;
        }
        else {
          INVALID_CODE_PATH("Invalid format string: '{' not followed by '{' or '}'");
        }
      }
      else if(format.arr[0] == '}') {
        if(format.arr[1] == '}') {
          const size_t num_chars = format.arr - string;
          if(num_chars > 0) {
            result.load_string(string, num_chars);
          }

          format.arr += 1;
          format.len -= 1;
          string = format.arr;
        }
        else {
          INVALID_CODE_PATH("Invalid format string: '}' not followed by '}'");
        }
      }

      format.arr += 1;
      format.len -= 1;
    }
  }
}
}
#endif
