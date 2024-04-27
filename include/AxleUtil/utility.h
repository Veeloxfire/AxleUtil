#ifndef AXLEUTIL_UTILITY_H_
#define AXLEUTIL_UTILITY_H_

#include <utility>
#include <new>

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/threading.h>
#include <AxleUtil/formattable.h>
#include <AxleUtil/memory.h>
#include <AxleUtil/serialize.h>

namespace Axle {

constexpr inline u64 MAX_DECIMAL_U64_DIGITS = sizeof("18446744073709551615") - 1;

constexpr u64 greatest_common_divisor(u64 v1, u64 v2) {
  //Swap to be the correct way around
  if (v1 > v2) {
    const u64 temp = v1;
    v1 = v2;
    v2 = temp;
  }

  while (v2 != 0) {
    const u64 temp = v1 % v2;
    v1 = v2;
    v2 = temp;
  }

  return v1;
}

constexpr u64 lowest_common_multiple(u64 v1, u64 v2) {
  const u64 gcd = greatest_common_divisor(v1, v2);

  return (v1 * v2) / gcd;
}

constexpr inline u64 FNV1_HASH_BASE = 0xcbf29ce484222325;
constexpr inline u64 FNV1_HASH_PRIME = 0x100000001b3;

constexpr uint64_t fnv1a_hash(const char* c, size_t size) {
  uint64_t base = FNV1_HASH_BASE;

  while (size > 0) {
    base ^= *c;
    base *= FNV1_HASH_PRIME;

    c++;
    size--;
  }

  return base;
}

constexpr u64 fnv1a_hash_u16(u64 start, u16 u) {
  //1
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //2
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;

  return start;
}

constexpr u64 fnv1a_hash_u32(u64 start, u32 u) {
  //1
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //2
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //3
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //4
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;

  return start;
}

constexpr u64 fnv1a_hash_u64(u64 start, u64 u) {
  //1
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //2
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //3
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //4
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //5
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //6
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //7
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;
  u >>= 8;

  //8
  start ^= (u & 0xff);
  start *= FNV1_HASH_PRIME;

  return start;
}

template<typename T, typename B>
constexpr T bit_fill_lower(B bits) {
  if (bits == 0) return 0;

  if (bits > sizeof(T) * 8) bits = sizeof(T) * 8;
  constexpr uint64_t MAX_SHIFT = 64;

  return static_cast<uint64_t>(-1) >> (MAX_SHIFT - bits);
}

template<typename T, typename B>
constexpr T bit_fill_upper(B bits) {
  return ~bit_fill_lower<T, B>((sizeof(T) * 8) - bits);
}

constexpr size_t ceil_div(size_t x, size_t y) noexcept {
  return x / y + (x % y != 0);
}

constexpr uint64_t ceil_to_pow_2(uint64_t v) {
  //See: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  ASSERT(v != 0);
  ASSERT(v <= (1llu << 63llu));

  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;

  return v;
}

//Log 2 for uniform random 64 bit number
constexpr inline uint64_t log_2(uint64_t v) {
  //TODO: fancy bitwise stuff

  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  uint64_t max = 64;
  uint64_t min = 0;

  while (max > min + 1) {
    uint64_t mid = min + ((max - min) / 2);
    uint64_t guess = 1llu << mid;

    if (guess == v) {
      return mid;
    }
    else if (guess < v) {
      min = mid;
    }
    else {
      max = mid;
    }
  }

  return min;
}

constexpr inline uint64_t pow_16(uint64_t v) {
  return (1ull << (4ull * v));
}

constexpr inline uint64_t pow_10(uint64_t v) {
  if (v > 19) {
    INVALID_CODE_PATH("Power too high!");
  }

  constexpr uint64_t pow10[] = {
    1ull,
    10ull,
    100ull,
    1000ull,
    10000ull,
    100000ull,
    1000000ull,
    10000000ull,
    100000000ull,
    1000000000ull,
    10000000000ull,
    100000000000ull,
    1000000000000ull,
    10000000000000ull,
    100000000000000ull,
    1000000000000000ull,
    10000000000000000ull,
    100000000000000000ull,
    1000000000000000000ull,
    10000000000000000000ull
  };

  return pow10[v];
}

constexpr inline uint64_t log_10_floor(uint64_t v) {
  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  //Max uint64_t = 18446744073709551615
  //Max pow 10   = 10000000000000000000ull

  uint64_t max = 20;
  uint64_t min = 0;

  while (max > min + 1) {
    uint64_t mid = min + ((max - min) / 2);
    uint64_t guess = pow_10(mid);

    if (guess == v) {
      return mid;
    }
    else if (guess < v) {
      min = mid;
    }
    else {
      max = mid;
    }
  }

  return min;
}

//Log 2 optimised for small numbers
//Floors the output
constexpr inline uint64_t small_log_2_floor(uint64_t v) {
  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  uint64_t counter = 0;
  while (v > 1) {
    v >>= 1;
    counter += 1;
  }

  return counter;
}

constexpr inline uint64_t small_log_2_ceil(uint64_t v) {
  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  int found1 = 0;//max value of 1

  uint64_t counter = 0;
  while (v > 1) {
    found1 |= (v & 0b1);
    v >>= 1;
    counter += 1;
  }

  return counter + found1;
}

template<typename T>
constexpr T ceil_to_n(T val, T n) {
  const T raised = val + (n - 1);
  return raised - (raised % n);
}

template<typename T, usize N>
constexpr T ceil_to_N(T val) {
  const T raised = val + (N - 1);
  return raised - (raised % N);
}

template<typename T>
constexpr T ceil_to_8(T val) {
  return ceil_to_N<T, 8>(val);
}

template<typename T, typename L>
size_t _sort_range_part(T* base, size_t lo, size_t hi, const L& pred) {
  size_t pivot = (hi + lo) / 2llu;

  size_t i = lo;
  size_t j = hi;

  while (pred(base[i], base[pivot])) {
    i += 1;
  }

  while (pred(base[pivot], base[j])) {
    j -= 1;
  }

  if (i >= j) return j;

  //Account for the moving things
  //means we dont have to copy
  if (i == pivot) {
    pivot = j;
  }
  else if (j == pivot) {
    pivot = i;
  }

  {
    T hold = std::move(base[i]);

    base[i] = std::move(base[j]);
    base[j] = std::move(hold);
  }

  while (true) {
    do {
      i += 1;
    } while (pred(base[i], base[pivot]));

    do {
      j -= 1;
    } while (pred(base[pivot], base[j]));

    if (i >= j) return j;

    //Account for the moving things
    //means we dont have to copy
    if (i == pivot) {
      pivot = j;
    }
    else if (j == pivot) {
      pivot = i;
    }

    {
      T hold = std::move(base[i]);

      base[i] = std::move(base[j]);
      base[j] = std::move(hold);
    }
  }

}

template<typename T, typename L>
void _sort_range_impl(T* base, size_t lo, size_t hi, const L& pred) {
  if (lo < hi) {
    size_t p = _sort_range_part(base, lo, hi, pred);
    _sort_range_impl(base, lo, p, pred);
    _sort_range_impl(base, p + 1, hi, pred);
  }
}

template<typename T, typename L>
void sort_range(T* start, T* end, const L& pred) {
  size_t num = (end - start);

  if (num != 0) {
    _sort_range_impl<T, L>(start, 0, num - 1, pred);
  }
}

template<typename T>
struct Array {
  T* data = nullptr;// ptr to data in the array
  size_t size = 0;// used size
  size_t capacity = 0;

