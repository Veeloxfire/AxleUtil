#include <AxleUtil/strings.h>
#include <AxleUtil/safe_lib.h>

#ifdef AXLE_TRACING
#include <Tracer/trace.h>
#endif

namespace Axle {
Table::Table() : data(allocate_default<const InternString*>(8)), size(8) {}


Table::~Table() {
  free_destruct_n<const InternString*>(data, size);
  data = nullptr;
  size = 0;
  num_full = 0;
}

const InternString** Table::find(const char* str, size_t len, uint64_t hash) const {
  uint64_t test_index = hash % size;

  const InternString** first_tombstone = nullptr;

  const InternString* el = data[test_index];
  while (el != nullptr) {

    if (el == Intern::TOMBSTONE && first_tombstone == nullptr) {
      //Tombstone space
      first_tombstone =  data + test_index;
    }
    else if (el->hash == hash && el->len == len && memeq_ts<char>(str, el->string, len)) {
      //Success
      return data + test_index;
    }

    //Try next one
    test_index++;
    test_index %= size;
    el = data[test_index];
  }

  //Test for tombstone
  if (first_tombstone != nullptr) {
    return first_tombstone;
  }
  else {
    return data + test_index;
  }
}

const InternString** Table::find_empty(uint64_t hash) const {
  uint64_t test_index = hash % size;

  const InternString* el = data[test_index];
  while (true) {

    if (el == nullptr || el == Intern::TOMBSTONE) {
      //Empty space
      return data + test_index;
    }

    //Try next one
    test_index++;
    test_index %= size;
    el = data[test_index];
  }
}

void Table::try_resize() {
  if (num_full >= static_cast<usize>(static_cast<float>(size) * LOAD_FACTOR)) {
    const size_t old_size = size;
    const InternString** const old_data = data;

    do {
      size <<= 1;
    } while (num_full >= static_cast<usize>(static_cast<float>(size) * LOAD_FACTOR));
    data = allocate_default<const InternString*>(size);

    {
      auto i = old_data;
      const auto end = old_data + old_size;
      for (; i < end; i++) {
        const InternString* i_str = *i;

        if (i_str != nullptr && i_str != Intern::TOMBSTONE) {
          auto** place = find_empty(i_str->hash);
          *place = i_str;
        }
      }
    }

    free_no_destruct<const InternString*>(old_data);
  }
}

static void destroy_is(void* is) {
  InternString* i = reinterpret_cast<InternString*>(is);
  destruct_arr<const char>(i->string, i->len + 1);
  destruct_single<InternString>(i);
}

static void destroy_is_big(void* is) {
  InternString* i = reinterpret_cast<InternString*>(is);
  usize original_size = sizeof(InternString) * i->len + 1;
  destruct_arr<const char>(i->string, i->len + 1);
  destruct_single<InternString>(i);

  free_destruct_n<u8>(reinterpret_cast<u8*>(is), original_size);
}

const InternString* StringInterner::find(const char* string, const size_t length) const {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  STACKTRACE_FUNCTION();

  if(string == nullptr || length == 0) {
    ASSERT(string == 0 && length == 0);
    return &empty_string;
  }

  ASSERT(string != nullptr && length > 0);

  const uint64_t hash = fnv1a_hash(string, length);

  const InternString** const place = table.find(string, length, hash);

  const InternString* el = *place;
  if (el == nullptr || el == Intern::TOMBSTONE) {
    return nullptr;
  }
  else {
    return el;
  }
}

const InternString* StringInterner::intern(const char* string, const size_t length) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  STACKTRACE_FUNCTION();
  
  if(string == nullptr || length == 0) {
    ASSERT(string == 0 && length == 0);
    return &empty_string;
  }

  ASSERT(string != nullptr && length > 0);

  const uint64_t hash = fnv1a_hash(string, length);

  const InternString** const place = table.find(string, length, hash);

  const InternString* el = *place;
  if (el == nullptr || el == Intern::TOMBSTONE) {
    InternString* new_el;
    char* string_data;
    
    {
      usize alloc_size = sizeof(InternString) + length + 1;
      auto* dl = allocs.alloc_destruct_element();
      if(allocs.is_big_alloc(alloc_size)) {
        dl->deleter = &destroy_is_big;
      }
      else {
        dl->deleter = &destroy_is;
      }

      void* mem = allocs.alloc_raw(alloc_size, alignof(InternString));
      mem = std::assume_aligned<alignof(InternString)>(mem);
      
      new_el = new(mem) InternString();


      string_data = new(reinterpret_cast<u8*>(mem) + sizeof(InternString)) char[length + 1];
      new_el->string = string_data;

      dl->data = new_el;
    }

    new_el->hash = hash;
    new_el->len = length;
  
    ASSERT(new_el->string == string_data);
    memcpy_ts(string_data, length + 1, string, length);
    string_data[length] = '\0';
    
    *place = new_el;

    table.num_full++;
    table.try_resize();
    return new_el;
  }
  else {
    return el;
  }
}
}
