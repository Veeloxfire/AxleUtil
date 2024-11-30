#ifndef AXLEUTIL_HASHMAP_H_
#define AXLEUTIL_HASHMAP_H_

#include <AxleUtil/memory.h>

namespace Axle::Hash {
enum class DefaultChainType {
  Internal, External,
};

template<typename T>
concept ValidInternalTrait = requires {
  { T::DEFAULT_CHAIN_TYPE } -> Axle::IS_SAME_TYPE<const DefaultChainType&>;
  typename T::storage_t;
  typename T::value_t;
  typename T::param_t;
  { T::EMPTY } -> Axle::IS_SAME_TYPE<const typename T::storage_t&>;
  { T::TOMBSTONE } -> Axle::IS_SAME_TYPE<const typename T::storage_t&>;
  requires requires (const T::storage_t cs, const T::param_t k) {
    { T::hash(cs) } -> Axle::IS_SAME_TYPE<u64>;
    { T::value(cs) } -> Axle::IS_SAME_TYPE<typename T::value_t>;
    { T::eq(cs, cs) } -> Axle::IS_SAME_TYPE<bool>;
    { T::storage_t(k) } -> Axle::IS_SAME_TYPE<typename T::storage_t>; 
  };
};

template<typename T>
struct DefaultHashmapTrait;

template<typename T, ValidInternalTrait Trait = DefaultHashmapTrait<T>>
struct InternalHashSet {
  constexpr static float LOAD_FACTOR = 0.75;

  using storage_t = typename Trait::storage_t;
  using value_t = typename Trait::value_t;
  using param_t = typename Trait::param_t;

  storage_t* data = nullptr;// ptr to data in the array
  usize el_capacity = 0;
  usize used = 0;

  constexpr bool needs_resize(usize extra) const {
    return (el_capacity * LOAD_FACTOR) <= (used + extra);
  }

  ~InternalHashSet();

  bool contains(param_t key) const;
  storage_t& internal_get(const storage_t& s_key) const;
  value_t get(param_t key) const;
  void try_extend(usize num);
  void insert(param_t key);
  void remove(param_t key);
};

template<typename K, typename T, ValidInternalTrait Trait = DefaultHashmapTrait<K>>
struct InternalHashTable {
  constexpr static float LOAD_FACTOR = 0.75;

  using storage_t = typename Trait::storage_t;
  using value_t = typename Trait::value_t;
  using param_t = typename Trait::param_t;

  union val_storage_t {
    char _placeholder = '\0';
    T val;

    void clear() {
      val.~T();
      _placeholder = '\0';
    }
  };

  u8* data = nullptr;// ptr to data in the array
  usize el_capacity = 0;
  usize used = 0;

  constexpr bool needs_resize(size_t extra) const {
    return (el_capacity * LOAD_FACTOR) <= (used + extra);
  }

  constexpr static usize val_arr_offset(usize size) {
    return ceil_to_N<alignof(val_storage_t)>(
        size * sizeof(storage_t));
  }
  
  static inline storage_t* key_arr(u8* raw_data) {
    return reinterpret_cast<storage_t*>(raw_data);
  }

  inline storage_t* key_arr() const {
    return key_arr(data);
  }

  inline val_storage_t* val_arr(u8* raw_data, usize capacity) const {
    return reinterpret_cast<val_storage_t*>(raw_data + 
      val_arr_offset(capacity));
  }

  inline val_storage_t* val_arr() const {
    return val_arr(data, el_capacity);
  }

  ~InternalHashTable();
  bool contains(param_t key) const;
  usize get_soa_index(param_t key) const;
  void try_extend(usize num);
  T* get_val(param_t key) const;
  void insert(param_t key, T&& val);
  T* get_or_create(param_t key);

  struct Iterator {
    InternalHashTable<K, T, Trait>* table;
    usize i;

    value_t key() const {
      if (!is_valid()) return Trait::value(Trait::EMPTY);
      return Trait::value(table->key_arr()[i]);
    }
    T* val() const {
      if (!is_valid()) return nullptr;
      return &table->val_arr()[i].val;
    }

    constexpr bool is_valid() const {
      return table != nullptr;
    }

    void next() {
      if (table == nullptr) return;

      i += 1;

      storage_t* keys = table->key_arr() + i;
      while (i < table->el_capacity
        && (Trait::eq(*keys, Trait::EMPTY)
         || Trait::eq(*keys, Trait::TOMBSTONE))) {
        keys += 1;
        i += 1;
      }

      if (i == table->el_capacity) {
        table = nullptr;
      }
    }
  };  

