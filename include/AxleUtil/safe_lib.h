#ifndef AXLEUTIL_SAFE_LIB_H_
#define AXLEUTIL_SAFE_LIB_H_

#include <cstdlib>
#include <cassert>
#include <new>

#include <cstdint>

namespace Axle {
namespace Primitives {
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using usize = size_t;
using f32 = float;
static_assert(sizeof(f32) == 4);
using f64 = double;
static_assert(sizeof(f64) == 8);
}

using namespace Primitives;

#define STR_REPLAC2(a) #a
#define STR_REPLACE(a) STR_REPLAC2(a)

#define BYTE(a) (static_cast<uint8_t>(a))
#define JOI2(a, b) a ## b
#define JOIN(a, b) JOI2(a, b)

#define TODO() static_assert(false, "Code is broken")

void throw_testing_assertion(const char* message);
void abort_assertion(const char* message) noexcept;

#define ASSERT(expr) do { if(!(expr))\
Axle::throw_testing_assertion("Assertion failed in at line " STR_REPLACE(__LINE__) ", file " __FILE__ ":\n" #expr); } while(false)

#define INVALID_CODE_PATH(reason) Axle::throw_testing_assertion("Invalid Code path \"" reason "\"")

//#define COUNT_ALLOC

template<typename T>
struct TypeIdentity {
  using Type = T;
};

template<typename T>
using Self = typename TypeIdentity<T>::Type;

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
constexpr inline void default_init(Self<T>* const dest, const size_t dest_size) {
  for (size_t i = 0; i < dest_size; i++) {
    new(dest + i) T();
  }
}

template<typename T>
constexpr inline void default_init(Self<T>* const dest) {
  new(dest) T();
}

template<typename T>
constexpr inline void destruct_arr(Self<T>* const ptr, const size_t num) {
  for (size_t i = 0; i < num; i++) {
    ptr[i].~T();
  }
}

template<typename T>
constexpr inline void destruct_arr_void(void* const ptr_v, const size_t num) {
  T* ptr = std::launder(reinterpret_cast<T*>(ptr_v));

  for (size_t i = 0; i < num; i++) {
    ptr[i].~T();
  }
}

template<typename T>
constexpr inline void destruct_single(Self<T>* const ptr) {
  ptr->~T();
}

template<typename T>
constexpr inline void destruct_single_void(void* const ptr) {
  std::launder(reinterpret_cast<T*>(ptr))->~T();
}

template<typename T>
constexpr void reset_type(Self<T>* t) noexcept {
  t->~T();
  new(t) T();
}

template<typename T>
struct ViewArr {
  T* data = nullptr;
  usize size = 0;

  [[nodiscard]] constexpr const T* begin() const { return data; }
  [[nodiscard]] constexpr const T* end() const { return data + size; }

  [[nodiscard]] constexpr T* mut_begin() const { return data; }
  [[nodiscard]] constexpr T* mut_end() const { return data + size; }

  [[nodiscard]] constexpr T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }

  [[nodiscard]] constexpr operator ViewArr<const T>() const {
    return { data, size };
  }
};

template<typename T>
struct ViewArr<const T> {
  const T* data = nullptr;
  usize size = 0;

  [[nodiscard]] constexpr const T* begin() const { return data; }
  [[nodiscard]] constexpr const T* end() const { return data + size; }

  [[nodiscard]] constexpr const T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }
};

template<usize N>
[[nodiscard]] constexpr ViewArr<const char> lit_view_arr(const char(&arr)[N]) {
  ASSERT(arr[N - 1] == '\0');
  return {
    arr,
    N - 1,
  };
}

template<typename T>
constexpr inline void memcpy_ts(const ViewArr<T>& dest, const ViewArr<const T>& src) {
  ASSERT(dest.size == src.size);

  const size_t size = src.size;
  for (size_t i = 0; i < size; ++i) {
    dest.data[i] = src.data[i];
  }
}

template<typename T>
constexpr inline void memcpy_ts(const ViewArr<T>& dest, const ViewArr<T>& src) {
  return memcpy_ts<T>(dest, ViewArr<const T>(src));
}

template<typename T>
[[nodiscard]] constexpr inline bool memeq_ts(const ViewArr<const T>& dest, const ViewArr<const T>& src) {
  if (dest.size != src.size) return false;
  if (dest.data == src.data) return true;

  const size_t length = dest.size;
  for (size_t i = 0; i < length; ++i) {
    if (dest.data[i] != src.data[i]) return false;
  }

  return true;
}

template<typename T>
[[nodiscard]] constexpr inline bool memeq_ts(const ViewArr<T>& dest, const ViewArr<T>& src) {
  return memeq_ts<T>(ViewArr<const T>(dest), ViewArr<const T>(src));
}

template<typename T>
[[nodiscard]] constexpr inline bool memeq_ts(const ViewArr<const T>& dest, const ViewArr<T>& src) {
  return memeq_ts<T>(dest, ViewArr<const T>(src));
}

template<typename T>
[[nodiscard]] constexpr inline bool memeq_ts(const ViewArr<T>& dest, const ViewArr<const T>& src) {
  return memeq_ts<T>(ViewArr<const T>(dest), src);
}

template<typename T>
struct Viewable {
  template<typename U>
  struct TemplateFalse {
    constexpr static bool VAL = false;
  };

  static_assert(TemplateFalse<T>::VAL, "Attempted to use unspecialized viewable");
};

//Default implementation for a type which
//should do the same for `const` and non-`const`
template<typename T>
struct Viewable<const T> {
  using ViewT = typename Viewable<T>::ViewT;

