#ifndef AXLEUTIL_MEMORY_H_
#define AXLEUTIL_MEMORY_H_

#include <AxleUtil/safe_lib.h>
#include <memory>

namespace Axle {
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

#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().insert(t, 1);
#endif

  new(t) T(std::forward<U>(u)...);
  return t;
}

template<typename T>
T* reallocate_default(Self<T>* ptr, const size_t old_size, const size_t new_size) {
  T* val = (T*)std::realloc((void*)ptr, sizeof(T) * new_size);
  ASSERT(val != nullptr);

  if (old_size < new_size) {
    default_init<T>(val + old_size, new_size - old_size);
  }

#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().update(ptr, val, new_size);
#endif

  return val;
}

template<typename T>
void free_destruct_single(Self<T>* ptr) {
#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().remove(ptr);
#endif

  if (ptr == nullptr) return;

  ptr->~T();
  std::free((void*)ptr);
}

template<typename T>
void free_destruct_n(Self<T>* ptr, size_t num) {
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
void free_no_destruct(Self<T>* ptr) {
#ifdef COUNT_ALLOC
  ALLOC_COUNTER::allocated().remove(ptr);
#endif

  if (ptr == nullptr) return;

  std::free((void*)ptr);
}

//TODO: Anything allocated via this memory will not be destroyed
struct MemoryPool {
  u8* mem;
  usize total;
  usize top;

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
    Destructlist* prev;
    void* data;
    DeleterS deleter;
  };
  
  using DeleterN = void(*)(void*, usize);
  struct DestructlistN {
    DestructlistN* prev;
    void* data;
    usize n;
    DeleterN deleter;
  };

  struct Block {
    Block* prev = nullptr;
    u8 mem[BLOCK_SIZE];
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

  usize curr_top = 0;
  Block* curr = nullptr;
  Block* curr_big = nullptr;
  Destructlist* dl_top = nullptr;
  DestructlistN* dln_top = nullptr;

  void new_block() {
    Block* old = curr;
    curr = new Block();
    curr->prev = old;

    curr_top = 0;
  }
 
  void* alloc_internal(usize size, usize align) {
    ASSERT(size <= BLOCK_SIZE);

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

  void* alloc_raw(usize size, usize align) {
    if(size <= BLOCK_SIZE) {
      return alloc_internal(size, align);
    }
    else {
      assert(align <= 8);
      return Axle::allocate_default<u8>(size);
    }
  }

  Destructlist* alloc_destruct_element() {
    void* dl_space = alloc_internal(sizeof(Destructlist), alignof(Destructlist));
    Destructlist* dl = new(dl_space) Destructlist();
    dl->prev = dl_top;
    dl->data = nullptr;
    dl->deleter = nullptr;
    dl_top = dl;
    return dl;
  }

  DestructlistN* alloc_destruct_element_N() {
    void* dl_space = alloc_internal(sizeof(DestructlistN), alignof(DestructlistN));
    DestructlistN* dl = new(dl_space) DestructlistN();
    dl->prev = dln_top;
    dl->data = nullptr;
    dl->deleter = nullptr;
    dln_top = dl;
    return dl;
  }

  template<typename T>
  T* allocate() {
    Destructlist* dl = alloc_destruct_element();

    if constexpr(sizeof(T) <= BLOCK_SIZE) {
      void* t_space = alloc_internal(sizeof(T), alignof(T));
      dl->deleter = &destruct_single_void<T>;

      T* t = new (t_space) T();
      dl->data = t;
      return t;
    }
    else {
      T* t = Axle::allocate_default<T>();
      dl->deleter = &delete_big_alloc<T>;
      dl->data = t;
      return t;
    }
  }

  template<typename T>
  T* allocate_n(usize n) {
    ASSERT(n > 0);
    DestructlistN* dl = alloc_destruct_element_N();

    if (sizeof(T) * n <= BLOCK_SIZE) {
      void* t_space = alloc_internal(sizeof(T) * n, alignof(T));
      dl->deleter = &destruct_arr_void<T>;
      dl->n = n;

      T* t = new(t_space)T[n];

      dl->data = t;
      return t;
    }
    else {
      T* t = Axle::allocate_default<T>(n);
      dl->deleter = &delete_big_alloc_arr<T>;
      dl->n = n;
      dl->data = t;
      return t;
    }
  }

  GrowingMemoryPool() = default;
  GrowingMemoryPool(const GrowingMemoryPool&) = delete;
  GrowingMemoryPool(GrowingMemoryPool&&) = delete;
  ~GrowingMemoryPool() {
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
