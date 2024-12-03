#ifndef AXLEUTIL_HASH_H_
#define AXLEUTIL_HASH_H_

#include <AxleUtil/memory.h>

namespace Axle::Hash {
template<typename T>
concept ValidInternalTrait = requires {
  typename T::value_t;
  typename T::param_t;
  requires OneOf<const typename T::param_t, const typename T::value_t, const typename T::value_t&>;
  
  { T::EMPTY } -> Axle::IS_SAME_TYPE<const typename T::value_t&>;
  { T::TOMBSTONE } -> Axle::IS_SAME_TYPE<const typename T::value_t&>;
  requires requires (const T::param_t k) {
    { T::hash(k) } -> Axle::IS_SAME_TYPE<u64>;
    { T::eq(k, k) } -> Axle::IS_SAME_TYPE<bool>;
  };
};

template<typename T>
struct DefaultHashmapTrait;

template<typename T, ValidInternalTrait Trait = DefaultHashmapTrait<T>>
struct InternalHashSet {
  constexpr static float LOAD_FACTOR = 0.75;

  using value_t = typename Trait::value_t;
  using param_t = typename Trait::param_t;

  value_t* data = nullptr;// ptr to data in the array
  usize el_capacity = 0;
  usize used = 0;

  constexpr bool needs_resize(usize extra) const {
    return (el_capacity * LOAD_FACTOR) <= (used + extra);
  }

  ~InternalHashSet();

  bool contains(const param_t key) const;
  value_t& internal_get(const param_t key) const;
  value_t get(const param_t key) const;
  void try_extend(usize num);
  void insert(const param_t key);
  void remove(const param_t key);
};

template<typename K, typename T, ValidInternalTrait Trait = DefaultHashmapTrait<K>>
struct InternalHashTable {
  constexpr static float LOAD_FACTOR = 0.75;

  using value_t = typename Trait::value_t;
  using param_t = typename Trait::param_t;

  union val_storage_t {
    char _placeholder = '\0';
    T val;

    constexpr void clear() {
      val.~T();
      _placeholder = '\0';
    }

    constexpr ~val_storage_t() {}
  };

  u8* data = nullptr;// ptr to data in the array
  usize el_capacity = 0;
  usize used = 0;

  constexpr bool needs_resize(size_t extra) const {
    return (el_capacity * LOAD_FACTOR) <= (used + extra);
  }

  constexpr static usize val_arr_offset(usize size) {
    return ceil_to_N<alignof(val_storage_t)>(
        size * sizeof(value_t));
  }
  
  static inline value_t* key_arr(u8* raw_data) {
    return reinterpret_cast<value_t*>(raw_data);
  }