  [[nodiscard]] constexpr T& operator[](size_t index) const {
    ASSERT(index < size);
    return data[index];
  }

  //No copy!
  constexpr Array(const Array&) noexcept = delete;

  constexpr Array() noexcept = default;
  constexpr Array(Array&& arr) noexcept : data(arr.data), size(arr.size), capacity(arr.capacity)
  {
    arr.data = nullptr;
    arr.size = 0;
    arr.capacity = 0;
  }

  //Array(size_t s) noexcept : data(allocate_default<T>(s)), size(s), capacity(s) {}

  Array& operator=(Array&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0);
    capacity = std::exchange(arr.capacity, 0);

    return *this;
  }

  void free() {
    free_destruct_n<T>(data, size);
    data = nullptr;
    size = 0;
    capacity = 0;
  }

  ~Array() noexcept {
    free();
  }

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }

  constexpr T* back() { return data + size - 1; }

  constexpr T* mut_begin() { return data; }
  constexpr T* mut_end() { return data + size; }

  T remove_at(const size_t index) {
    ASSERT(index < size);

    T t = std::move(data[index]);

    for (size_t i = index; i < size - 1; i++) {
      data[i] = std::move(data[i + 1]);
    }
    size--;
    return t;
  }

  void insert_at_internal(const size_t index, T t) {
    ASSERT(index <= size);
    reserve_extra(1);

    size++;
    for (size_t i = size; (i - 1) > index; i--) {
      data[i - 1] = std::move(data[i - 2]);
    }

    data[index] = std::move(t);
  }

  void insert_at(const size_t index, T&& t) {
    insert_at_internal(index, std::move(t));
  }

  template<typename L>
  void remove_if(L&& lambda) {
    size_t num_removed = 0;

    for (size_t i = 0; i < size; i++) {
      if (lambda(data[i])) {
        [[maybe_unused]]T removed = std::move(data[i]);
        num_removed++;
      }
      else {
        if (num_removed > 0) {
          data[i - num_removed] = std::move(data[i]);
        }
      }
    }
    size -= num_removed;
  }

  template<typename L>
  [[nodiscard]] constexpr bool any_of(L&& lambda) const {
    auto i = begin();
    const auto i_end = end();

    for (; i < i_end; i++) {
      if (lambda(i)) {
        return true;
      }
    }

    return false;
  }

  template<typename L>
  [[nodiscard]] constexpr const T* find_if(L&& lambda) const {
    auto i = begin();
    const auto i_end = end();

    for (; i < i_end; i++) {
      if (lambda(i)) {
        return i;
      }
    }

    return nullptr;
  }

  constexpr void replace_a_with_b(const T& a, const T& b) {
    for (size_t i = 0; i < size; i++) {
      if (data[i] == a) {
        data[i] = b;
      }
    }
  }

  void insert_internal(T t) noexcept {
    try_reserve_next(size + 1);

    new(data + size) T(std::move(t));
    size++;
  }

  void insert(T&& t) noexcept {
    //need this to stop reallocations breaking things
    insert_internal(std::move(t));
  }

  void insert(const T& t) noexcept {
    //need this to stop reallocations breaking things
    insert_internal(t);
  }

  //TODO: rename intert_default
  void insert_uninit(const size_t num = 1) noexcept {
    if (num > 0) {
      reserve_extra(num);

      default_init<T>(data + size, num);
      size += num;
    }
  }

  //Test for extra space after the size (not capacity)
  void reserve_extra(const size_t extra) noexcept {
    try_reserve_next(size + extra);
  }

  //Test for a total needed space
  void reserve_total(const size_t total) noexcept {
    try_reserve_next(total);
  }

  void try_reserve_next(const size_t total_required) noexcept {
    if (total_required < capacity) return;

    const size_t prev = capacity;

    //Min capacity should be 8
    if (total_required < 8) {
      capacity = 8;
    }
    else {
      capacity = ceil_to_pow_2(total_required);
    }

    data = reallocate_default<T>(data, prev, capacity);
  }

  void shrink() noexcept {
    if (size < capacity) {
      if (size == 0) {
        this->free();
      }
      else {
        size_t old_cap = capacity;
        capacity = size;
        data = reallocate_default<T>(data, old_cap, capacity);
      }
    }
  }

  void clear() noexcept {
    auto i = mut_begin();
    auto end = mut_end();

    for (; i < end; i++) {
      i->~T();
    }

    size = 0;
  }

  void pop() noexcept {
    ASSERT(size > 0);
    size--;
    (data + size)->~T();
  }

  T take() noexcept {
    T t = std::move(data[size - 1]);
    pop();
    return t;
  }

  void pop_n(size_t num) noexcept {
    ASSERT(num <= size);
    const auto* old_end = data + size;

    if (num > size) {
      size = 0;
    }
    else {
      size -= num;
    }

    auto* i = data + size;

    for (; i < old_end; i++) {
      i->~T();
    }
  }

  [[nodiscard]] constexpr bool contains(const T& t) const noexcept {
    auto i = begin();
    const auto end_i = end();

    for (; i < end_i; i++) {
      if (*i == t) {
        return true;
      }
    }

    return false;
  }

  void concat_move(T* arr, const size_t N) noexcept {
    ASSERT(!(data <= arr && arr < (data + capacity)));
    reserve_extra(N);

    T* start = data + size;
    for(usize i = 0; i < N; ++i) {
      *start = std::move(*arr);
      start += 1;
      arr += 1;
    }

    size += N;
  }

  void concat(Array<T>&& arr) noexcept {
    ASSERT(&arr != this);
    concat_move(arr.data, arr.size);
    arr.free();
  }

  void concat(const T* arr, const size_t N) noexcept {
    reserve_extra(N);

    T* start = data + size;

    for (usize i = 0; i < N; ++i) {
      *start = *arr;
      start += 1;
      arr += 1;
    }

    size += N;
  }

  void concat(const ViewArr<const T>& arr) noexcept {
    concat(arr.data, arr.size);
  }
};

