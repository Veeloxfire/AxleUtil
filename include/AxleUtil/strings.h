#ifndef AXLEUTIL_STRINGS_H_
#define AXLEUTIL_STRINGS_H_

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/format.h>

struct InternString {
  uint64_t hash;
  size_t len;
  char string[1];//placeholder for any length array

  constexpr bool operator==(const InternString& i)const {
    return i.hash == hash && i.len == len
      && memeq_ts<char>(i.string, string, len);
  }

  constexpr bool operator!=(const InternString& i)const {
    return !operator==(i);
  }

  constexpr static size_t alloc_size(size_t string_len) {
    return sizeof(InternString) + string_len + 1 /* for null byte */;
  }
};

template<>
struct Viewable<const InternString*> {
  using ViewT = const char;

  template<typename U>
  static constexpr ViewArr<U> view(const InternString* str) {
    return {str->string, str->len};
  }
};

namespace Format {
  template<>
  struct FormatArg<const InternString*> {
    template<Formatter F>
    constexpr static void load_string(F& res, const InternString* str) {
      res.load_string(str->string, str->len);
    }
  };

  template<>
  struct FormatArg<InternString> {
    template<Formatter F>
    constexpr static void load_string(F& res, const InternString& str) {
      res.load_string(str.string, str.len);
    }
  };
}

namespace Intern {
  inline constexpr InternString TOMBSTONE_STR = {0,0,{'\0'}};
  inline constexpr const InternString* TOMBSTONE = &TOMBSTONE_STR;

  bool is_alphabetical_order(const InternString* l, const InternString* r);
}

struct Table {
  constexpr static float LOAD_FACTOR = 0.75;

  const InternString** data = nullptr;
  size_t num_full = 0;
  size_t size = 0;

  Table();
  ~Table();

  void try_resize();

  const InternString** find(const char* str, size_t len, uint64_t hash) const;
  const InternString** find_empty(uint64_t hash) const;
};

struct StringInterner {
  BumpAllocator allocs = {};
  Table table = {};

  const InternString* intern(const char* string, size_t len);

  inline const InternString* intern(const ViewArr<const char>& arr) {
    return intern(arr.data, arr.size);
  }

  inline const InternString* intern(const OwnedArr<const char>& arr) {
    return intern(arr.data, arr.size);
  }

  template<typename ... T>
  inline const InternString* format_intern(const Format::FormatString& fmt, const T& ... ts) {
    Format::ArrayFormatter formatter = {};

    Format::format_to_formatter(formatter, fmt, ts...);

    return intern(view_arr(formatter));
  }
};

struct InternStringSet {
  constexpr static float LOAD_FACTOR = 0.75;

  const InternString** data = nullptr;// ptr to data in the array
  size_t el_capacity = 0;
  size_t used = 0;

  constexpr bool needs_resize(size_t extra) const {
    return (el_capacity * LOAD_FACTOR) <= (used + extra);
  }

  ~InternStringSet();

  bool contains(const InternString* key) const;
  const InternString** get(const InternString* key) const;
  void try_extend(size_t num);
  void insert(const InternString* const key);
};

template<typename T>
struct InternHashTable {
  constexpr static float LOAD_FACTOR = 0.75;

  uint8_t* data = nullptr;// ptr to data in the array
  size_t el_capacity = 0;
  size_t used = 0;

  struct Iterator {
    InternHashTable<T>* table;
    usize i;

    const InternString* key() const {
      if (!is_valid()) return nullptr;
      return table->key_arr()[i];
    }
    T* val() const {
      if (!is_valid()) return nullptr;
      return table->val_arr() + i;
    }

    constexpr bool is_valid() const {
      return table != nullptr;
    }

