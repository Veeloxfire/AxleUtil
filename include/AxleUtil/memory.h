#ifndef AXLEUTIL_MEMORY_H_
#define AXLEUTIL_MEMORY_H_

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/math.h>
#include <memory>

namespace Axle {
#ifdef AXLE_COUNT_ALLOC
template<typename T>
void free_heap_check(T* ptr, size_t num_bytes) {
  const uint8_t* ptr_end = (const uint8_t*)ptr + num_bytes;

  for (size_t i = 0; i < 4; i++) {
    ASSERT(ptr_end[i] == 0xFD);
  }
}

struct ALLOC_COUNTER {
  struct Allocation {
    bool static_lifetime = false;
    const char* type_name = nullptr;
    const void* mem = nullptr;
    size_t element_size = 0;
    size_t count = 0;
  };

  bool main_program_runtime = false;

  Allocation* allocs = nullptr;
  size_t num_allocs = 0;
  size_t capacity = 0;

  size_t num_static_allocs = 0;
  size_t current_allocated_size = 0;

  size_t max_allocated_blocks = 0;
  size_t max_allocated_size   = 0;

  size_t update_calls = 0;
  size_t null_remove_calls = 0;
  size_t valid_remove_calls = 0;
  size_t insert_calls = 0;

  // Active by default
  inline static bool GLOBALLY_ACTIVE = true;
  inline static ALLOC_COUNTER& allocated() {
    ASSERT(GLOBALLY_ACTIVE);
    struct global_counter { 
      ALLOC_COUNTER counter;
      ~global_counter() {
        GLOBALLY_ACTIVE = false;
      }
    };
    static global_counter allocated_s ={};

    return allocated_s.counter;
  }

  inline ~ALLOC_COUNTER() {
    std::free(allocs);
  }

  inline void reset() {
    main_program_runtime = false;

    if(allocs != nullptr) std::free(allocs);
    num_allocs = 0;
    capacity = 0;

    current_allocated_size = 0;
    num_static_allocs = 0;

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

    allocs[num_allocs].static_lifetime = !main_program_runtime;
    allocs[num_allocs].type_name = typeid(T).name();
    allocs[num_allocs].mem  = (const void*)t;
    allocs[num_allocs].element_size = sizeof(T);
    allocs[num_allocs].count = num;

    if(!main_program_runtime) {
      num_static_allocs++;
    }
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
    ASSERT(to != nullptr && num > 0);
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
        ASSERT(num > 0);
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
  inline void remove_single(Allocation* i) {
    num_allocs--;
    if(i->static_lifetime) {
      ASSERT(num_static_allocs > 0);
      num_static_allocs--;
    }
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
    
    ASSERT(num_allocs > 0);

    const void* t_v = (void*)t;

    auto i = allocs;
    const auto end = allocs + num_allocs;

    for (; i < end; i++) {
      if (i->mem == t_v) {
        //free_heap_check(t, i->count * i->element_size);

        current_allocated_size -= (i->count * i->element_size);
        remove_single(i);
        return;
      }
    }

    INVALID_CODE_PATH("Freed something that wasnt allocated");
  }
};
#endif

template<typename T>
T* allocate_default(const size_t num) {
  if (num == 0) return nullptr;

  T* t = (T*)std::malloc(sizeof(T) * num);

  ASSERT(t != nullptr);

#ifdef AXLE_COUNT_ALLOC
  ALLOC_COUNTER::allocated().insert(t, num);
#endif

  default_init<T>(t, num);
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

#ifdef AXLE_COUNT_ALLOC
  ALLOC_COUNTER::allocated().insert(t, 1);
#endif

  new(t) T(std::forward<U>(u)...);
  return t;
}

template<typename T>
T* reallocate_default(Self<T>* ptr, const size_t old_size, const size_t new_size) {
  ASSERT((ptr != nullptr && old_size > 0) || (ptr == nullptr && old_size == 0));
  ASSERT(new_size != 0);
  T* val = (T*)std::realloc((void*)ptr, sizeof(T) * new_size);
  ASSERT(val != nullptr);

  if (old_size < new_size) {
    default_init<T>(val + old_size, new_size - old_size);
  }

#ifdef AXLE_COUNT_ALLOC
  if(ALLOC_COUNTER::GLOBALLY_ACTIVE) {
    ALLOC_COUNTER::allocated().update(ptr, val, new_size);
  }
#endif

  return val;
}

template<typename T>
void free_destruct_single(Self<T>* ptr) {
#ifdef AXLE_COUNT_ALLOC
  if(ALLOC_COUNTER::GLOBALLY_ACTIVE) {
    ALLOC_COUNTER::allocated().remove(ptr);
  }
#endif

  if (ptr == nullptr) return;

  ptr->~T();
  std::free((void*)ptr);
}

template<typename T>
void free_destruct_n(Self<T>* ptr, size_t num) {
#ifdef AXLE_COUNT_ALLOC
  if(ALLOC_COUNTER::GLOBALLY_ACTIVE) {
    ALLOC_COUNTER::allocated().remove(ptr);
  }
#endif

  if (ptr == nullptr) return;

  for (size_t i = 0; i < num; i++) {
    ptr[i].~T();
  }

  std::free((void*)ptr);
}

template<typename T>
void free_no_destruct(Self<T>* ptr) {
#ifdef AXLE_COUNT_ALLOC
  if(ALLOC_COUNTER::GLOBALLY_ACTIVE) {
    ALLOC_COUNTER::allocated().remove(ptr);
  }
#endif

  if (ptr == nullptr) return;

  std::free((void*)ptr);
}

//TODO: Anything allocated via this memory will not be destroyed
struct MemoryPool {
  u8* mem = nullptr;
  usize total = 0;
  usize top = 0;