namespace Format {
  template<>
  struct FormatArg<Array<char>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const Array<char>& str) {
      res.load_string(str.data, str.size);
    }
  };

  template<>
  struct FormatArg<Array<const char>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const Array<const char>& str) {
      res.load_string(str.data, str.size);
    }
  };
}

template<typename T>
struct Viewable<Array<T>> {
  using ViewT = T;

  template<typename U>
  static constexpr ViewArr<U> view(const Array<T>& t) {
    return {t.data, t.size};
  }
};


template<>
struct Serializer<Array<u8>> {
  Array<u8>& bytes;

  constexpr Serializer(Array<u8>& arr) : bytes(arr) {}

  void write_bytes(const ViewArr<const u8>& data) {
    bytes.concat(data);
  }
};

template<typename T>
void copy_arr_to(const Array<T>& from, Array<T>& to) noexcept {
  to.clear();
  to.reserve_total(from.size);

  for (size_t i = 0; i < from.size; i++) {
    to.data[i] = from.data[i];
  }

  to.size += from.size;
}

template<typename T>
Array<T> copy_arr(const Array<T>& from) noexcept {
  Array<T> to = {};
  copy_arr_to(from, to);
  return to;
}

template<typename T>
void combine_unique(const Array<T>& from, Array<T>& to) noexcept {
  auto i = from.begin();
  const auto end = from.end();

  const size_t initial_size = to.size;
  for (; i < end; i++) {
    for (size_t i_to = 0; i_to < initial_size; i_to++) {
      if (*i == to.data[i_to]) {
        goto NEXT_COMBINE;
      }
    }

    to.insert(*i);

  NEXT_COMBINE:
    continue;
  }
}

