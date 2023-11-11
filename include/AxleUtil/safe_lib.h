#ifndef AXLEUTIL_SAFE_LIB_H_
#define AXLEUTIL_SAFE_LIB_H_

#include <cstdlib>
#include <cassert>
#include <new>

#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using usize = size_t;

#define STR_REPLAC2(a) #a
#define STR_REPLACE(a) STR_REPLAC2(a)

#define BYTE(a) (static_cast<uint8_t>(a))
#define JOI2(a, b) a ## b
#define JOIN(a, b) JOI2(a, b)

#define TODO() static_assert(false, "Code is broken")

void throw_testing_assertion(const char* message);
void abort_assertion(const char* message);

#ifdef ASSERT_EXCEPTIONS
#define ASSERT(expr) do { if(!(expr))\
throw_testing_assertion("Assertion failed in at line " STR_REPLACE(__LINE__) ", file " __FILE__ ":\n" #expr); } while(false)

#define INVALID_CODE_PATH(reason) throw_testing_assertion("Invalid Code path \"" reason "\"")
#else
#define ASSERT(expr) assert(expr)

#ifdef NDEBUG
#define INVALID_CODE_PATH(reason) abort_assertion("Invalid Code path \"" reason "\"")
#else
#define INVALID_CODE_PATH(reason) assert(((reason), false));
#endif
#endif

//#define COUNT_ALLOC

template<typename T>
constexpr inline void memcpy_ts(T* dest, size_t dest_size, const T* source, size_t src_size) {
  ASSERT(dest_size >= src_size);

  for (size_t i = 0; i < src_size; ++i) {
    dest[i] = source[i];
  }
}

template<typename T>
constexpr inline bool memeq_ts(const T* buff1, const T* buff2, size_t length) {
  if (buff1 == buff2) return true;

  for (size_t i = 0; i < length; ++i) {
    if (buff1[i] != buff2[i]) return false;
  }

  return true;
}

constexpr bool streq_ts(const char* str1, const char* str2) {
  while (str1[0] != '\0' && str1[0] == str2[0]) {
    str1++;
    str2++;
  }

  return str1[0] == str2[0];//both are ended
}

constexpr size_t strlen_ts(const char* c) {
  const char* const base = c;
  while (*c != '\0') { c++; }

  return c - base;
}

template<typename T>
constexpr inline void default_init(T* const dest, const size_t dest_size) {
  for (size_t i = 0; i < dest_size; i++) {
    new(dest + i) T();
  }
}

template<typename T>
constexpr inline void default_init(T* const dest) {
  new(dest) T();
}

template<typename T>
constexpr inline void destruct_arr(T* const ptr, const size_t num) {
  for (size_t i = 0; i < num; i++) {
    ptr[i].~T();
  }
}

template<typename T>
constexpr inline void destruct_single(T* const ptr) {
  ptr->~T();
}

template<typename T>
constexpr void reset_type(T* t) noexcept {
  t->~T();
  new(t) T();
}

template<typename T>
struct ViewArr {
  T* data = nullptr;
  usize size = 0;

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }

  constexpr T* mut_begin() const { return data; }
  constexpr T* mut_end() const { return data + size; }

  constexpr T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }

  constexpr operator ViewArr<const T>() const {
    return { data, size };
  }
};

template<typename T>
struct ViewArr<const T> {
  const T* data = nullptr;
  usize size = 0;

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }

  constexpr const T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }
};

template<usize N>
constexpr ViewArr<const char> lit_view_arr(const char(&arr)[N]) {
  ASSERT(arr[N - 1] == '\0');
  return {
    arr,
    N - 1,
  };
}

template<typename T>
struct Viewable {
  template<typename U>
  struct TemplateFalse {
    constexpr static bool VAL = false;
  };

  static_assert(TemplateFalse<T>::VAL, "Attempted to use unspecialized viewable");
};

template<typename T>
struct Viewable<ViewArr<T>> {
  using ViewT = T;

  template<typename U>
  static constexpr ViewArr<U> view(const ViewArr<T>& v) {
    return v;
  }
};

template<typename T, size_t N>
struct Viewable<T[N]> {
  using ViewT = T;

  template<typename U>
  static constexpr ViewArr<U> view(T(&arr)[N]) {
    return {arr, N};
  }
};



namespace ViewTemplates {
  template<typename T, bool c_view>
  struct MaybeConst {
    using Type = T;
  };

  template<typename T>
  struct MaybeConst<T, true> {
    using Type = const T;
  };


  template<typename T, typename U>
  struct PickNonVoid {
    using Type = T;
  };

  template<typename U>
  struct PickNonVoid<void, U> {
    using Type = U;
  };

  template<>
  struct PickNonVoid<void, void>;

  template<typename T, typename VT, bool c_view>
  struct Dispatch {
    template<typename U>
    struct TemplateFalse {
      constexpr static bool VAL = false;
    };

    static_assert(TemplateFalse<T>::VAL, "Invalid use of Dispatch, expected a reference");

  };

  template<typename T, typename VT, bool c_view>
  struct Dispatch<const T&, VT, c_view> {
    using VTo = typename MaybeConst<typename PickNonVoid<
      VT, 
      typename Viewable<T>::ViewT
    >::Type, c_view>::Type;

