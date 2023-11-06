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
void reset_type(T* t) noexcept {
  t->~T();
  new(t) T();
}


#ifdef COUNT_ALLOC
template<typename T>
void free_heap_check(T* ptr, size_t num_bytes) {
  const uint8_t* ptr_end = (const uint8_t*)ptr + num_bytes;

  for (size_t i = 0; i < 4; i++) {
    ASSERT(ptr_end[i] == 0xFD);
  }
}

struct ALLOC_COUNTER {
  using DESTRUCTOR = void (*)(void*, size_t);

  struct Allocation {
    const char* type_name;
    const void* mem;
    size_t element_size;
    size_t count;
    DESTRUCTOR destruct_arr;
  };

  Allocation* allocs = nullptr;
  size_t num_allocs = 0;
  size_t capacity = 0;

  size_t current_allocated_size = 0;

  size_t max_allocated_blocks = 0;
  size_t max_allocated_size   = 0;

  size_t update_calls = 0;
  size_t null_remove_calls = 0;
  size_t valid_remove_calls = 0;
  size_t insert_calls = 0;

  inline void reset() {
    allocs = nullptr;
    num_allocs = 0;
    capacity = 0;

    current_allocated_size = 0;

    max_allocated_blocks = 0;
    max_allocated_size   = 0;

    update_calls = 0;
    null_remove_calls = 0;
    valid_remove_calls = 0;
    insert_calls = 0;
  }

  template<typename T>
  void insert(T* t, size_t num) {
    insert_calls++;

    if (capacity == num_allocs) {
      if (capacity == 0) {
        capacity = 8;
      }
      else {
        capacity <<= 1;
      }


      auto* new_allocs = (Allocation*)std::realloc(allocs, capacity * sizeof(Allocation));
      ASSERT(new_allocs != nullptr);

      allocs = new_allocs;
    }

    allocs[num_allocs].type_name = typeid(T).name();
    allocs[num_allocs].mem  = (const void*)t;
    allocs[num_allocs].element_size = sizeof(T);
    allocs[num_allocs].count = num;
    allocs[num_allocs].destruct_arr = (DESTRUCTOR)&destruct_arr<T>;

    num_allocs++;
    if (num_allocs > max_allocated_blocks) {
      max_allocated_blocks = num_allocs;
    }

    current_allocated_size += num * sizeof(T);
    if (current_allocated_size > max_allocated_size) {
      max_allocated_size = current_allocated_size;
    }
  }

  template<typename T>
  void update(T* from, T* to, size_t num) {
    if (from == nullptr) {
      insert<T>(to, num);
      return;
    }

    update_calls++;

    const void* f_v = (const void*)from;

    auto i = allocs;
    const auto end = allocs + num_allocs;

    for (; i < end; i++) {
      if (i->mem == f_v) {
        i->mem = (const void*)to;

        current_allocated_size -= (i->count * i->element_size);
        i->count = num;
        current_allocated_size += (i->count * i->element_size);

        if (current_allocated_size > max_allocated_size) {
          max_allocated_size = current_allocated_size;
        }

        return;
      }
    }

    INVALID_CODE_PATH("Tried to update something that wasnt allocated");
  }

  //This is an unordered remove
  void remove_single(Allocation* i) {
    num_allocs--;
    const auto end = allocs + num_allocs;

    //Remove it by moving it to the end of the array
    //then shortening the array

    //Dont need to do anything if "i" is already at the end
    if (i != end) {

      //Swap "end" with "i"
      std::swap(*i, *end);
    }
  }

  template<typename T>
  void remove(T* t) {
    if (t == nullptr) {
      null_remove_calls++;
      return;
    }

    valid_remove_calls++;

    const void* t_v = (void*)t;

    auto i = allocs;
    const auto end = allocs + num_allocs;

    for (; i < end; i++) {
      if (i->mem == t_v) {
        free_heap_check(t, i->count * i->element_size);

        current_allocated_size -= (i->count * i->element_size);
        remove_single(i);
        return;
      }
    }

    INVALID_CODE_PATH("Freed something that wasnt allocated");
  }


  static ALLOC_COUNTER& allocated() {
    static ALLOC_COUNTER allocated_s ={};

    return allocated_s;
  }
};
#endif

template<typename T>
T* allocate_default(const size_t num) {
  T* t = (T*)std::malloc(sizeof(T) * num);

  ASSERT(t != nullptr);

#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().insert(t, num);
#endif

  default_init(t, num);
  return t;
}

template<typename T>
inline T* allocate_default() {
  return allocate_default<T>(1);
}

template<typename T, typename ... U>
T* allocate_single_constructed(U&& ... u) {
  T* t = (T*)std::malloc(sizeof(T));

  ASSERT(t != nullptr);

#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().insert(t, 1);
#endif

  new(t) T(std::forward<U>(u)...);
  return t;
}

template<typename T>
T* reallocate_default(T* ptr, const size_t old_size, const size_t new_size) {
  T* val = (T*)std::realloc((void*)ptr, sizeof(T) * new_size);
  ASSERT(val != nullptr);

  if (old_size < new_size) {
    default_init(val + old_size, new_size - old_size);
  }

#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().update(ptr, val, new_size);
#endif

  return val;
}

template<typename T>
void free_destruct_single(T* ptr) {
#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().remove(ptr);
#endif

  if (ptr == nullptr) return;

  ptr->~T();
  std::free((void*)ptr);
}

template<typename T>
void free_destruct_n(T* ptr, size_t num) {
#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().remove(ptr);
#endif

  if (ptr == nullptr) return;

  for (size_t i = 0; i < num; i++) {
    ptr[i].~T();
  }

  std::free((void*)ptr);
}

template<typename T>
void free_no_destruct(T* ptr) {
#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().remove(ptr);
#endif

  if (ptr == nullptr) return;

  std::free((void*)ptr);
}

constexpr size_t strlen_ts(const char* c) {
  const char* const base = c;
  while (*c != '\0') { c++; }

  return c - base;
}

#define TODO() static_assert(false, "Code is broken")

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


template<typename T, size_t size>
struct ConstArray {
  T data[size];

  template<typename ... U>
  constexpr static auto create(U&& ... u) {
    static_assert(sizeof...(U) == size, "Must be fully filled");

    return { {std::forward<U>(u)...} };
  }

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }
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