template<typename T>
void reverse_array(Array<T>& arr) noexcept {
  if (arr.size == 0) { return; }

  auto* i_beg = arr.data;
  auto* i_back = arr.data + arr.size - 1;

  //Swap first with back - then step inwards
  while (i_beg < i_back) {
    T temp = std::move(*i_beg);
    *i_beg = std::move(*i_back);
    *i_back = std::move(temp);

    i_beg++;
    i_back--;
  }
}

template<typename T>
struct BucketArray {
  struct BLOCK {
    union EL {
      char _unused;
      T el;

      EL() : _unused(0) {}
      ~EL() {}
    };

    constexpr static size_t BLOCK_SIZE = 64;

    size_t filled = 0;
    BLOCK* next = nullptr;

    EL data[BLOCK_SIZE];
  };

  struct Iter {
    size_t index = 0;
    BLOCK* block = nullptr;
    
    constexpr void next() {
      if(block == nullptr) return;

      index++;
      if (index >= block->filled) {
        index = 0;
        block = block->next;
      }
    }

    constexpr T* get() const {
      if(block == nullptr) return nullptr;
      else {
        ASSERT(block->filled > index);
        return &block->data[index].el;
      }
    }

    constexpr Iter& operator++() {
      next();
      return *this;
    }

    constexpr T& operator*() const {
      return *get();
    }

    constexpr T* operator->() const {
      return get();
    }

    constexpr bool operator!=(const Iter& i) const {
      return (index != i.index) || (block != i.block);
    }
    constexpr bool operator==(const Iter& i) const {
      return (index == i.index) && (block == i.block);
    }
  };

  struct ConstIter {
    size_t index = 0;
    const BLOCK* block = nullptr;

    constexpr void next() {
      if(block == nullptr) return;

      index++;
      if (index >= block->filled) {
        index = 0;
        block = block->next;
      }
    }

    constexpr const T* get() const {
      if(block == nullptr) return nullptr;
      else {
        ASSERT(block->filled > index);
        return &block->data[index].el;
      }
    }

    constexpr ConstIter& operator++() {
      next();
      return *this;
    }

    constexpr const T& operator*() const {
      return *get();
    }

    constexpr const T* operator->() const {
      return get();
    }
    constexpr bool operator!=(const ConstIter& i) const {
      return (index != i.index) || (block != i.block);
    }
    constexpr bool operator==(const ConstIter& i) const {
      return (index == i.index) && (block == i.block);
    }
  };

  Iter mut_begin() {
    return { 0, first };
  }

  Iter mut_end() {
    return {};
  }

  ConstIter begin() const {
    return { 0, first };
  }

  ConstIter end() const {
    return {};
  }

  BLOCK* first = nullptr;
  BLOCK* last = nullptr;

  ~BucketArray() {
    BLOCK* b = first;
    while(b != nullptr) {
      BLOCK* next = b->next;
      for(usize i = 0; i < b->filled; ++i) {
        b->data[i].el.~T();
      }
      free_destruct_single<BLOCK>(b);
      b = next;
    }
  }

  template<typename ... U>
  T* insert(U&& ... u) {
    if(last == nullptr) {
      first = allocate_default<BLOCK>();
      last = first;

      first->next = nullptr;
      first->filled = 0;
    }
    else if (last->filled == BLOCK::BLOCK_SIZE) {
      last->next = allocate_default<BLOCK>();
      last = last->next;

      last->next = nullptr;
      last->filled = 0;
    }

    T* el = &last->data[last->filled].el;
    new (el) T(std::forward<U>(u)...);
    last->filled++;

    return el;
  }
};

template<typename T>
struct FreelistBlockAllocator {
  struct Element {
    union {
      Element* next = nullptr;
      T el;
    };
    ~Element() {}
  };

  struct BLOCK {
    constexpr static size_t BLOCK_SIZE = 32;

    //size_t filled = 0;
    BLOCK* prev = nullptr;
    Element data[BLOCK_SIZE] = {};
  };

  BLOCK* top = nullptr;
  Element* alloc_list = nullptr;