    void next() {
      if (table == nullptr) return;

      i += 1;

      const InternString** keys = table->key_arr() + i;
      while (i < table->el_capacity
        && (*keys == nullptr || *keys == Intern::TOMBSTONE)) {
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

  constexpr bool needs_resize(size_t extra) const {
    return (el_capacity * LOAD_FACTOR) <= (used + extra);
  }

  constexpr const InternString** key_arr() const {
    return (const InternString**)data;
  }

  constexpr T* val_arr() const {
    return (T*)(data + (el_capacity * sizeof(const InternString*)));
  }

  ~InternHashTable() {
    {
      const InternString** keys = key_arr();
      T* vals = val_arr();

      for (size_t i = 0; i < el_capacity; i++) {
        const InternString* key = keys[i];

        if (key != nullptr && key != Intern::TOMBSTONE) {
          vals[i].~T();
        }
      }
    }

    free_no_destruct(data);

    data = nullptr;
    el_capacity = 0;
    used = 0;
  }

  bool contains(const InternString* key) const {
    ASSERT(key != nullptr && key != Intern::TOMBSTONE);
    if (el_capacity == 0) return false;

    const InternString** keys = key_arr();

    size_t index = key->hash % el_capacity;

    const InternString* test_key = keys[index];
    while (true) {
      if (test_key == key) {
        return true;
      }
      else if (test_key == nullptr || test_key == Intern::TOMBSTONE) {
        return false;
      }

      index++;
      index %= el_capacity;
      test_key = keys[index];
    }
  }

  size_t get_soa_index(const InternString* key) const {
    ASSERT(key != nullptr && key != Intern::TOMBSTONE);
    const InternString** hash_arr = (const InternString**)data;

    bool found_tombstone = false;
    size_t tombstone_index = 0;

    size_t index = key->hash % el_capacity;

    const InternString* test_key = hash_arr[index];
    while (test_key != nullptr) {
      if (key == test_key) {
        return index;
      }
      else if (test_key == Intern::TOMBSTONE && !found_tombstone) {
        found_tombstone = true;
        tombstone_index = index;
      }

      index++;
      index %= el_capacity;
      test_key = hash_arr[index];
    }

    if (found_tombstone) {
      return tombstone_index;
    }
    else {
      return index;
    }
  }

  void try_extend(size_t num) {
    if (needs_resize(num)) {
      uint8_t* old_data = data;
      const size_t old_el_cap = el_capacity;

      do {
        el_capacity <<= 1;
      } while (needs_resize(num));

      const size_t required_alloc_bytes = el_capacity
        * (sizeof(const InternString*) + sizeof(T));

      data = allocate_default<uint8_t>(required_alloc_bytes);

      const InternString** hash_arr = (const InternString**)data;
      T* val_arr = (T*)(data + el_capacity * sizeof(const InternString*));

      const InternString** old_hash_arr = (const InternString**)old_data;
      T* old_val_arr = (T*)(old_data + old_el_cap * sizeof(const InternString*));

      for (size_t i = 0; i < old_el_cap; i++) {
        const InternString* key = old_hash_arr[i];

        if (key != nullptr && key != Intern::TOMBSTONE) {
          const size_t new_index = get_soa_index(key);

          hash_arr[new_index] = key;
          val_arr[new_index] = std::move(old_val_arr[i]);
        }
      }

      //Destruct and free
      {
        destruct_arr(old_hash_arr, el_capacity);
        //Dont need to free old values as they've been moved

        free_no_destruct(old_data);
      }
    }
  }

  T* get_val(const InternString* const key) const {
    ASSERT(key != nullptr && key != Intern::TOMBSTONE);
    if (el_capacity == 0) return nullptr;

    const size_t soa_index = get_soa_index(key);

    const InternString* test_key = key_arr()[soa_index];
    if (test_key == key) return val_arr() + soa_index;
    else {
      ASSERT(test_key == nullptr || test_key == Intern::TOMBSTONE);
      return nullptr;
    }

  }

  void insert(const InternString* const key, T&& val) {
    ASSERT(key != nullptr && key != Intern::TOMBSTONE);
    if (el_capacity == 0) {
      el_capacity = 8;
      data = allocate_default<uint8_t>(8 * (sizeof(const InternString*) + sizeof(T)));

      size_t soa_index = get_soa_index(key);

      key_arr()[soa_index] = key;
      val_arr()[soa_index] = std::move(val);
      used += 1;
    }
    else {
      size_t soa_index = get_soa_index(key);
      
      const InternString** key_loc = key_arr() + soa_index;
      if (*key_loc == key) {
        val_arr()[soa_index] = std::move(val);
        return;
      }

      ASSERT(*key_loc == nullptr || *key_loc == Intern::TOMBSTONE);

      *key_loc = key;
      val_arr()[soa_index] = std::move(val);
      used += 1;

      if (needs_resize(0)) {
        try_extend(0);
      }
    }
  }

  T* get_or_create(const InternString* const key) {
    ASSERT(key != nullptr && key != Intern::TOMBSTONE);
    if (el_capacity == 0) {
      el_capacity = 8;
      data = allocate_default<uint8_t>(8 * (sizeof(const InternString*) + sizeof(T)));

      size_t soa_index = get_soa_index(key);

      T* const val = val_arr() + soa_index;

      key_arr()[soa_index] = key;
      *val = T{};
      used += 1;
      return val;
    }
    else {
      size_t soa_index = get_soa_index(key);


      const InternString** test_key = key_arr() + soa_index;

      if (*test_key == key) {
        return val_arr() + soa_index;
      }

      ASSERT(*test_key == nullptr || *test_key == Intern::TOMBSTONE);

      T* val = val_arr() + soa_index;

      *test_key = key;
      *val = T{};
      used += 1;

      if (needs_resize(0)) {
        try_extend(0);

        //need to reset the key and val location
        soa_index = get_soa_index(key);

        ASSERT(key_arr()[soa_index] == key);
        return val_arr() + soa_index;
      }
      else {
        return val;
      }
    }
  }
};
#endif
