#ifndef AXLEUTIL_UTILITY_H_
#define AXLEUTIL_UTILITY_H_

#include <utility>

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/threading.h>
#include <AxleUtil/formattable.h>
#include <AxleUtil/memory.h>
#include <AxleUtil/serialize.h>
#include <AxleUtil/math.h>

namespace Axle {

constexpr inline u64 FNV1_HASH_BASE = 0xcbf29ce484222325u;
constexpr inline u64 FNV1_HASH_PRIME = 0x100000001b3u;

constexpr uint64_t fnv1a_hash(const char* c, size_t size) {
  uint64_t base = FNV1_HASH_BASE;

  while (size > 0u) {
    base ^= *c;
    base *= FNV1_HASH_PRIME;

    c++;
    size--;
  }

  return base;
}

constexpr u64 fnv1a_hash_u16(u64 start, u16 u) {
  //1
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //2
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;

  return start;
}

constexpr u64 fnv1a_hash_u32(u64 start, u32 u) {
  //1
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //2
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //3
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //4
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;

  return start;
}

constexpr u64 fnv1a_hash_u64(u64 start, u64 u) {
  //1
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //2
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //3
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //4
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //5
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //6
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //7
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;
  u >>= 8u;

  //8
  start ^= (u & 0xffu);
  start *= FNV1_HASH_PRIME;

  return start;
}

template<typename T, typename U>
concept SortPredicate = requires(const T t, const U u0, const U u1) {
  { t(u0, u1) } -> IS_SAME_TYPE<std::strong_ordering>;
};

template<typename T, SortPredicate<T> L>
size_t _sort_range_part(const Axle::ViewArr<T>& base, size_t b, size_t e, const L& pred) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  ASSERT(e <= std::numeric_limits<size_t>::max());
  ASSERT(e > 0u);
  ASSERT(b < e && e - b > 1u);
  size_t pivot = (e + b + 1u) / 2llu;
  ASSERT(pivot > b);
  ASSERT(pivot < e);

  size_t i = b;
  size_t j = e;

  while (true) {
    while (i < e) {
      if (i == pivot) break;
      if (pred(base[i], base[pivot]) >= 0) break;
      i += 1u;
    }

    while (j > b) {
      if ((j - 1u) == pivot) break;
      if (pred(base[pivot], base[j - 1u]) >= 0) break;
      j -= 1u;
    }
    
    ASSERT(i < e);
    ASSERT(j > b);

    // pivoted correctly
    if (i >= (j - 1u)) {
      return i;
    }

    {
      ASSERT(i < (j - 1u));
      ASSERT(pred(base[i], base[j - 1u]) >= 0);
      T hold = std::move(base[i]);

      base[i] = std::move(base[j - 1u]);
      base[j - 1u] = std::move(hold);
    }

    // pivot value has moved
    if (i == pivot) {
      pivot = j - 1u;
    }
    else if (j - 1u == pivot) {
      pivot = i;
    }

    i += 1u;
    j -= 1u;
  }

}

template<typename T, SortPredicate<T> L>
void _sort_range_impl(const ViewArr<T>& view, size_t b, size_t e, const L& pred) {
  if (b + 1u < e) {
    const size_t p = _sort_range_part(view, b, e, pred);
    _sort_range_impl(view, b, p, pred);
    _sort_range_impl(view, p, e, pred);
  }
}

template<typename T, SortPredicate<T> L>
void sort_view(const Axle::ViewArr<T>& view, const L& pred) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  _sort_range_impl<T, L>(view, 0u, view.size, pred);
}

