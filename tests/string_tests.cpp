#include <AxleUtil/strings.h>
#include <AxleUtil/stdext/compare.h>

#include <AxleTest/unit_tests.h>
using namespace Axle;

TEST_FUNCTION(Interned_Strings, creation) {
  StringInterner interner = {};

  const InternString* str1 = interner.intern("hello", 5);
  const u64 hash = fnv1a_hash("hello", 5);

  TEST_ARR_EQ("hello", (usize)6, str1->string, str1->len + 1);//actually adds a null byte on
  TEST_EQ(hash, str1->hash);

  const InternString* str2 = interner.intern("hello", 5);
  TEST_EQ(str1, str2);
  TEST_EQ(*str1, *str2);

  TEST_ARR_EQ("hello", (usize)5, str2->string, str2->len);
  TEST_EQ(hash, str2->hash);

  const InternString* str3 = interner.intern(copy_arr("hello", 5));
  TEST_EQ(str1, str3);
  TEST_EQ(str2, str3);

  TEST_ARR_EQ("hello", (usize)5, str3->string, str3->len);
  TEST_EQ(hash, str3->hash);

  const InternString* str4 = interner.intern(lit_view_arr("hello"));
  TEST_EQ(str1, str4);
  TEST_EQ(str2, str4);
  TEST_EQ(str3, str4);

  TEST_ARR_EQ("hello", (usize)5, str4->string, str4->len);
  TEST_EQ(hash, str4->hash);

  const InternString* str5 = interner.format_intern("{}", lit_view_arr("hello"));
  TEST_EQ(str1, str5);
  TEST_EQ(str2, str5);
  TEST_EQ(str3, str5);
  TEST_EQ(str5, str5);

  TEST_ARR_EQ("hello", (usize)5, str5->string, str5->len);
  TEST_EQ(hash, str5->hash);

  const InternString* str6 = interner.intern("hello2", 6);
  const u64 hash2 = fnv1a_hash("hello2", 6);

  TEST_ARR_EQ("hello2", (usize)6, str6->string, str6->len);
  TEST_EQ(hash2, str6->hash);
  TEST_NEQ(str1, str6);
  TEST_NEQ(str2, str6);
  TEST_NEQ(str3, str6);
  TEST_NEQ(str4, str6);
  TEST_NEQ(str5, str6);

  TEST_NEQ(*str1, *str6);

  StringInterner interner2 = {};

  const InternString* str7 = interner2.intern("hello", 5);
  TEST_NEQ(str1, str7);
  TEST_NEQ(str2, str7);
  TEST_NEQ(str3, str7);
  TEST_NEQ(str4, str7);
  TEST_NEQ(str5, str7);
  TEST_NEQ(str6, str7);

  TEST_EQ(*str1, *str7);

  TEST_ARR_EQ("hello", (usize)5, str7->string, str7->len);
  TEST_EQ(hash, str7->hash);
}

TEST_FUNCTION(Interned_Strings, empty_string) {
  StringInterner interner = {};

  const InternString* e1 = &interner.empty_string;

  const InternString* s1 = interner.intern(nullptr, 0);
  const InternString* s2 = interner.intern(nullptr, 0);

  TEST_EQ(e1, s1);
  TEST_EQ(e1, s2);
  TEST_EQ(s1, s2);
  TEST_EQ(static_cast<usize>(0), e1->len);
  TEST_EQ(static_cast<usize>(0), s1->len);
  TEST_EQ(static_cast<usize>(0), s2->len);

  {
    const ViewArr<const char> e_raw = view_arr(e1);
    TEST_EQ(static_cast<const char*>(nullptr), e_raw.data);
    TEST_EQ(static_cast<usize>(0), e_raw.size);
  }

  {
    Format::ArrayFormatter fmt = {};
    Format::format_to(fmt, "{}", e1);

    const auto out = fmt.view();
    TEST_ARR_EQ(static_cast<const char*>(""), static_cast<usize>(0), out.data, out.size);
  }
}