    using Ret = ViewArr<VTo>;
    constexpr static ViewArr<VTo> view(const T& t) {
      return Viewable<T>::template view<VTo>(t);
    }
  };

  template<typename T, size_t N, typename VT, bool c_view>
  struct Dispatch<T(&)[N], VT, c_view> {
    using VTo = typename MaybeConst<typename PickNonVoid<
      VT, 
    typename Viewable<T[N]>::ViewT
  >::Type, c_view>::Type;

  using Ret = ViewArr<VTo>;
  constexpr static ViewArr<VTo> view(T(&t)[N]) {
    return Viewable<T[N]>::template view<VTo>(t);
  }
};

template<typename T, size_t N, typename VT, bool c_view>
struct Dispatch<const T(&)[N], VT, c_view> {
  using VTo = typename MaybeConst<typename PickNonVoid<
    VT, 
    typename Viewable<const T[N]>::ViewT
  >::Type, c_view>::Type;

  using Ret = ViewArr<VTo>;
  constexpr static ViewArr<VTo> view(const T(&t)[N]) {
    return Viewable<const T[N]>::template view<VTo>(t);
  }
};
}

template<
  typename T, 
  typename VT = void
>
constexpr auto view_arr(const T& t) 
-> typename ViewTemplates::Dispatch<decltype(t), VT, false>::Ret {
  return ViewTemplates::Dispatch<decltype(t), VT, false>::view(t);
}

template<
  typename T, 
  typename VT = void
>
constexpr auto view_arr(const T& t, usize start, usize count) 
  -> typename ViewTemplates::Dispatch<decltype(t), VT, false>::Ret
{
  const auto arr = ViewTemplates::Dispatch<decltype(t), VT, false>::view(t);
  ASSERT(arr.size >= start + count);
  return {
    arr.data + start,
    count,
  };
}

template<
  typename T, 
  typename VT = void
>
constexpr auto const_view_arr(const T& t)
  -> typename ViewTemplates::Dispatch<decltype(t), VT, true>::Ret {
  return ViewTemplates::Dispatch<decltype(t), VT, true>::view(t);
}

template<
  typename T, 
  typename VT = void
>
constexpr auto const_view_arr(const T& t, usize start, usize count) 
  -> typename ViewTemplates::Dispatch<decltype(t), VT, true>::Ret
{
  const auto arr = ViewTemplates::Dispatch<decltype(t), VT, true>::view(t);
  ASSERT(arr.size >= start + count);
  return {
    arr.data + start,
    count,
  };
}

template<typename T, size_t size>
struct ConstArray {
  T data[size];

  template<typename ... U>
  constexpr static auto create(U&& ... u) {
    static_assert(sizeof...(U) == size, "Must be fully filled");

    return ConstArray{ {std::forward<U>(u)...} };
  }

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }
};

template<typename T, size_t size>
struct Viewable<ConstArray<T, size>> {
  using ViewT = T;

  template<typename U>
  static constexpr ViewArr<T> view(const ConstArray<T, size>& v) {
    return {v.data, size};
  }
};

#define FOR(name, it) \
for(auto it = (name).begin(), JOIN(__end, __LINE__) = (name).end(); \
it < JOIN(__end, __LINE__); it++)

#define FOR_MUT(name, it) \
for(auto it = (name).mut_begin(), JOIN(__end, __LINE__) = (name).mut_end(); \
it < JOIN(__end, __LINE__); it++)


constexpr uint8_t absolute(int8_t i) {
  if (i == INT8_MIN) {
    return static_cast<uint8_t>(INT8_MAX) + 1u;
  }
  else if (i < 0) {
    return static_cast<uint8_t>(-i);
  }
  else {
    return static_cast<uint8_t>(i);
  }
}

constexpr uint16_t absolute(int16_t i) {
  if (i == INT16_MIN) {
    return static_cast<uint16_t>(INT16_MAX) + 1u;
  }
  else if (i < 0) {
    return static_cast<uint16_t>(-i);
  }
  else {
    return static_cast<uint16_t>(i);
  }
}

constexpr uint32_t absolute(int32_t i) {
  if (i == INT32_MIN) {
    return static_cast<uint32_t>(INT32_MAX) + 1u;
  }
  else if (i < 0) {
    return static_cast<uint32_t>(-i);
  }
  else {
    return static_cast<uint32_t>(i);
  }
}

constexpr uint64_t absolute(int64_t i) {
  if (i == INT64_MIN) {
    return 0x8000000000000000ull;
  }
  else if (i < 0) {
    return static_cast<uint64_t>(-i);
  }
  else {
    return static_cast<uint64_t>(i);
  }
}

template<typename T>
constexpr inline T square(T t) { return t * t; }

template<typename T>
constexpr inline T larger(T t1, T t2) noexcept {
  return t1 > t2 ? t1 : t2;
}

template<typename T>
constexpr inline T smaller(T t1, T t2) noexcept {
  return t1 < t2 ? t1 : t2;
}

template<typename T, size_t N>
constexpr size_t array_size(T(&)[N]) {
  return N;
}

template<typename A>
struct ArraySize;

template<typename T, usize N>
struct ArraySize<T[N]> {
  constexpr static usize VAL = N;
};

#endif