  u8* push_alloc_bytes(usize size, usize align);

  template<typename T>
  T* push() {
    u8* ast = push_alloc_bytes(sizeof(T), alignof(T));
    return new (ast) T();
  }

  template<typename T>
  T* push_n(usize n) {
    u8* ast = push_alloc_bytes(sizeof(T) * n, alignof(T));

    for (usize i = 0; i < n; i++) {
      u8* d = ast + (i * sizeof(T));
      new (d) T();
    }
    //Does this work? Is this required?
    return std::launder<T>(reinterpret_cast<T*>(ast));
  }
};

template<usize BLOCK_SIZE> 
struct GrowingMemoryPool {
  using DeleterS = void(*)(void*);
  struct Destructlist {
    Destructlist* prev = nullptr;
    void* data = nullptr;
    DeleterS deleter = nullptr;
  };
  
  using DeleterN = void(*)(void*, usize);
  struct DestructlistN {
    DestructlistN* prev = nullptr;
    void* data = nullptr;
    usize n = 0;
    DeleterN deleter = nullptr;
  };

  struct Block {
    Block* prev = nullptr;
    u8 mem[BLOCK_SIZE] = {};
  };

  template<typename T>
  static void delete_big_alloc(void* data) {
    T* t = std::launder(static_cast<T*>(data));
    free_destruct_single<T>(t);
  }

  template<typename T>
  static void delete_big_alloc_arr(void* data, usize n) {
    T* t = std::launder(static_cast<T*>(data));
    free_destruct_n<T>(t, n);
  }

  static_assert(BLOCK_SIZE >= sizeof(Destructlist));
  static_assert(BLOCK_SIZE >= sizeof(DestructlistN));

  constexpr static bool is_big_alloc(usize N) {
    return N > BLOCK_SIZE;
  }

  void new_block() {
    Block* old = curr;
    curr = new Block();
    curr->prev = old;

    curr_top = 0;
  }