template<typename T>
struct Array {
  T* data = nullptr;// ptr to data in the array
  size_t size = 0u;// used size
  size_t capacity = 0u;

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
    arr.size = 0u;
    arr.capacity = 0u;
  }

  //Array(size_t s) noexcept : data(allocate_default<T>(s)), size(s), capacity(s) {}

  Array& operator=(Array&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0u);
    capacity = std::exchange(arr.capacity, 0u);

    return *this;
  }

  void free() {
    free_destruct_n<T>(data, size);
    data = nullptr;
    size = 0u;
    capacity = 0u;
  }

  ~Array() noexcept {
    free();
  }

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }

  constexpr T* back() { return data + size - 1u; }

  constexpr T* mut_begin() { return data; }
  constexpr T* mut_end() { return data + size; }

  T remove_at(const size_t index) {
    ASSERT(index < size);

    T t = std::move(data[index]);

    for (size_t i = index; i < size - 1u; i++) {
      data[i] = std::move(data[i + 1u]);
    }
    size--;
    return t;
  }

  void insert_at_internal(const size_t index, T t) {
    ASSERT(index <= size);
    reserve_extra(1u);

    size++;
    for (size_t i = size; (i - 1u) > index; i--) {
      data[i - 1] = std::move(data[i - 2u]);
    }

    data[index] = std::move(t);
  }

  void insert_at(const size_t index, T&& t) {
    insert_at_internal(index, std::move(t));
  }

  template<typename L>
  void remove_if(L&& lambda) {
    size_t num_removed = 0u;

    for (size_t i = 0u; i < size; i++) {
      if (lambda(data[i])) {
        [[maybe_unused]]T removed = std::move(data[i]);
        num_removed++;
      }
      else {
        if (num_removed > 0u) {
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
    for (size_t i = 0u; i < size; i++) {
      if (data[i] == a) {
        data[i] = b;
      }
    }
  }

  void insert_internal(T t) noexcept {
    try_reserve_next(size + 1u);

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
  void insert_uninit(const size_t num = 1u) noexcept {
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
    if (total_required <= capacity) return;

    const size_t prev = capacity;

    //Min capacity should be 8
    if (total_required < 8u) {
      capacity = 8u;
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

    size = 0u;
  }

  void pop() noexcept {
    ASSERT(size > 0u);
    size--;
    (data + size)->~T();
  }

  T take() noexcept {
    T t = std::move(data[size - 1u]);
    pop();
    return t;
  }

  void pop_n(size_t num) noexcept {
    ASSERT(num <= size);
    const auto* old_end = data + size;

    if (num > size) {
      size = 0u;
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
    for(usize i = 0u; i < N; ++i) {
      *start = std::move(*arr);
      start += 1u;
      arr += 1u;
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

    for (usize i = 0u; i < N; ++i) {
      *start = *arr;
      start += 1u;
      arr += 1u;
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


template<ByteOrder Ord>
struct Serializer<Array<u8>, Ord> {
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

  for (size_t i = 0u; i < from.size; i++) {
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
    for (size_t i_to = 0u; i_to < initial_size; i_to++) {
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
  auto* i_back = arr.data + arr.size - 1u;

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

      EL() : _unused() {}
      ~EL() {}
    };

    constexpr static size_t BLOCK_SIZE = 64;

    size_t filled = 0u;
    BLOCK* next = nullptr;

    EL data[BLOCK_SIZE];
  };

  struct Iter {
    size_t index = 0u;
    BLOCK* block = nullptr;
    
    constexpr void next() {
      if(block == nullptr) return;

      index++;
      if (index >= block->filled) {
        index = 0u;
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
    size_t index = 0u;
    const BLOCK* block = nullptr;

    constexpr void next() {
      if(block == nullptr) return;

      index++;
      if (index >= block->filled) {
        index = 0u;
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
    return { 0u, first };
  }

  Iter mut_end() {
    return {};
  }

  ConstIter begin() const {
    return { 0u, first };
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
      for(usize i = 0u; i < b->filled; ++i) {
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
      first->filled = 0u;
    }
    else if (last->filled == BLOCK::BLOCK_SIZE) {
      last->next = allocate_default<BLOCK>();
      last = last->next;

      last->next = nullptr;
      last->filled = 0u;
    }

    T* el = &last->data[last->filled].el;
    new (el) T(std::forward<U>(u)...);
    last->filled++;

    return el;
  }
};

template<typename T>
struct Queue {
  T* holder = nullptr;
  usize start = 0;
  usize size = 0;
  usize capacity = 0;

  //No copy!
  Queue(const Queue&) = delete;

  Queue(Queue&& q) noexcept : holder(q.holder), start(q.start), size(q.size), capacity(q.capacity)
  {
    q.holder = nullptr;
    q.start = 0u;
    q.size = 0u;
    q.capacity = 0u;
  }

  Queue() noexcept = default;

  Queue& operator=(Queue&& q) noexcept {
    if(&q == this) return *this;
    free();

    holder = std::exchange(q.holder, nullptr);
    start = std::exchange(q.start, 0u);
    size = std::exchange(q.size, 0u);
    capacity = std::exchange(q.capacity, 0u);

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
    start = 0u;
    size = 0u;
    capacity = 0u;
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

    if (start == 0u) {
      start = capacity - 1u;
    }
    else {
      start--;
    }
    size++;

    new(holder + start) T(std::move(t));
  }

  T pop_front() {
    ASSERT(size > 0u);

    T val = std::move(holder[start]);
    start++;
    size--;

    if (start >= capacity) {
      start -= capacity;
    }

    return val;
  }

  T& front() {
    ASSERT(size > 0u);
    return holder[start];
  }

  void push_back(T u) {
    if (size == capacity) {
      extend();
    }

    new(holder + _ptr_index(size)) T(std::move(u));
    size++;
  }

  T pop_back() {
    ASSERT(size > 0u);

    size--;
    T val = std::move(holder[_ptr_index(size)]);

    return val;
  }

  T& back() {
    ASSERT(size > 0u);
    return holder[_ptr_index(size)];
  }

  void extend() {
    if (capacity == 0u) {
      holder = allocate_default<T>(8u);
      capacity = 8u;
      return;
    }

    usize new_cap = capacity << 1u;

    T* new_holder = allocate_default<T>(new_cap);

    usize i = 0u;
    usize end_i = size + 1u;
    for (; i < end_i; i++) {
      new_holder[i] = std::move(holder[_ptr_index(i)]);
    }

    free_no_destruct<T>(holder);
    holder = new_holder;
    capacity = new_cap;

    start = 0u;
  }

  void shrink() {
    if (size == 0u) {
      free();
      return;
    }

    usize new_cap = size;

    T* new_holder = allocate_default<T>(new_cap);

    usize i = 0u;
    usize end_i = size + 1u;
    for (; i < end_i; i++) {
      new_holder[i] = std::move(holder[_ptr_index(i)]);
    }

    free_no_destruct(holder);
    holder = new_holder;
    capacity = new_cap;

    start = 0u;
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

    start = 0u;
    size = 0u;
  }
};

template<typename T>
struct AtomicQueue {
  Mutex mutex;

  T* holder = nullptr;
  usize start = 0;
  usize size = 0;
  usize capacity = 0;

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
    start = 0u;
    size = 0u;
    capacity = 0u;
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

    if (size == 0u) {
      mutex.release();
      return false;
    }

    size -= 1u;
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
    if (capacity == 0u) {
      holder = allocate_default<T>(8u);
      capacity = 8u;
      return;
    }

    usize new_cap = capacity << 1u;

    T* new_holder = allocate_default<T>(new_cap);

    usize i = 0u;
    usize end_i = size + 1u;
    for (; i < end_i; i++) {
      new_holder[i] = std::move(holder[_ptr_index(i)]);
    }

    free_no_destruct<T>(holder);
    holder = new_holder;
    capacity = new_cap;

    start = 0u;
  }
};

template<typename T>
struct OwnedArr {
  T* data = nullptr;
  usize size = 0u;

  constexpr OwnedArr() noexcept = default;
  constexpr OwnedArr(T* t, usize s) noexcept : data(t), size(s) {}
  constexpr OwnedArr(OwnedArr&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0u))
  {}

  OwnedArr(const OwnedArr& arr) = delete;
  OwnedArr& operator=(const OwnedArr& arr) = delete;

  void free() {
    free_destruct_n<T>(data, size);
    data = nullptr;
    size = 0u;
  }

  ~OwnedArr() noexcept {
    free();
  }

  constexpr OwnedArr& operator=(OwnedArr&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0u);

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
  usize size = 0u;

  constexpr OwnedArr() noexcept = default;
  constexpr OwnedArr(const T* t, usize s) noexcept : data(t), size(s) {}
  constexpr OwnedArr(OwnedArr<const T>&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0u))
  {}

  constexpr OwnedArr(OwnedArr<T>&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0u))
  {}

  OwnedArr(const OwnedArr& arr) = delete;
  OwnedArr& operator=(const OwnedArr& arr) = delete;

  void free() {
    free_destruct_n<const T>(data, size);
    data = nullptr;
    size = 0u;
  }

  ~OwnedArr() noexcept {
    free();
  }

  constexpr OwnedArr<const T>& operator=(OwnedArr<const T>&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0u);

    return *this;
  }

  constexpr OwnedArr<const T>& operator=(OwnedArr<T>&& arr) noexcept {
    ASSERT(arr.data != data);
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0u);

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

  for (usize i = 0u; i < n; ++i) {
    arr[i] = source[i];
  }
  return OwnedArr<T>(arr, n);
}

template<typename T, usize N>
OwnedArr<T> copy_arr(const T(&source)[N]) {
  T* arr = allocate_default<T>(N);

  for (usize i = 0u; i < N; ++i) {
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
  usize s = std::exchange(arr.size, 0u);
  arr.capacity = 0u;

  return OwnedArr<T>(d, s);
}

template<typename T>
OwnedArr<const T> bake_const_arr(Array<T>&& arr) {
  arr.shrink();

  T* d = std::exchange(arr.data, nullptr);
  usize s = std::exchange(arr.size, 0u);
  arr.capacity = 0u;

  return OwnedArr<const T>(d, s);
}

template<typename T, typename U>
OwnedArr<T> cast_arr(OwnedArr<U>&& arr) {
  static_assert(std::is_trivial_v<T> && std::is_trivial_v<U>);
  static_assert(sizeof(T) % sizeof(U) == 0llu);
  return { reinterpret_cast<T*>(std::exchange(arr.data, nullptr)), 
           std::exchange(arr.size, 0u) / sizeof(U) };
}


template<typename T>
struct ArrayMax {
  T* data = nullptr;
  usize size = 0;
  usize capacity = 0;

  constexpr ArrayMax() = default;
  constexpr ArrayMax(T* data_, usize size_, usize capacity_) noexcept 
    : data(data_), size(size_), capacity(capacity_)
  {}
  constexpr ArrayMax(ArrayMax&& arr) noexcept
    : data(std::exchange(arr.data, nullptr)),
    size(std::exchange(arr.size, 0u)),
    capacity(std::exchange(arr.capacity, 0u))
  {}

  ArrayMax(const ArrayMax& arr) = delete;
  ArrayMax& operator=(const ArrayMax& arr) = delete;

  void free() {
    free_destruct_n<T>(data, size);
    data = nullptr;
    size = 0u;
    capacity = 0u;
  }

  ~ArrayMax() noexcept {
    free();
  }

  constexpr ArrayMax& operator=(ArrayMax&& arr) noexcept {
    if(&arr == this) return *this;
    free();

    data = std::exchange(arr.data, nullptr);
    size = std::exchange(arr.size, 0u);
    capacity = std::exchange(arr.capacity, 0u);

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
  constexpr void insert_uninit(const size_t num = 1u) {
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

    for (size_t i = index; i < size - 1u; i++) {
      data[i] = std::move(data[i + 1u]);
    }
    size--;
    return t;
  }

  constexpr void pop() {
    ASSERT(size > 0u);
    size--;
    (data + size)->~T();
  }

  constexpr void pop_n(size_t num) {
    ASSERT(num <= size);
    const T* old_end = data + size;

    if (num > size) {
      size = 0u;
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
    T t = std::move(data[size - 1u]);
    pop();
    return t;
  }
  
  constexpr void clear() noexcept {
    auto i = mut_begin();
    auto end = mut_end();

    for (; i < end; i++) {
      i->~T();
    }

    size = 0u;
  }

  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }
  constexpr T* mut_begin() { return data; }
  constexpr T* mut_end() { return data + size; }

  constexpr T* back() const {
    ASSERT(size > 0u);
    return data + (size - 1u);
  }

  template<typename L>
  constexpr void remove_if(L&& lambda) {
    size_t num_removed = 0u;

    for (size_t i = 0u; i < size; i++) {
      if (lambda(data[i])) {
        [[maybe_unused]]T removed = std::move(data[i]);
        num_removed++;
      }
      else {
        if (num_removed > 0u) {
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
    0u,
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

}
#endif