TEST_FUNCTION(Interned_Strings, find) {
  StringInterner interner = {};

  const InternString* str1 = interner.intern("hello", 5);
  const u64 hash = fnv1a_hash("hello", 5);

  TEST_ARR_EQ("hello", (usize)6, str1->string, str1->len + 1);//actually adds a null byte on
  TEST_EQ(hash, str1->hash);

  const InternString* str2 = interner.find("hello", 5);
  TEST_EQ(str1, str2);
  TEST_EQ(*str1, *str2);

  TEST_ARR_EQ("hello", (usize)5, str2->string, str2->len);
  TEST_EQ(hash, str2->hash);

  {
    const InternString* str_no = interner.find("helloo", 6);
    TEST_EQ(static_cast<const InternString*>(nullptr), str_no);
  }

  const InternString* str3 = interner.find(copy_arr("hello", 5));
  TEST_EQ(str1, str3);
  TEST_EQ(str2, str3);

  TEST_ARR_EQ("hello", (usize)5, str3->string, str3->len);
  TEST_EQ(hash, str3->hash);

  const InternString* str4 = interner.find(lit_view_arr("hello"));
  TEST_EQ(str1, str4);
  TEST_EQ(str2, str4);
  TEST_EQ(str3, str4);

  TEST_ARR_EQ("hello", (usize)5, str4->string, str4->len);
  TEST_EQ(hash, str4->hash);
}


TEST_FUNCTION(Interned_Strings, big_creation) {
  constexpr usize SIZE = StringInterner::ALLOC_BLOCK_SIZE * 2;
  OwnedArr<char> str = new_arr<char>(SIZE);
  
  const char options[] = {'a', 'b','c','d','e','f','g'};
  for(usize i = 0; i < SIZE; ++i) {
    str[i] = options[i % array_size(options)];
  }

  StringInterner strings = {};
  
  const InternString* s = strings.intern(view_arr(str));

  TEST_NEQ((const InternString*)nullptr, s);
  TEST_STR_EQ(str, s); 
}

TEST_FUNCTION(Interned_Strings, order) {
  StringInterner interner = {};

  const InternString* str1 = interner.intern("hello", 5);
  const InternString* str2 = interner.intern("world", 5);
  const InternString* str3 = interner.intern("hello", 5);

  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str1, str1));
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str2, str2));
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str3, str3));

  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str1, str3));
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str3, str1));

  TEST_EQ(std::strong_ordering::less, lexicographic_order(str1, str2));
  TEST_EQ(std::strong_ordering::greater, lexicographic_order(str2, str1));
}

TEST_FUNCTION(Interned_Strings, order) {
  StringInterner interner = {};

  const char str1_holder[] = "hello";
  const char str3_holder[] = "hello";

  auto str1 = Axle::lit_view_arr(str1_holder);
  auto str2 = Axle::lit_view_arr("world");
  auto str3 = Axle::lit_view_arr(str3_holder);

  TEST_NEQ(str1.data, str3.data);

  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str1, str1));
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str2, str2));
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str3, str3));
  
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str1, str3));
  TEST_EQ(std::strong_ordering::equivalent, lexicographic_order(str3, str1));

  TEST_EQ(std::strong_ordering::less, lexicographic_order(str1, str2));
  TEST_EQ(std::strong_ordering::less, lexicographic_order(str3, str2));
  TEST_EQ(std::strong_ordering::greater, lexicographic_order(str2, str1));
  TEST_EQ(std::strong_ordering::greater, lexicographic_order(str2, str3));

  TEST_EQ(std::strong_ordering::greater, lexicographic_order(str1, view_arr(str3, 0, str3.size - 1)));
  TEST_EQ(std::strong_ordering::less, lexicographic_order(view_arr(str3, 0, str3.size - 1), str1));
  
  TEST_EQ(std::strong_ordering::greater, lexicographic_order(str1, view_arr(str1, 0, str1.size - 1)));
  TEST_EQ(std::strong_ordering::less, lexicographic_order(view_arr(str1, 0, str1.size - 1), str1));
}