  FreelistBlockAllocator() = default;
  FreelistBlockAllocator(const FreelistBlockAllocator&) = delete;
  FreelistBlockAllocator(FreelistBlockAllocator&&) = delete;

  ~FreelistBlockAllocator()
#ifdef ASSERT_EXCEPTIONS
    noexcept(false) 
#endif
  {
    ASSERT(_debug_all_are_free());

    while (top != nullptr) {
      BLOCK* next = top->prev;
      free_destruct_single<BLOCK>(top);
      top = next;
    }

    top = nullptr;
    alloc_list = nullptr;
  }

  void new_block() {
    BLOCK* const new_b = allocate_default<BLOCK>();

    new_b->prev = top;
    top = new_b;

    top->data[0].next = top->data + 1;

    for (usize i = 1; i < BLOCK::BLOCK_SIZE - 1; i++) {
      Element& e = top->data[i];
      e.next = top->data + i + 1;
    }

    top->data[BLOCK::BLOCK_SIZE - 1].next = alloc_list;

    alloc_list = top->data;
  }

  T* allocate() {
    if (alloc_list == nullptr) {
      new_block();
    }

    Element* e = alloc_list;
    alloc_list = e->next;

    T* const new_t = &e->el;
    new(new_t) T();

    return new_t;
  }

  bool _debug_valid_free_ptr(const T* const t) const {
    const Element* e = alloc_list;
    while (e != nullptr) {
      if (&e->el == t) return false;
      e = e->next;
    }

    const BLOCK* b = top;
    while (b != nullptr) {
      const u8* block_base = reinterpret_cast<const u8*>(b->data);
      const u8* block_top = reinterpret_cast<const u8*>(b->data + BLOCK::BLOCK_SIZE);

      const u8* t_base = reinterpret_cast<const u8*>(t);
      const u8* t_top = reinterpret_cast<const u8*>(t + 1);
      if (block_base <= t_base && t_top <= block_top) return true;

      b = b->prev;
    }

    return false;
  }

  bool _debug_all_are_free() const {
    usize actual = 0;
    const Element* e = alloc_list;
    while (e != nullptr) {
      actual += 1;
      e = e->next;
    }

    usize expected = 0;
    const BLOCK* b = top;
    while (b != nullptr) {
      expected += BLOCK::BLOCK_SIZE;
      b = b->prev;
    }

    return actual == expected;
  }


  void free(const T* t) {
    ASSERT(_debug_valid_free_ptr(t));

    destruct_single<const T>(t);

    Element* new_e = const_cast<Element*>(reinterpret_cast<const Element*>(t));
    new_e->next = alloc_list;

    alloc_list = new_e;
  }
};

struct SquareBitMatrix {
  //Per block: ceil(side_length / 8) bytes = (side_length / 8) + 1

  //Block length = (side_length / 8) + 1
  //Block pointer = data + (Block length * val)
  //each bit corresponds to its equivalent value at that bit index

  uint8_t* data = nullptr;
  size_t side_length = 0;
  size_t capacity = 0;

  void free();

  constexpr SquareBitMatrix() = default;
  ~SquareBitMatrix();


  //TEMP
  SquareBitMatrix(SquareBitMatrix&&) = delete;
  SquareBitMatrix(const SquareBitMatrix&) = delete;

  constexpr static size_t bytes_per_val_per_side(size_t side_length) {
    return side_length == 0 ? 0
      : ((side_length - 1) / 8) + 1;
  }

  bool test_a_intersects_b(size_t a, size_t b) const;
  void set_a_intersects_b(size_t a, size_t b);
  void remove_a_intersects_b(size_t a, size_t b);

  size_t new_value();
};

struct BitArray {
  uint8_t* data;
  size_t length;
  size_t highest_set;

  constexpr BitArray() : data(nullptr), length(0), highest_set(0) {}
  BitArray(size_t length);
  BitArray(BitArray&&) noexcept;
  BitArray& operator=(BitArray&&) noexcept;

  BitArray(const BitArray&) = delete;
  BitArray& operator=(const BitArray&) = delete;
  ~BitArray();


  void set(size_t a);
  bool test(size_t a) const;

  bool intersects(const BitArray& b) const;
  bool test_all() const;

  usize count_set() const;
  usize count_unset() const;


  struct UnsetBitItr {
    const u8* data;
    usize index;
    usize length;

    usize next();
  };  

  UnsetBitItr unset_itr() const;

  void clear();
};

template<typename T>
struct Queue {
  T* holder;
  usize start;
  usize size;
  usize capacity;

  //No copy!
  Queue(const Queue&) = delete;

  Queue(Queue&& q) noexcept : holder(q.holder), start(q.start), size(q.size), capacity(q.capacity)
  {
    q.holder = nullptr;
    q.start = 0;
    q.size = 0;
    q.capacity = 0;
  }

  Queue() noexcept = default;

