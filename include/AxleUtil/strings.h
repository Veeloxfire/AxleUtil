#ifndef AXLEUTIL_STRINGS_H_
#define AXLEUTIL_STRINGS_H_

#include <AxleUtil/memory.h>
#include <AxleUtil/format.h>

#include <compare>

namespace Axle {
struct InternString {
  uint64_t hash;
  size_t len;
  char* string;

  constexpr bool operator==(const InternString& i)const {
    return i.hash == hash && i.len == len
      && memeq_ts<char>(i.string, string, len);
  }

  constexpr bool operator!=(const InternString& i)const {
    return !operator==(i);
  }
};

namespace Intern {
  inline constexpr InternString TOMBSTONE_STR = {0,0, nullptr};
  inline constexpr const InternString* TOMBSTONE = &TOMBSTONE_STR;
}

template<>
struct Viewable<const InternString*> {
  using ViewT = const char;

  template<typename U>
  static constexpr ViewArr<U> view(const InternString* str) {
    ASSERT(str != Intern::TOMBSTONE && str != nullptr);
    return {str->string, str->len};
  }
};

namespace Format {
  template<>
  struct FormatArg<const InternString*> {
    template<Formatter F>
    constexpr static void load_string(F& res, const InternString* str) {
      ASSERT(str != Intern::TOMBSTONE && str != nullptr);
      if(str->len == 0) {
        ASSERT(str->string == nullptr);
      }
      else {
        res.load_string(str->string, str->len);
      }
    }
  };

  template<>
  struct FormatArg<InternString> {
    template<Formatter F>
    constexpr static void load_string(F& res, const InternString& str) {
      ASSERT(&str != Intern::TOMBSTONE);
      if(str.len == 0) {
        ASSERT(str.string == nullptr);
      }
      else {
        res.load_string(str.string, str.len);
      }
    }
  };
}

constexpr std::strong_ordering lexicographic_order(const ViewArr<const char>& l, const ViewArr<const char>& r) {
  const auto size_order = l.size <=> r.size;
  if(l.data == r.data) return size_order;

  const usize min_size = (size_order < 0) ? l.size : r.size;
  const char* const lstr = l.data;
  const char* const rstr = r.data;

  for (usize i = 0; i < min_size; i++) {
    const char cl = lstr[i];
    const char cr = rstr[i];

    const auto c_order = cl <=> cr;
    if (c_order != 0) {
      return c_order;
    }
  }

  return size_order;
}

constexpr std::strong_ordering lexicographic_order(const InternString* l, const InternString* r) {
  return lexicographic_order(view_arr(l), view_arr(r));
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
  constexpr static usize ALLOC_BLOCK_SIZE = 2048;
  GrowingMemoryPool<ALLOC_BLOCK_SIZE> allocs = {};
  Table table = {};
  InternString empty_string = {};

  StringInterner() = default;
  ~StringInterner() = default;

  // Cannot copy or move
  StringInterner(const StringInterner&) = delete;
  StringInterner(StringInterner&&) = delete;
  StringInterner& operator=(const StringInterner&) = delete;
  StringInterner& operator=(StringInterner&&) = delete;

  const InternString* find(const char* string, size_t len) const;
  const InternString* intern(const char* string, size_t len);

  inline const InternString* find(const ViewArr<const char>& arr) const {
    return find(arr.data, arr.size);
  }

  inline const InternString* find(const OwnedArr<const char>& arr) const {
    return find(arr.data, arr.size);
  }

  inline const InternString* intern(const ViewArr<const char>& arr) {
    return intern(arr.data, arr.size);
  }

  inline const InternString* intern(const OwnedArr<const char>& arr) {
    return intern(arr.data, arr.size);
  }

  template<typename ... T>
  inline const InternString* format_intern(const Format::FormatString& fmt, const T& ... ts) {
    Format::ArrayFormatter formatter = {};

    Format::format_to(formatter, fmt, ts...);

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

    free_no_destruct<u8>(data);

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
        destruct_arr<const InternString*>(old_hash_arr, el_capacity);
        //Dont need to free old values as they've been moved

        free_no_destruct<u8>(old_data);
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
}
#endif