  template<typename U>
  [[nodiscard]] static constexpr ViewArr<U> view(const T& v) {
    return Viewable<T>::template view<U>(v);
  }
};

template<typename T>
struct Viewable<ViewArr<T>> {
  using ViewT = T;

  template<typename U>
  [[nodiscard]] static constexpr ViewArr<U> view(const ViewArr<T>& v) {
    return v;
  }
};

template<typename T, size_t N>
struct Viewable<T[N]> {
  using ViewT = T;

  template<typename U>
  [[nodiscard]] static constexpr ViewArr<U> view(T(&arr)[N]) {
    return {arr, N};
  }
};

template<typename T, size_t N>
struct Viewable<const T[N]> {
  using ViewT = const T;

  template<typename U>
  [[nodiscard]] static constexpr ViewArr<U> view(const T(&arr)[N]) {
    return {arr, N};
  }
};

namespace ViewTemplates {
  template<typename T, bool c_view>
  struct MaybeConst; 

  template<typename T>
  struct MaybeConst<T, false> {
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
  using ViewType = typename MaybeConst<typename PickNonVoid<
      VT, 
      typename Viewable<T>::ViewT
  >::Type, c_view>::Type;
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<T, VT, false>> view_arr(T& t) {
  return Viewable<T>::template view<ViewTemplates::ViewType<T, VT, false>>(t);
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<T, VT, false>> view_arr(T& t, usize start, usize count) {
  const auto arr = Viewable<T>::template view<ViewTemplates::ViewType<T, VT, false>>(t);
  ASSERT(arr.size >= start + count);
  return {
    arr.data + start,
    count,
  };
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<T, VT, true>> const_view_arr(T& t) {
  return Viewable<T>::template view<ViewTemplates::ViewType<T, VT, true>>(t);
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<T, VT, true>> const_view_arr(T& t, usize start, usize count) {
  const auto arr = Viewable<T>::template view<ViewTemplates::ViewType<T, VT, true>>(t);
  ASSERT(arr.size >= start + count);
  return {
    arr.data + start,
    count,
  };
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<const T, VT, false>> view_arr(const T& t) {
  return Viewable<const T>::template view<ViewTemplates::ViewType<const T, VT, false>>(t);
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<const T, VT, false>> view_arr(const T& t, usize start, usize count) {
  const auto arr = Viewable<const T>::template view<ViewTemplates::ViewType<const T, VT, false>>(t);
  ASSERT(arr.size >= start + count);
  return {
    arr.data + start,
    count,
  };
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<const T, VT, true>> const_view_arr(const T& t) {
  return Viewable<const T>::template view<ViewTemplates::ViewType<const T, VT, true>>(t);
}

template<
  typename VT = void,
  typename T
>
[[nodiscard]] constexpr ViewArr<ViewTemplates::ViewType<const T, VT, true>> const_view_arr(const T& t, usize start, usize count) {
  const auto arr = Viewable<const T>::template view<ViewTemplates::ViewType<const T, VT, true>>(t);
  ASSERT(arr.size >= start + count);
  return {
    arr.data + start,
    count,
  };
}

template<typename T, typename U>
[[nodiscard]] ViewArr<T> cast_arr(const ViewArr<U>& arr) {
  static_assert(std::is_trivial_v<T> && std::is_trivial_v<U>);
  static_assert(sizeof(T) % sizeof(U) == 0);
  return { reinterpret_cast<T*>(arr.data), arr.size / sizeof(U) };
}

template<typename T, size_t size>
struct ConstArray {
  T data[size];

  template<typename ... U>
  [[nodiscard]] constexpr static auto create(U&& ... u) {
    static_assert(sizeof...(U) == size, "Must be fully filled");

    return ConstArray{ {std::forward<U>(u)...} };
  }

  [[nodiscard]] constexpr const T* begin() const { return data; }
  [[nodiscard]] constexpr const T* end() const { return data + size; }
};

template<typename T, size_t size>
struct Viewable<ConstArray<T, size>> {
  using ViewT = T;

  template<typename U>
  [[nodiscard]] static constexpr ViewArr<T> view(const ConstArray<T, size>& v) {
    return {v.data, size};
  }
};

#define FOR(name, it) \
for(auto it = (name).begin(), JOIN(__end, __LINE__) = (name).end(); \
it != JOIN(__end, __LINE__); ++it)

#define FOR_MUT(name, it) \
for(auto it = (name).mut_begin(), JOIN(__end, __LINE__) = (name).mut_end(); \
it != JOIN(__end, __LINE__); ++it)


[[nodiscard]] constexpr uint8_t absolute(int8_t i) {
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

[[nodiscard]] constexpr uint16_t absolute(int16_t i) {
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

[[nodiscard]] constexpr uint32_t absolute(int32_t i) {
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

[[nodiscard]] constexpr uint64_t absolute(int64_t i) {
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
[[nodiscard]] constexpr inline T square(T t) { return t * t; }

template<typename T>
[[nodiscard]] constexpr inline T larger(T t1, T t2) noexcept {
  return t1 > t2 ? t1 : t2;
}

template<typename T>
[[nodiscard]] constexpr inline T smaller(T t1, T t2) noexcept {
  return t1 < t2 ? t1 : t2;
}

template<typename T, size_t N>
[[nodiscard]] constexpr size_t array_size(T(&)[N]) {
  return N;
}

template<typename A>
struct ArraySize;

template<typename T, usize N>
struct ArraySize<T[N]> {
  constexpr static usize VAL = N;
};
}
#endif