  inline value_t* key_arr() const {
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
  bool contains(const param_t key) const;
  usize get_soa_index(const param_t key) const;
  void try_extend(usize num);
  T* get_val(const param_t key) const;
  void insert(const param_t key, T&& val);
  T* get_or_create(const param_t key) requires requires(T t) { {T()}->IS_SAME_TYPE<T>; };

  struct Iterator {
    InternalHashTable<K, T, Trait>* table;
    usize i;

    value_t key() const {
      if (!is_valid()) return Trait::EMPTY;
      return table->key_arr()[i];
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

      value_t* keys = table->key_arr() + i;
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
  free_no_destruct<value_t>(data);

  data = nullptr;
  el_capacity = 0;
  used = 0;
}

template<typename T, ValidInternalTrait Trait>
bool InternalHashSet<T, Trait>::contains(const param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if(el_capacity == 0) return false;

  const usize start_index = Trait::hash(key) % el_capacity;

  usize index = start_index;
  do {
    value_t& test_key = data[index];
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
typename InternalHashSet<T, Trait>::value_t& InternalHashSet<T, Trait>::internal_get(const param_t s_key) const {
  bool found_tombstone = false;
  usize tombstone_index = 0;

  const usize start_index = Trait::hash(s_key) % el_capacity;

  usize index = start_index;
  do {
    value_t& test_key = data[index];

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
typename InternalHashSet<T, Trait>::value_t InternalHashSet<T, Trait>::get(const param_t key) const {
  return get_internal(key);
}

template<typename T, ValidInternalTrait Trait>
void InternalHashSet<T, Trait>::try_extend(usize num) {
  if (needs_resize(num)) {
    value_t* old_data = data;
    const usize old_el_cap = el_capacity;

    do {
      el_capacity <<= 1;
    } while (needs_resize(num));

    data = allocate_default<value_t>(el_capacity);

    for(usize i = 0; i < el_capacity; ++i) {
      data[i] = Trait::EMPTY;
    }

    for (size_t i = 0; i < old_el_cap; i++) {
      value_t& st = old_data[i];

      if (!Trait::eq(Trait::EMPTY, st)
          && !Trait::eq(Trait::TOMBSTONE, st)) {
        value_t& loc = internal_get(st);
        loc = st;
      }
    }

    free_no_destruct<value_t>(old_data);
  }
}

template<typename T, ValidInternalTrait Trait>
void InternalHashSet<T, Trait>::insert(const param_t key) {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
 
  if (el_capacity == 0) {
    ASSERT(used == 0);
    el_capacity = 8;
    data = allocate_default<value_t>(el_capacity);
    for(usize i = 0; i < el_capacity; ++i) {
      data[i] = Trait::EMPTY;
    }

    value_t& loc = internal_get(key);
    loc = value_t(key);
    used += 1;
  }
  else {
    value_t& loc = internal_get(key);

    if (Trait::eq(key, loc)) return;//already contained

    ASSERT(Trait::eq(loc, Trait::EMPTY)
        || Trait::eq(loc, Trait::TOMBSTONE));

    loc = value_t(key);
    used += 1;
    if (needs_resize(0)) {
      try_extend(0);
    }
  }
}

template<typename T, ValidInternalTrait Trait>
void InternalHashSet<T, Trait>::remove(const param_t key) {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  ASSERT(used > 0 && el_capacity > 0);

  value_t& loc = internal_get(key);
  loc = Trait::TOMBSTONE;
  used -= 1;
}

template<typename K, typename T, ValidInternalTrait Trait>
InternalHashTable<K, T, Trait>::~InternalHashTable() {
  if(data != nullptr) {
    value_t* keys = key_arr();
    val_storage_t* vals = val_arr();

    for (size_t i = 0; i < el_capacity; i++) {
      if (!Trait::eq(keys[i], Trait::EMPTY)
          && !Trait::eq(keys[i], Trait::TOMBSTONE)) {
        vals[i].clear(); 
      }
    }

    destruct_arr<value_t>(keys, el_capacity);
    destruct_arr<val_storage_t>(vals, el_capacity);
  }

  free_no_destruct<u8>(data);

  data = nullptr;
  el_capacity = 0;
  used = 0;
}

template<typename K, typename T, ValidInternalTrait Trait>
bool InternalHashTable<K, T, Trait>::contains(const param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if (el_capacity == 0) return false;

  const value_t* keys = key_arr();

  const usize first_index = Trait::hash(key) % el_capacity;

  usize index = first_index;
  do {
    const value_t& test_key = keys[index];
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
size_t InternalHashTable<K, T, Trait>::get_soa_index(const param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  const value_t* const keys = key_arr();

  bool found_tombstone = false;
  usize tombstone_index = 0;

  const usize first_index = Trait::hash(key) % el_capacity;
  usize index = first_index;

  do {
    const value_t& test_key = keys[index];
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
  new (data) value_t[el_capacity];
  new (data + val_arr_offset(el_capacity)) val_storage_t[el_capacity];

  value_t* keys = key_arr();

  for(usize i = 0; i < el_capacity; ++i) {
    keys[i] = Trait::EMPTY;
  }

  if (old_data != nullptr) {

    val_storage_t* values = val_arr();

    value_t* old_keys = key_arr(old_data);
    val_storage_t* old_values = val_arr(old_data, old_el_cap);

    for (size_t i = 0; i < old_el_cap; i++) {
      const value_t& key = old_keys[i];

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
      destruct_arr<value_t>(old_keys, el_capacity);
      destruct_arr<val_storage_t>(old_values, el_capacity);

      free_no_destruct<u8>(old_data);
    }
  }
}

template<typename K, typename T, ValidInternalTrait Trait>
T* InternalHashTable<K, T, Trait>::get_val(const param_t key) const {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if (el_capacity == 0) return nullptr;

  const size_t soa_index = get_soa_index(key);

  value_t& test_key = key_arr()[soa_index];
  if (Trait::eq(key, test_key)) return &val_arr()[soa_index].val;
  else {
    ASSERT(Trait::eq(Trait::EMPTY, test_key)
        || Trait::eq(Trait::TOMBSTONE, test_key));
    return nullptr;
  }
}

template<typename K, typename T, ValidInternalTrait Trait>
void InternalHashTable<K, T, Trait>::insert(const param_t key, T&& val) {
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
    
    value_t& key_loc = key_arr()[soa_index];
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
T* InternalHashTable<K, T, Trait>::get_or_create(const param_t key) requires requires(T t) { {T()}->IS_SAME_TYPE<T>; }
    {
  ASSERT(!Trait::eq(key, Trait::EMPTY)
      && !Trait::eq(key, Trait::TOMBSTONE));
  if (el_capacity == 0) {
    try_extend(1);

    const usize soa_index = get_soa_index(key);

    val_storage_t& val = val_arr()[soa_index];

    key_arr()[soa_index] = key;
    val.val = T();
    used += 1;
    return &val.val;
  }
  else {
    const usize soa_index = get_soa_index(key);

    value_t& test_key = key_arr()[soa_index];

    if (Trait::eq(key, test_key)) {
      return &val_arr()[soa_index].val;
    }
    
    ASSERT(Trait::eq(Trait::EMPTY, test_key)
        || Trait::eq(Trait::TOMBSTONE, test_key));

    val_storage_t& val = val_arr()[soa_index];

    test_key = key;
    val.val = T();
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
