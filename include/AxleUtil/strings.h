#ifndef AXLEUTIL_STRINGS_H_
#define AXLEUTIL_STRINGS_H_

#include <AxleUtil/hash.h>
#include <AxleUtil/format.h>

#include <compare>

namespace Axle {
struct InternString {
  uint64_t hash = 0;
  size_t len = 0;
  const char* string = nullptr;

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
  inline const InternString* format_intern(const Format::FormatString<T...>& fmt, const T& ... ts) {
    Format::ArrayFormatter formatter = {};

    Format::format_to(formatter, fmt, ts...);

    return intern(view_arr(formatter));
  }
};

namespace Hash {
  template<>
  struct DefaultHashmapTrait<const InternString*> {
    using value_t = const InternString*;
    using param_t = const InternString*;

    static constexpr const value_t EMPTY = nullptr;
    static constexpr const value_t TOMBSTONE = Intern::TOMBSTONE;

    static constexpr usize hash(param_t s) noexcept {
      return s->hash;
    }
    static constexpr bool eq(param_t s0, param_t s1) noexcept {
      return s0 == s1;
    }
  };
}

using InternStringSet = Hash::InternalHashSet<const InternString*>;

template<typename T>
using InternHashTable = Hash::InternalHashTable<const InternString*, T>;
}
#endif