  void* alloc_small_internal(usize size, usize align) {
    ASSERT(!is_big_alloc(size));

    ASSERT(curr_top <= BLOCK_SIZE);
    if(curr == nullptr) {
      new_block();
    }
    
    void* top_p = curr->mem + curr_top;
    usize remaining = BLOCK_SIZE - curr_top;

    if(std::align(align, size, top_p, remaining) == nullptr) {
      //Failed - need a new block
      new_block();

      top_p = curr->mem + curr_top;
      remaining = BLOCK_SIZE - curr_top;

      if(std::align(align, size, top_p, remaining) == nullptr) {
        INVALID_CODE_PATH("Could not allocate object in empty block");
      }
    }

    ASSERT(remaining <= (BLOCK_SIZE - curr_top));
    curr_top = (BLOCK_SIZE - remaining) + size;
    ASSERT(curr_top <= BLOCK_SIZE);

    return top_p;
  }

  void* alloc_raw_no_delete(usize size, usize align) {
    if(size == 0) {
      return nullptr;
    }
    else if(is_big_alloc(size)) {
      ASSERT(align <= 8);
      return Axle::allocate_default<u8>(size);
    }
    else {
      return alloc_small_internal(size, align);
    }
  }

  Destructlist* alloc_destruct_element() {
    void* dl_space = alloc_small_internal(sizeof(Destructlist), alignof(Destructlist));
    Destructlist* dl = new(dl_space) Destructlist();
    dl->prev = dl_top;
    dl->data = nullptr;
    dl->deleter = nullptr;
    dl_top = dl;
    return dl;
  }

  DestructlistN* alloc_destruct_element_N() {
    void* dl_space = alloc_small_internal(sizeof(DestructlistN), alignof(DestructlistN));
    DestructlistN* dl = new(dl_space) DestructlistN();
    dl->prev = dln_top;
    dl->data = nullptr;
    dl->deleter = nullptr;
    dln_top = dl;
    return dl;
  }

  void* alloc_raw(usize size, usize align) {
    if(size == 0) {
      return nullptr;
    }
    else if(is_big_alloc(size)) {
      ASSERT(align <= 8);
      void* data = Axle::allocate_default<u8>(size);

      DestructlistN* dl = alloc_destruct_element_N();
      dl->data = data;
      dl->n = size;
      dl->deleter = &delete_big_alloc_arr<u8>;

      return data;
    }
    else {
      return alloc_small_internal(size, align);
    }
  }

  template<typename T>
  T* allocate() {
    if constexpr(std::is_trivially_destructible_v<T>) {
      if constexpr(is_big_alloc(sizeof(T))) {
        Destructlist* dl = alloc_destruct_element();
        
        T* t = Axle::allocate_default<T>();
        dl->deleter = &delete_big_alloc<T>;
        dl->data = t;
        return t;
      }
      else {
        // Don't need a deleter here
        void* t_space = alloc_small_internal(sizeof(T), alignof(T));
        T* t = new (t_space) T();
        return t;
      }
    }
    else {
      Destructlist* dl = alloc_destruct_element();

      if constexpr(is_big_alloc(sizeof(T))) {
        T* t = Axle::allocate_default<T>();
        dl->deleter = &delete_big_alloc<T>;
        dl->data = t;
        return t;
      }
      else {
        void* t_space = alloc_small_internal(sizeof(T), alignof(T));
        dl->deleter = &destruct_single_void<T>;

        T* t = new (t_space) T();
        dl->data = t;
        return t;
      }
    }
  }