  Queue& operator=(Queue&& q) noexcept {
    if(&q == this) return *this;
    free();

    holder = std::exchange(q.holder, nullptr);
    start = std::exchange(q.start, 0);
    size = std::exchange(q.size, 0);
    capacity = std::exchange(q.capacity, 0);

    return *this;
  }

  void free() {
    if (start + size > capacity) {
      usize temp = capacity - start;
      destruct_arr<T>(holder + start, temp);
      destruct_arr<T>(holder + start, size - temp);
    }
    else {
      destruct_arr<T>(holder + start, size);
    }

    free_no_destruct<T>(holder);
    holder = nullptr;
    start = 0;
    size = 0;
    capacity = 0;
  }

  ~Queue() noexcept {
    free();
  }

  //Does the index taking into account wrapping
  usize _ptr_index(usize i) const {
    return (start + i) % capacity;
  }

  void push_front(T t) {
    if (size == capacity) {
      extend();
    }

    if (start == 0) {
      start = capacity - 1;
    }
    else {
      start--;
    }
    size++;

    new(holder + start) T(std::move(t));
  }

  T pop_front() {
    ASSERT(size > 0);

    T val = std::move(holder[start]);
    start++;
    size--;

    if (start >= capacity) {
      start -= capacity;
    }

    return val;
  }

  void _internal_push_back(T u) {
    if (size == capacity) {
      extend();
    }

    new(holder + _ptr_index(size)) T(std::move(u));
    size++;
  }

  void push_back(const T& t) {
    _internal_push_back(t);
  }

  void push_back(T&& t) {
    _internal_push_back(std::move(t));
  }

  T pop_back() {
    ASSERT(size > 0);

    size--;
    T val = std::move(holder[_ptr_index(size)]);

    return val;
  }

  void extend() {
    if (capacity == 0) {
      holder = allocate_default<T>(8);
      capacity = 8;
      return;
    }

    usize new_cap = capacity << 1;

    T* new_holder = allocate_default<T>(new_cap);

    usize i = 0;
    usize end_i = size + 1;
    for (; i < end_i; i++) {
      new_holder[i] = std::move(holder[_ptr_index(i)]);
    }

    free_no_destruct<T>(holder);
    holder = new_holder;
    capacity = new_cap;

    start = 0;
  }

  void shrink() {
    if (size == 0) {
      free();
      return;
    }

    usize new_cap = size;

    T* new_holder = allocate_default<T>(new_cap);

    usize i = 0;
    usize end_i = size + 1;
    for (; i < end_i; i++) {
      new_holder[i] = std::move(holder[_ptr_index(i)]);
    }

    free_no_destruct(holder);
    holder = new_holder;
    capacity = new_cap;

    start = 0;
  }

  void clear() {
    if (start + size > capacity) {
      usize temp = capacity - start;
      destruct_arr<T>(holder + start, temp);
      destruct_arr<T>(holder + start, size - temp);
    }
    else {
      destruct_arr<T>(holder + start, size);
    }

    start = 0;
    size = 0;
  }
};

template<typename T>
struct AtomicQueue {
  SpinLockMutex mutex;

  T* holder;
  usize start;
  usize size;
  usize capacity;

  //No copy!
  AtomicQueue(const AtomicQueue&) = delete;

  AtomicQueue() noexcept = default;

  void free() {
    if (start + size > capacity) {
      usize temp = capacity - start;
      destruct_arr<T>(holder + start, temp);
      destruct_arr<T>(holder + start, size - temp);
    }
    else {
      destruct_arr<T>(holder + start, size);
    }

    free_no_destruct<T>(holder);
    holder = nullptr;
    start = 0;
    size = 0;
    capacity = 0;
  }

  ~AtomicQueue() noexcept {
    free();
  }

  //Does the index taking into account wrapping
  usize _ptr_index(usize i) const {
    return (start + i) % capacity;
  }

  //returns true if there is a value in out_t
  bool try_pop_front(T* out_t) {
    bool acquired = mutex.acquire_if_free();
    if (!acquired) return false;

    if (size == 0) {
      mutex.release();
      return false;
    }

    size -= 1;
    *out_t = std::move(holder[start]);
    start++;
    start %= capacity;

    mutex.release();
    return true;
  }

  void push_back(T t) {
    mutex.acquire();
    if (size == capacity) {
      extend();
    }

    size += 1;

    new(holder + _ptr_index(size - 1)) T(std::move(t));
    mutex.release();
  }

  void extend() {
    if (capacity == 0) {
      holder = allocate_default<T>(8);
      capacity = 8;
      return;
    }

    usize new_cap = capacity << 1;

    T* new_holder = allocate_default<T>(new_cap);

    usize i = 0;
    usize end_i = size + 1;
    for (; i < end_i; i++) {
      new_holder[i] = std::move(holder[_ptr_index(i)]);
    }

    free_no_destruct<T>(holder);
    holder = new_holder;
    capacity = new_cap;

    start = 0;
  }
};

