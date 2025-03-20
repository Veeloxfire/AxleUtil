#ifndef AXLEUTIL_FORMAT_H_
#define AXLEUTIL_FORMAT_H_

#include <AxleUtil/formattable.h>
#include <AxleUtil/utility.h>
#include <AxleUtil/stacktrace.h>
namespace Axle {
namespace Format {
  struct ArrayFormatter {
    struct HeapArr {
      bool is_heap;
      Array<char> arr;
    };

    struct LocalArr {
      bool is_heap = false;
      u8 size = 0;
      char arr[sizeof(HeapArr) - 2] = {};

      constexpr ~LocalArr() = default;
    };

    constexpr static usize LOCAL_ARR_SIZE = ArraySize<decltype(LocalArr::arr)>::VAL;

    static_assert(sizeof(HeapArr) == sizeof(LocalArr));
    static_assert(sizeof(LocalArr) < UINT8_MAX);//uses a u8 to determine size

    union {
      bool is_heap;
      LocalArr local_arr = {};
      HeapArr heap_arr;
    };

    ArrayFormatter() : local_arr() {};
    ~ArrayFormatter() {
      if (is_heap) {
        heap_arr.~HeapArr();
      }
      else {
        local_arr.~LocalArr();
      }
    }

    void reserve_extra(usize n) {
      if(!is_heap) {
        if(local_arr.size + n <= LOCAL_ARR_SIZE) return;
        Array<char> arr = {};
        arr.reserve_total(local_arr.size + n);
        arr.concat(local_arr.arr, local_arr.size);

        local_arr.~LocalArr();

        new (&heap_arr) HeapArr{
          true,
          std::move(arr),
        };
      }
      else {
        heap_arr.arr.reserve_extra(n);
      } 
    }

    void clear() {
      if(is_heap) {
        heap_arr.arr.clear();
      }
      else {
        local_arr.size = 0;
      }
    }

    ViewArr<const char> view() const {
      if (is_heap) {
        return const_view_arr(heap_arr.arr);
      }
      else {
        return ViewArr<const char>{local_arr.arr, static_cast<usize>(local_arr.size)};
      }

    }

    OwnedArr<char> take() {
      if (is_heap) {
        OwnedArr<char> c = bake_arr(std::move(heap_arr.arr));

        heap_arr.~HeapArr();
        new (&local_arr) LocalArr{};

        return c;
      }
      else {
        OwnedArr<char> c = copy_arr(local_arr.arr, static_cast<usize>(local_arr.size));
        local_arr.size = 0;
        return c;
      }
    }

    inline void load_string(const char* str, usize N) {
      ASSERT(N > 0);

      if (is_heap) {
        heap_arr.arr.concat(str, N);
      }
      else {
        if (LOCAL_ARR_SIZE - static_cast<usize>(local_arr.size) < N) {
          Array<char> arr = {};
          arr.reserve_total(local_arr.size + N);

          arr.concat(local_arr.arr, local_arr.size);
          arr.concat(str, N);

          local_arr.~LocalArr();

          new (&heap_arr) HeapArr{
            true,
            std::move(arr),
          };
        }
        else {
          memcpy_ts(local_arr.arr + local_arr.size, LOCAL_ARR_SIZE - local_arr.size,
                    str, N);

          local_arr.size += static_cast<u8>(N);
        }
      }
    }


    template<usize N>
    void load_string_lit(const char(&str)[N]) {
      ASSERT(str[N - 1] == '\0');
      load_string(str, N - 1);
    }

    template<usize N>
    void load_string_exact(const char(&str)[N]) {
      load_string(str, N);
    }

    inline void null_terminate() {
      if (is_heap) {
        heap_arr.arr.insert('\0');
      }
      else {
        if (LOCAL_ARR_SIZE - static_cast<usize>(local_arr.size) == 0) {
          Array<char> arr = {};
          arr.reserve_total(local_arr.size + 1u);

          arr.concat(local_arr.arr, local_arr.size);
          arr.insert('\0');

          local_arr.~LocalArr();

          new (&heap_arr) HeapArr{
            true,
            std::move(arr),
          };
        }
        else {
          local_arr.arr[local_arr.size] = '\0';
          local_arr.size += 1u;
        }
      }
    }


    inline void load_char(char c) {
      ASSERT(c != '\0');
      if (is_heap) {
        heap_arr.arr.insert(c);
      }
      else {
        if (LOCAL_ARR_SIZE - static_cast<usize>(local_arr.size) == 0) {
          Array<char> arr = {};
          arr.reserve_total(local_arr.size + 1u);

          arr.concat(local_arr.arr, local_arr.size);
          arr.insert(c);

          local_arr.~LocalArr();

          new (&heap_arr) HeapArr{
            true,
            std::move(arr),
          };
        }
        else {
          local_arr.arr[local_arr.size] = c;
          local_arr.size += 1u;
        }
      }
    }
  };

  struct ViewFormatter {
    ViewArr<char> view;
    usize capacity;

    constexpr ViewFormatter(const ViewArr<char>& view)
      : view{view.data, 0}, capacity{view.size} {}

    constexpr void load_string(const char* str, usize N) {
      ASSERT(N > 0);
      const usize remaining = capacity - view.size;
      ASSERT(N <= remaining);

      memcpy_ts(view.mut_end(), remaining, str, N);
      view.size += N;
    }

    template<usize N>
    constexpr void load_string_lit(const char(&str)[N]) {
      ASSERT(str[N - 1] == '\0');
      load_string(str, N - 1);
    }

    template<usize N>
    constexpr void load_string_exact(const char(&str)[N]) {
      load_string(str, N);
    }

    constexpr void null_terminate() {
      const usize remaining = capacity - view.size;
      ASSERT(remaining >= 1);

      view.size += 1;
      view[view.size - 1] = '\0';
    }

    constexpr void load_char(char c) {
      ASSERT(c != '\0');
      
      const usize remaining = capacity - view.size;
      ASSERT(remaining >= 1);

      view.size += 1;
      view[view.size - 1] = c;
    }
  };
}

template<>
struct Viewable<Format::ArrayFormatter> {
  using ViewT = const char;

  template<typename U>
  static constexpr ViewArr<U> view(const Format::ArrayFormatter& f) {
    if (f.is_heap) {
      return view_arr(f.heap_arr.arr);
    }
    else {
      return { f.local_arr.arr, static_cast<usize>(f.local_arr.size) };
    }
  }
};

//Doesn't null terminate
template<typename ... T>
OwnedArr<char> format(const Format::FormatString<T...>& format, const T& ... ts) {
  STACKTRACE_FUNCTION();
  Format::ArrayFormatter result = {};
  //result.arr.reserve_total(format.len);

  Format::format_to(result, format, ts...);

  return result.take();
}

OwnedArr<char> format_type_set(const ViewArr<const char>& format, size_t prepend_spaces, size_t max_width);
}
#endif