  template<typename T>
  T* allocate_n(usize n) {
    if(n == 0) return nullptr;

    if constexpr (std::is_trivially_destructible_v<T>) {
      if (is_big_alloc(sizeof(T) * n)) {
        DestructlistN* dl = alloc_destruct_element_N();
        
        T* t = Axle::allocate_default<T>(n);
        dl->deleter = &delete_big_alloc_arr<T>;
        dl->n = n;
        dl->data = t;
        return t;
      }
      else {
        void* t_space = alloc_small_internal(sizeof(T) * n, alignof(T));

        T* t = new(t_space)T[n];

        return t;
      }
    }
    else {
      DestructlistN* dl = alloc_destruct_element_N();

      if (is_big_alloc(sizeof(T) * n)) {
        T* t = Axle::allocate_default<T>(n);
        dl->deleter = &delete_big_alloc_arr<T>;
        dl->n = n;
        dl->data = t;
        return t;
      }
      else {
        void* t_space = alloc_small_internal(sizeof(T) * n, alignof(T));
        dl->deleter = &destruct_arr_void<T>;
        dl->n = n;

        T* t = new(t_space)T[n];

        dl->data = t;
        return t;
      }
    }
  }

  void free() {
    while(dl_top != nullptr) {
      ASSERT(dl_top->deleter != nullptr);
      ASSERT(dl_top->data != nullptr);
      dl_top->deleter(dl_top->data);
      Destructlist* p = dl_top->prev;
      destruct_single<Destructlist>(dl_top);
      dl_top = p;
    }
    
    while(dln_top != nullptr) {
      ASSERT(dln_top->deleter != nullptr);
      ASSERT(dln_top->data != nullptr);
      ASSERT(dln_top->n > 0);
      dln_top->deleter(dln_top->data, dln_top->n);
      DestructlistN* p = dln_top->prev;
      destruct_single<DestructlistN>(dln_top);
      dln_top = p;
    }

    while (curr != nullptr) {
      Block* save = curr->prev;

      delete curr;

      curr = save;
    }

    curr_top = 0;
    ASSERT(curr == nullptr);
    ASSERT(dl_top == nullptr);
    ASSERT(dln_top == nullptr);
  }

  usize curr_top = 0;
  Block* curr = nullptr;
  Destructlist* dl_top = nullptr;
  DestructlistN* dln_top = nullptr;

  constexpr GrowingMemoryPool() = default;
  GrowingMemoryPool(const GrowingMemoryPool&) = delete;

  constexpr GrowingMemoryPool(GrowingMemoryPool&& gp) noexcept
    : curr_top(std::exchange(gp.curr_top, static_cast<usize>(0))),
      curr(std::exchange(gp.curr, nullptr)),
      dl_top(std::exchange(gp.dl_top, nullptr)),
      dln_top(std::exchange(gp.dln_top, nullptr))
  {}

  GrowingMemoryPool& operator=(GrowingMemoryPool&& gp)
  {
    free();// clear current memory
    curr_top = std::exchange(gp.curr_top, static_cast<usize>(0));
    curr = std::exchange(gp.curr, nullptr);
    dl_top = std::exchange(gp.dl_top, nullptr);
    dln_top = std::exchange(gp.dln_top, nullptr);

    return *this;
  }

  ~GrowingMemoryPool() {
    free();
  }
};

template<typename T>
struct FreelistBlockAllocator {
  struct Element {
    alignas(T) u8 el[sizeof(T)];
    Element* next;
  };

  struct BLOCK {
    constexpr static size_t BLOCK_SIZE = 32u;

    //size_t filled = 0;
    BLOCK* prev = nullptr;
    Element data[BLOCK_SIZE] = {};
    
    constexpr BLOCK() = default;
    constexpr ~BLOCK() = default;
    
    constexpr BLOCK(const BLOCK&) = delete;
    constexpr BLOCK& operator=(const BLOCK&) = delete;
  };

  BLOCK* top = nullptr;
  Element* alloc_list = nullptr;

  FreelistBlockAllocator() = default;
  FreelistBlockAllocator(const FreelistBlockAllocator&) = delete;
  FreelistBlockAllocator(FreelistBlockAllocator&&) = delete;
  FreelistBlockAllocator& operator=(const FreelistBlockAllocator&) = delete;
  FreelistBlockAllocator& operator=(FreelistBlockAllocator&&) = delete;