template<typename T>
struct OwnedArr {
  T* data = nullptr;
  usize size = 0;

  constexpr OwnedArr() noexcept = default;
  constexpr OwnedArr(T* t, usize s) noexcept : data(t), size(s) {}
  constexpr OwnedArr(OwnedArr&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0))
  {}

  OwnedArr(const OwnedArr& arr) = delete;
  OwnedArr& operator=(const OwnedArr& arr) = delete;

  void free() {
    free_destruct_n<T>(data, size);
    data = nullptr;
    size = 0;
  }

  ~OwnedArr() noexcept {
    free();
  }

  constexpr OwnedArr& operator=(OwnedArr&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0);

    return *this;
  }

  constexpr T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }

  constexpr const T* begin() const noexcept { return data; }
  constexpr const T* end() const noexcept { return data + size; }
  constexpr T* mut_begin() noexcept { return data; }
  constexpr T* mut_end() noexcept { return data + size; }
};

template<typename T>
struct OwnedArr<const T> {
  const T* data = nullptr;
  usize size = 0;

  constexpr OwnedArr() noexcept = default;
  constexpr OwnedArr(const T* t, usize s) noexcept : data(t), size(s) {}
  constexpr OwnedArr(OwnedArr<const T>&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0))
  {}

  constexpr OwnedArr(OwnedArr<T>&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0))
  {}

  OwnedArr(const OwnedArr& arr) = delete;
  OwnedArr& operator=(const OwnedArr& arr) = delete;

  void free() {
    free_destruct_n<const T>(data, size);
    data = nullptr;
    size = 0;
  }

  ~OwnedArr() noexcept {
    free();
  }

  constexpr OwnedArr<const T>& operator=(OwnedArr<const T>&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0);

    return *this;
  }

  constexpr OwnedArr<const T>& operator=(OwnedArr<T>&& arr) noexcept {
    ASSERT(arr.data != data);
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0);

    return *this;
  }

  constexpr const T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }

  constexpr const T* begin() const noexcept { return data; }
  constexpr const T* end() const noexcept { return data + size; }
};


namespace Format {
  template<>
  struct FormatArg<OwnedArr<const char>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const OwnedArr<const char>& str) {
      res.load_string(str.data, str.size);
    }
  };

  template<>
  struct FormatArg<OwnedArr<char>> {
    template<Formatter F>
    constexpr static void load_string(F& res, const OwnedArr<char>& str) {
      res.load_string(str.data, str.size);
    }
  };
}

template<typename T>
struct Viewable<OwnedArr<T>> {
  using ViewT = T;

  template<typename U>
  static constexpr ViewArr<U> view(const OwnedArr<T>& t) {
    return {t.data, t.size};
  }
};

template<typename T>
OwnedArr<T> new_arr(usize size) {
  T* arr = allocate_default<T>(size);
  return OwnedArr(arr, size);
}

template<typename T>
OwnedArr<T> copy_arr(const T* source, usize n) {
  T* arr = allocate_default<T>(n);

  for (usize i = 0; i < n; ++i) {
    arr[i] = source[i];
  }
  return OwnedArr<T>(arr, n);
}

template<typename T, usize N>
OwnedArr<T> copy_arr(const T(&source)[N]) {
  T* arr = allocate_default<T>(N);

  for (usize i = 0; i < N; ++i) {
    arr[i] = source[i];
  }
  return OwnedArr<T>(arr, N);
}

template<typename T>
OwnedArr<T> copy_arr(const ViewArr<const T>& in_arr) {
  return copy_arr(in_arr.data, in_arr.size);
}

template<typename T>
OwnedArr<T> copy_arr(const OwnedArr<T>& in_arr) {
  return copy_arr(in_arr.data, in_arr.size);
}

template<typename T>
OwnedArr<T> copy_bake_arr(const Array<T>& in_arr) {
  return copy_arr(in_arr.data, in_arr.size);
}

template<typename T>
OwnedArr<T> bake_arr(Array<T>&& arr) {
  arr.shrink();

  T* d = std::exchange(arr.data, nullptr);
  usize s = std::exchange(arr.size, 0);
  arr.capacity = 0;

  return OwnedArr<T>(d, s);
}

template<typename T>
OwnedArr<const T> bake_const_arr(Array<T>&& arr) {
  arr.shrink();

  T* d = std::exchange(arr.data, nullptr);
  usize s = std::exchange(arr.size, 0);
  arr.capacity = 0;

  return OwnedArr<const T>(d, s);
}

template<typename T, typename U>
OwnedArr<T> cast_arr(OwnedArr<U>&& arr) {
  static_assert(std::is_trivial_v<T> && std::is_trivial_v<U>);
  static_assert(sizeof(T) % sizeof(U) == 0);
  return { reinterpret_cast<T*>(std::exchange(arr.data, nullptr)), 
           std::exchange(arr.size, 0) / sizeof(U) };
}


