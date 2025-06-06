#ifndef AXLEUTIL_SAFE_LIB_H_
#define AXLEUTIL_SAFE_LIB_H_

#include <cstdlib>
#include <new>

#include <AxleUtil/primitives.h>
#include <AxleUtil/panic.h>

namespace Axle {
#define BYTE(a) (static_cast<uint8_t>(a))
#define JOI2(a, b) a ## b
#define JOIN(a, b) JOI2(a, b)

#define TODO() static_assert(false, "Code is broken")

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

constexpr usize strlen_ts(const char* c) {
  usize counter = 0;
  while (*c != '\0') { c++; counter++; }

  return counter;
}

template<typename T>
constexpr inline void default_init(Self<T>* dest, const size_t dest_size) {
  const T* const end = dest + dest_size;
  for (; dest < end; ++dest) {
    new(dest) T();
  }
}

template<typename T>
constexpr inline void default_init(Self<T>* const dest) {
  new(dest) T();
}

template<typename T>
constexpr inline void destruct_arr(Self<T>* ptr, const size_t num) {
  const T* const end = ptr + num;
  for (; ptr < end; ++ptr) {
    ptr->~T();
  }
}

template<typename T>
constexpr inline void destruct_arr_void(void* const ptr_v, const size_t num) {
  T* ptr = std::launder(reinterpret_cast<T*>(ptr_v));

  const T* const end = ptr + num;
  for (; ptr < end; ++ptr) {
    ptr->~T();
  }
}

template<typename T, typename ... Args>
constexpr inline void construct_single(Self<T>* const ptr,
    Args&& ... args) {
  new(ptr) T(std::forward<Args>(args)...);
}

template<typename T>
constexpr inline void destruct_single(Self<T>* const ptr) {
  ptr->~T();
}

template<typename T>
constexpr inline void destruct_single_void(void* const ptr) {
  destruct_single<T>(reinterpret_cast<T*>(ptr));
}

template<typename T, typename ... Args>
constexpr void reset_type(Self<T>* t, Args&& ... args) noexcept {
  t->~T();
  new(t) T(std::forward<Args>(args)...);
}

template<typename...>
struct DependentFalse {
  constexpr static bool VAL = false;
};

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

namespace Panic {
  [[noreturn]] void panic(const ViewArr<const char>& message) noexcept;
}

template<usize N>
[[nodiscard]] constexpr ViewArr<const char> lit_view_arr(const char(&arr)[N]) {
  ASSERT(arr[N - 1] == '\0');
  return {
    arr,
    N - 1,
  };
}

namespace Literals {
  constexpr Axle::ViewArr<const char> operator""_litview(const char* ptr, std::size_t N) {
    return { ptr, N };
  }
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
  static_assert(DependentFalse<T>::VAL, "Attempted to use unspecialized viewable");
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
  if constexpr (sizeof(T) >= sizeof(U)) {
    static_assert(sizeof(T) % sizeof(U) == 0);
    return { reinterpret_cast<T*>(arr.data), arr.size / sizeof(U) };
  }
  else {
    static_assert(sizeof(U) % sizeof(T) == 0);
    return { reinterpret_cast<T*>(arr.data), arr.size * sizeof(U) };
  }
}

template<typename T, usize size>
struct ConstArray {
  T data[size] = {};

  template<typename ... U>
  [[nodiscard]] constexpr static auto create(U&& ... u) {
    static_assert(sizeof...(U) == size, "Must be fully filled");

    return ConstArray{ {std::forward<U>(u)...} };
  }

  [[nodiscard]] constexpr const T* begin() const noexcept { return data; }
  [[nodiscard]] constexpr const T* end() const noexcept { return data + size; }
  [[nodiscard]] constexpr T* mut_begin() noexcept { return data; }
  [[nodiscard]] constexpr T* mut_end() noexcept { return data + size; }

  [[nodiscard]] constexpr T& operator[](const usize idx) noexcept {
    ASSERT(idx < size);
    return data[idx];
  }

  [[nodiscard]] constexpr const T& operator[](const usize idx) const noexcept {
    ASSERT(idx < size);
    return data[idx];
  }
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

template<typename T, usize size>
[[nodiscard]] constexpr ConstArray<T, size> fill_array(const T& t) {
  ConstArray<T, size> arr;
  FOR_MUT(arr, it) {
    *it = t;
  }
  return arr;
}

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

template<typename A>
struct ArraySize;

template<typename T>
[[nodiscard]] constexpr usize array_size(const T&) noexcept {
  return ArraySize<T>::SIZE;
}

template<typename T, usize N>
struct ArraySize<T[N]> {
  constexpr static usize SIZE = N;
};

template<typename T, usize N>
struct ArraySize<ConstArray<T, N>> {
  constexpr static usize SIZE = N;
};

template<typename RET, typename ... PARAMS>
using FUNCTION_PTR = RET(*)(PARAMS...);

template<typename T>
struct MEMBER {
  MEMBER() = delete;

  template<typename RET, typename ... PARAMS>
  using FUNCTION_PTR = RET(T::*)(PARAMS...);
};

template<typename T>
using DESTRUCTOR = FUNCTION_PTR<void, T>;

template<typename T>
constexpr inline DESTRUCTOR<T> get_destructor() {
  return &destruct_single<T>;
}

template<typename T>
struct EXECUTE_AT_END {
  T t;

  constexpr EXECUTE_AT_END(T&& t_) : t(std::move(t_)) {}

  ~EXECUTE_AT_END() noexcept(false) {
    t();
  }
};

template<typename T>
EXECUTE_AT_END(T&& t) -> EXECUTE_AT_END<T>;

#define DEFER(...) Axle::EXECUTE_AT_END JOIN(defer, __LINE__) = [__VA_ARGS__]() mutable ->void 

#define DO_NOTHING ((void)0)

template<typename T, typename U>
struct IS_SAME_TYPE_IMPL {
  constexpr static bool test = false;
};

template<typename T>
struct IS_SAME_TYPE_IMPL<T, T> {
  constexpr static bool test = true;
};

template<typename T, typename U>
concept IS_SAME_TYPE = IS_SAME_TYPE_IMPL<T, U>::test;

namespace _iMPL_A_can_cast_to_B {
  template<typename T>
  struct TEST_TRUE {
    static constexpr bool val = true;
  };

  struct TEST_FALSE {
    static constexpr bool val = false;
  };

  template<typename A, typename B>
  auto test_overload(const B* b) -> TEST_TRUE<decltype(static_cast<const A*>(b))>;

  template<typename A>
  auto test_overload(const void* v) -> TEST_FALSE;
}

template<typename A, typename B>
constexpr bool A_can_cast_to_B = decltype(_iMPL_A_can_cast_to_B::test_overload<A>(static_cast<const B*>(nullptr)))::val;

template<typename T, typename ... Ops>
concept OneOf = (IS_SAME_TYPE<T, Ops> || ...);
}
#endif