  ~FreelistBlockAllocator() noexcept 
  {
    free_all();
  }

  void new_block() {
    BLOCK* const new_b = allocate_default<BLOCK>();

    new_b->prev = top;
    top = new_b;

    for (usize i = 0; i < BLOCK::BLOCK_SIZE - 1; i++) {
      top->data[i].next = &top->data[i + 1u];
    }

    top->data[BLOCK::BLOCK_SIZE - 1u].next = alloc_list;

    alloc_list = top->data;
  }

  T* allocate() {
    if (alloc_list == nullptr) {
      new_block();
    }

    Element* e = alloc_list;
    alloc_list = e->next;

    e->next = nullptr;
    return new(e->el) T();
  }

  bool _debug_valid_free_ptr(const T* const t) const {
    const Element* e = alloc_list;
    while (e != nullptr) {
      if (e->el == reinterpret_cast<const u8*>(t)) return false;
      e = e->next;
    }

    const BLOCK* b = top;
    while (b != nullptr) {
      const u8* block_base = reinterpret_cast<const u8*>(b->data);
      const u8* block_top = reinterpret_cast<const u8*>(b->data + BLOCK::BLOCK_SIZE);

      const u8* t_base = reinterpret_cast<const u8*>(t);
      const u8* t_top = reinterpret_cast<const u8*>(t + 1u);
      if (block_base <= t_base && t_top <= block_top) return true;

      b = b->prev;
    }

    return false;
  }

  bool _debug_all_are_free() const {
    usize actual = 0u;
    const Element* e = alloc_list;
    while (e != nullptr) {
      actual += 1u;
      e = e->next;
    }

    usize expected = 0u;
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

    for (u8& b: new_e->el) {
      b = 0;
    }

    alloc_list = new_e;
  }

  void free_all() {
    while (top != nullptr) {
      for (Element& e : top->data) {
        if (e.next == nullptr) {
          T* t = std::launder(reinterpret_cast<T*>(e.el));
          destruct_single<T>(t);
        }
      }

      BLOCK* old = top;
      top = top->prev;
      free_destruct_single<BLOCK>(old);
    }

    top = nullptr;
    alloc_list = nullptr;
  }
};

template<typename T>
struct DenseBlockAllocator {
  struct Block {
    T* data = nullptr;
    usize size = 0;
    usize capacity = 0;

    constexpr usize remaining_size() const {
      ASSERT(size <= capacity);
      return capacity - size;
    }
  };

  // sorted array of blocks
  // first_empty points to the first block which isnt full
  // After that you can binary search up to the size
  // to find a block with the correct size
  Block* blocks = nullptr;
  usize size = 0;
  usize capacity = 0;

  usize next_alloc_size = 0;
  usize first_empty = 0;
  
  DenseBlockAllocator() = default;
  ~DenseBlockAllocator() {
    for(usize i = 0; i < size; ++i) {
      const Block* const b = blocks + i;
      free_destruct_n<T>(b->data, b->size);
    }

    free_destruct_n<Block>(blocks, capacity);
  }
  
  DenseBlockAllocator(const DenseBlockAllocator&) = delete;
  DenseBlockAllocator(DenseBlockAllocator&&) = delete;
  DenseBlockAllocator& operator=(const DenseBlockAllocator&) = delete;
  DenseBlockAllocator& operator=(DenseBlockAllocator&&) = delete;

  void grow() {
    ASSERT(size == capacity);
    if(capacity == 0) {
      capacity = 8;
    }
    else {
      capacity *= 2;
    }

    blocks = reallocate_default<Block>(blocks, size, capacity);
  }