  Iterator itr() {
    Iterator i{
        this,
        (usize)-1,
    };


    i.next();
    return i;
  }
};

template<typename T, ValidInternalTrait Trait>
InternalHashSet<T, Trait>::~InternalHashSet() {
  free_no_destruct<storage_t>(data);

  data = nullptr;
  el_capacity = 0;
  used = 0;
}

template<typename T, ValidInternalTrait Trait>
bool InternalHashSet<T, Trait>::contains(param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if(el_capacity == 0) return false;

  const usize start_index = Trait::hash(key) % el_capacity;

  usize index = start_index;
  do {
    storage_t& test_key = data[index];
    if (Trait::eq(key, test_key)) {
      return true;
    }
    else if (Trait::eq(Trait::EMPTY, test_key)) {
      return false;
    }

    index++;
    index %= el_capacity;
  } while(index != start_index);

  return false;
}

template<typename T, ValidInternalTrait Trait>
typename InternalHashSet<T, Trait>::storage_t& InternalHashSet<T, Trait>::internal_get(const storage_t& s_key) const {
  bool found_tombstone = false;
  usize tombstone_index = 0;

  const usize start_index = Trait::hash(s_key) % el_capacity;

  usize index = start_index;
  do {
    storage_t& test_key = data[index];

    if (Trait::eq(s_key, test_key)) {
      return data[index];
    }
    else if(Trait::eq(Trait::EMPTY, test_key)) {
      break;
    }
    else if (Trait::eq(Trait::TOMBSTONE, test_key) && !found_tombstone) {
      found_tombstone = true;
      tombstone_index = index;
    }

    index++;
    index %= el_capacity;
  } while (index != start_index);

  if (found_tombstone) {
    return data[tombstone_index];
  }
  else {
    return data[index];
  }
}

template<typename T, ValidInternalTrait Trait>
typename InternalHashSet<T, Trait>::value_t InternalHashSet<T, Trait>::get(param_t key) const {
  return Trait::value(get_internal(Trait::storage_t(key)));
}

template<typename T, ValidInternalTrait Trait>
void InternalHashSet<T, Trait>::try_extend(usize num) {
  if (needs_resize(num)) {
    storage_t* old_data = data;
    const usize old_el_cap = el_capacity;

    do {
      el_capacity <<= 1;
    } while (needs_resize(num));

    data = allocate_default<storage_t>(el_capacity);

    for(usize i = 0; i < el_capacity; ++i) {
      data[i] = Trait::EMPTY;
    }

    for (size_t i = 0; i < old_el_cap; i++) {
      storage_t& st = old_data[i];

      if (!Trait::eq(Trait::EMPTY, st)
          && !Trait::eq(Trait::TOMBSTONE, st)) {
        storage_t& loc = internal_get(st);
        loc = st;
      }
    }

    free_no_destruct<storage_t>(old_data);
  }
}

template<typename T, ValidInternalTrait Trait>
void InternalHashSet<T, Trait>::insert(param_t key) {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));

  storage_t s_key = key;
  
  if (el_capacity == 0) {
    ASSERT(used == 0);
    el_capacity = 8;
    data = allocate_default<storage_t>(el_capacity);
    for(usize i = 0; i < el_capacity; ++i) {
      data[i] = Trait::EMPTY;
    }

    storage_t& loc = internal_get(s_key);
    loc = s_key;
    used += 1;
  }
  else {
    storage_t& loc = internal_get(s_key);

    if (Trait::eq(s_key, loc)) return;//already contained

    ASSERT(Trait::eq(loc, Trait::EMPTY)
        || Trait::eq(loc, Trait::TOMBSTONE));

    loc = s_key;
    used += 1;
    if (needs_resize(0)) {
      try_extend(0);
    }
  }
}

template<typename T, ValidInternalTrait Trait>
void InternalHashSet<T, Trait>::remove(param_t key) {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  ASSERT(used > 0 && el_capacity > 0);

  storage_t& loc = internal_get(storage_t(key));
  loc = Trait::TOMBSTONE;
  used -= 1;
}

template<typename K, typename T, ValidInternalTrait Trait>
InternalHashTable<K, T, Trait>::~InternalHashTable() {
  if(data != nullptr) {
    storage_t* keys = key_arr();
    val_storage_t* vals = val_arr();

    for (size_t i = 0; i < el_capacity; i++) {
      if (!Trait::eq(keys[i], Trait::EMPTY)
          && !Trait::eq(keys[i], Trait::TOMBSTONE)) {
        vals[i].clear(); 
      }
    }

    destruct_arr<storage_t>(keys, el_capacity);
    destruct_arr<val_storage_t>(vals, el_capacity);
  }

  free_no_destruct<u8>(data);

  data = nullptr;
  el_capacity = 0;
  used = 0;
}

template<typename K, typename T, ValidInternalTrait Trait>
bool InternalHashTable<K, T, Trait>::contains(param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if (el_capacity == 0) return false;

  const storage_t* keys = key_arr();

  const usize first_index = Trait::hash(key) % el_capacity;

  usize index = first_index;
  do {
    const storage_t& test_key = keys[index];
    if (Trait::eq(key, test_key)) {
      return true;
    }
    else if (Trait::eq(Trait::EMPTY, test_key)) {
      return false;
    }

    index++;
    index %= el_capacity;
  } while(index != first_index);

  return false;
}