template<typename T>
struct ArrayMax {
  T* data;
  usize size;
  usize capacity;

  constexpr ArrayMax() = default;
  constexpr ArrayMax(T* data_, usize size_, usize capacity_) noexcept 
    : data(data_), size(size_), capacity(capacity_)
  {}
  constexpr ArrayMax(ArrayMax&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0)),
    capacity(std::exchange(arr.capacity, 0))
  {}

  ArrayMax(const ArrayMax& arr) = delete;
  ArrayMax& operator=(const ArrayMax& arr) = delete;

  void free() {
    free_destruct_n<T>(data, size);
    data = nullptr;
    size = 0;
    capacity = 0;
  }

  ~ArrayMax() noexcept {
    free();
  }

  constexpr ArrayMax& operator=(ArrayMax&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0);
    capacity = std::exchange(arr.capacity, 0);

    return *this;
  }

  constexpr T& operator[](usize i) const {
    ASSERT(i < size);
    return data[i];
  }

  constexpr void insert_internal(T t) {
    ASSERT(size < capacity);

    new(data + size) T(std::move(t));
    size++;
  }

  constexpr void insert(const T& t) {
    insert_internal(t);
  }

  constexpr void insert(T&& t) {
    insert_internal(std::move(t));
  }

  //TODO: rename intert_default
  constexpr void insert_uninit(const size_t num = 1) {
    if (num > 0) {
      ASSERT(size + num <= capacity);

      default_init<T>(data + size, num);
      size += num;
    }
  }

  void insert_at(const size_t index, T t) {
    ASSERT(index <= size);
    ASSERT(size < capacity);

    size++;
    for (size_t i = size - 1; i > index; i--) {
      data[i] = std::move(data[i - 1]);
    }

    data[index] = std::move(t);
  }

  T remove_at(const size_t index) {
    ASSERT(index < size);

    T t = std::move(data[index]);

    for (size_t i = index; i < size - 1; i++) {
      data[i] = std::move(data[i + 1]);
    }
    size--;
    return t;
  }

  constexpr void pop() {
    ASSERT(size > 0);
    size--;
    (data + size)->~T();
  }

  constexpr void pop_n(size_t num) {
    ASSERT(num <= size);
    const T* old_end = data + size;

    if (num > size) {
      size = 0;
    }
    else {
      size -= num;
    }

    T* i = data + size;

    for (; i < old_end; i++) {
      i->~T();
    }
  }

  constexpr T take() {
    T t = std::move(data[size - 1]);
    pop();
    return t;
  }
  
  constexpr void clear() noexcept {
    auto i = mut_begin();
    auto end = mut_end();

    for (; i < end; i++) {
      i->~T();
    }

    size = 0;
  }

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }
  constexpr T* mut_begin() { return data; }
  constexpr T* mut_end() { return data + size; }

  constexpr T* back() const {
    ASSERT(size > 0);
    return data + (size - 1);
  }

  template<typename L>
  constexpr void remove_if(L&& lambda) {
    size_t num_removed = 0;

    for (size_t i = 0; i < size; i++) {
      if (lambda(data[i])) {
        [[maybe_unused]]T removed = std::move(data[i]);
        num_removed++;
      }
      else {
        if (num_removed > 0) {
          data[i - num_removed] = std::move(data[i]);
        }
      }
    }
    size -= num_removed;
  }
};

template<typename T>
ArrayMax<T> new_arr_max(usize capacity) {
  return {
    allocate_default<T>(capacity),
    0,
    capacity,
  };
}

template<typename T>
struct Viewable<ArrayMax<T>> {
  using ViewT = T;

  template<typename U>
  static constexpr ViewArr<U> view(const ArrayMax<T>& t) {
    return {t.data, t.size};
  }
};

template<typename T>
constexpr void set_mask(T& t, const T mask) {
  t |= mask;
}

template<typename T>
constexpr void unset_mask(T& t, const T mask) {
  t &= ~mask;
}

template<typename T>
constexpr bool test_mask(const T t, const T mask) {
  return (t & mask) == mask;
}

template<typename T>
constexpr T combine_flag(const T full, const T mask, const bool set) {
  return (full & ~mask) | (mask * set);
}

#define COMBINE_FLAG(full, mask, set) (Axle::combine_flag(full, mask, set))
#define SET_FLAG(full, mask, set) (full = COMBINE_FLAG(full, mask, set))

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
inline constexpr bool IS_SAME_TYPE = IS_SAME_TYPE_IMPL<T, U>::test;

#ifdef NDEBUG
#define assert_if(cond, expr) ((void)0)
#else
#define assert_if(cond, expression) if(cond) ASSERT(expression)
#endif

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