  [[nodiscard]] Axle::ViewArr<T> internal_alloc_from_empty(const usize n) {
    usize blk_index;

    if(first_empty < size && (blocks + size - 1)->remaining_size() >= n) {
      // Should be able to allocate in the current blocks
      if(n == 1) {
        // just pick the first one
        blk_index = first_empty;
      }
      else {
        // search for one large enough
        usize l = 0;
        usize h = size - first_empty;
        while(l != h) {
          usize mid = (l + (h - 1)) / 2;

          Block* mid_b = blocks + mid + first_empty;

          if(n <= mid_b->remaining_size()) {
            h = mid;
          }
          else {
            l = mid + 1;
          }
        }

        ASSERT(l < (size - first_empty));

        blk_index = l + first_empty;
      }
    }
    else {
      // no chance, needs another allocation
      blk_index = size;
      if(size == capacity) {
        grow();
        ASSERT(blk_index < capacity);
        ASSERT(blk_index == size);
      }

      Block* blk = blocks + blk_index;
      size += 1;
      ASSERT(blk->data == nullptr);

      if(next_alloc_size == 0) {
        next_alloc_size = 32;// fairly arbitrary
      }

      usize na_size = next_alloc_size;
      next_alloc_size *= 2;

      if(n > na_size) {
        if(n < (1u << 15u)) {
          na_size = Axle::ceil_to_pow_2(n);
        }
        else {
          na_size = n;
        }
      }

      blk->data = Axle::allocate_default<T>(na_size);
      blk->size = 0;
      blk->capacity = na_size;
    }

    ASSERT(blk_index < size);
    ASSERT(first_empty <= blk_index);
    Block* blk = blocks + blk_index;
    ASSERT(blk->remaining_size() >= n);

    const Axle::ViewArr<T> vals = { blk->data + blk->size, n };
    blk->size += n;
    ASSERT(blk->capacity >= blk->size);

    const usize curr_remaining = blk->remaining_size();

    Block* first_empty_block = blocks + first_empty;
    ASSERT(first_empty_block <= blk);

    while(blk != first_empty_block) {
      Block* next_swap = blk - 1;

      // Swap backwards while its too large
      if(next_swap->remaining_size() > curr_remaining) {
        std::swap(*blk, *next_swap);
        blk = next_swap;
      }
      else {
        break;
      }
    }

    if(blk->remaining_size() == 0) {
      first_empty += 1;
    }

    return vals;
  }

  [[nodiscard]] T* allocate() {
    Axle::ViewArr<T> ts = internal_alloc_from_empty(1);

    ASSERT(ts.size == 1);
    T* t = ts.data;
    reset_type<T>(t);
    return t;
  }

  [[nodiscard]] Axle::ViewArr<T> allocate_n(usize n) {
    if(n == 0) { return {nullptr, 0}; }

    Axle::ViewArr<T> ts = internal_alloc_from_empty(n);

    ASSERT(ts.size == n);

    FOR_MUT(ts, t) {
      reset_type<T>(t);
    }
    return ts;
  }
};

struct ArenaAllocator {
  static_assert(sizeof(void*) == sizeof(uint64_t), "Must be 8 bytes");

  struct Block {
    constexpr static size_t BLOCK_SIZE = 1024;
    uint64_t data[BLOCK_SIZE] = {};

    Block* next = nullptr;

    Block() = default;
    ~Block();
  };

  struct FreeList {
    uint64_t qwords_available = 0;
    FreeList* next = nullptr;
  };

  Block* base = nullptr;
  FreeList* free_list = nullptr;

  ArenaAllocator() = default;
  ~ArenaAllocator();

  bool _debug_freelist_loops() const;
  bool _debug_valid_pointer(void* ptr) const;
  bool _debug_is_allocated_data(uint64_t* ptr, usize len) const;

  void new_block();
  void add_to_free_list(FreeList* fl);

  uint8_t* alloc_no_construct(size_t bytes);
  void free_no_destruct(void* val);

  template<typename T>
  inline T* alloc_no_construct() {
    return (T*)alloc_no_construct(sizeof(T));
  }
};
}

#endif