template<typename K, typename T, ValidInternalTrait Trait>
size_t InternalHashTable<K, T, Trait>::get_soa_index(param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  const storage_t* const keys = key_arr();

  bool found_tombstone = false;
  usize tombstone_index = 0;

  const usize first_index = Trait::hash(key) % el_capacity;
  usize index = first_index;

  do {
    const storage_t& test_key = keys[index];
    if (Trait::eq(key, test_key)) {
      return index;
    }
    else if (!found_tombstone &&
        Trait::eq(Trait::TOMBSTONE, test_key)) {
      found_tombstone = true;
      tombstone_index = index;
    }
    else if(Trait::eq(Trait::EMPTY, test_key)) {
      break;
    }

    index++;
    index %= el_capacity;
  } while(first_index != index);

  if (found_tombstone) {
    return tombstone_index;
  }
  else {
    return index;
  }
}

template<typename K, typename T, ValidInternalTrait Trait>
void InternalHashTable<K, T, Trait>::try_extend(size_t num) {
  ASSERT(needs_resize(num));

  uint8_t* old_data = data;
  const size_t old_el_cap = el_capacity;

  if (el_capacity == 0) {
    el_capacity = 8;
    while (needs_resize(num)) {
      el_capacity <<= 1;
    }
  }
  else {
    do {
      el_capacity <<= 1;
    } while (needs_resize(num));
  }

  const size_t required_alloc_bytes = 
    val_arr_offset(el_capacity)
    + el_capacity * sizeof(val_storage_t);

  data = allocate_default<uint8_t>(required_alloc_bytes);
  new (data) storage_t[el_capacity];
  new (data + val_arr_offset(el_capacity)) val_storage_t[el_capacity];

  storage_t* keys = key_arr();

  for(usize i = 0; i < el_capacity; ++i) {
    keys[i] = Trait::EMPTY;
  }

  if (old_data != nullptr) {

    val_storage_t* values = val_arr();

    storage_t* old_keys = key_arr(old_data);
    val_storage_t* old_values = val_arr(old_data, old_el_cap);

    for (size_t i = 0; i < old_el_cap; i++) {
      const storage_t& key = old_keys[i];

      if (!Trait::eq(Trait::EMPTY, key)
          && !Trait::eq(Trait::TOMBSTONE, key)) {
        const usize new_index = get_soa_index(key);

        val_storage_t& v = old_values[i];

        keys[new_index] = key;
        values[new_index].val = std::move(v.val);
        v.clear();

      }
    }

    //Destruct and free
    {
      destruct_arr<storage_t>(old_keys, el_capacity);
      destruct_arr<val_storage_t>(old_values, el_capacity);

      free_no_destruct<u8>(old_data);
    }
  }
}

template<typename K, typename T, ValidInternalTrait Trait>
T* InternalHashTable<K, T, Trait>::get_val(param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if (el_capacity == 0) return nullptr;

  const size_t soa_index = get_soa_index(key);

  storage_t& test_key = key_arr()[soa_index];
  if (Trait::eq(key, test_key)) return &val_arr()[soa_index].val;
  else {
    ASSERT(Trait::eq(Trait::EMPTY, test_key)
        || Trait::eq(Trait::TOMBSTONE, test_key));
    return nullptr;
  }
}

template<typename K, typename T, ValidInternalTrait Trait>
void InternalHashTable<K, T, Trait>::insert(param_t key, T&& val) {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));

  if (el_capacity == 0) {
    try_extend(1);

    const usize soa_index = get_soa_index(key);

    key_arr()[soa_index] = key;
    val_arr()[soa_index].val = std::move(val);
    used += 1;
  }
  else {
    const usize soa_index = get_soa_index(key);
    
    storage_t& key_loc = key_arr()[soa_index];
    if (Trait::eq(key, key_loc)) {
      val_arr()[soa_index].val = std::move(val);
      return;
    }

    ASSERT(Trait::eq(Trait::EMPTY, key_loc)
        || Trait::eq(Trait::TOMBSTONE, key_loc));

    key_loc = key;
    val_arr()[soa_index].val = std::move(val);
    used += 1;

    if (needs_resize(0)) {
      try_extend(0);
    }
  }
}

template<typename K, typename T, ValidInternalTrait Trait>
T* InternalHashTable<K, T, Trait>::get_or_create(param_t key) {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if (el_capacity == 0) {
    try_extend(1);

    const usize soa_index = get_soa_index(key);

    val_storage_t& val = val_arr()[soa_index];

    key_arr()[soa_index] = key;
    val.val = T{};
    used += 1;
    return &val.val;
  }
  else {
    const usize soa_index = get_soa_index(key);

    storage_t& test_key = key_arr()[soa_index];

    if (Trait::eq(key, test_key)) {
      return &val_arr()[soa_index].val;
    }
    
    ASSERT(Trait::eq(Trait::EMPTY, test_key)
        || Trait::eq(Trait::TOMBSTONE, test_key));

    val_storage_t& val = val_arr()[soa_index];

    test_key = key;
    val.val = T{};
    used += 1;

    if (needs_resize(0)) {
      try_extend(0);

      //need to reset the key and val location
      const usize re_index = get_soa_index(key);

      ASSERT(Trait::eq(key, key_arr()[re_index]));
      return &val_arr()[re_index].val;
    }
    else {
      return &val.val;
    }
  }
}
}

#endif
