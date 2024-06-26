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

TEST_FUNCTION(Interned_Strings, set) {
  StringInterner interner = {};

  const InternString* str1 = interner.intern("hello", 5);
  const InternString* str2 = interner.intern("world", 5);
  const InternString* str3 = interner.intern("a", 1);
  const InternString* str4 = interner.intern("ab", 2);
  const InternString* str5 = interner.intern("abc", 3);
  const InternString* str6 = interner.intern("abcd", 4);
  const InternString* str7 = interner.intern("abcde", 5);
  const InternString* str8 = interner.intern("abcdef", 6);
  const InternString* str9 = interner.intern("abcdefg", 7);
  const InternString* str10 = interner.intern("abcdefgh", 8);
  const InternString* str11 = interner.intern("abcdefghi", 9);
  const InternString* str12 = interner.intern("abcdefghij", 10);


  InternStringSet set = {};
  TEST_EQ(false, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));
  TEST_EQ((const InternString**)nullptr, set.data);
  TEST_EQ((usize)0, set.el_capacity);
  TEST_EQ((usize)0, set.used);
  
  set.insert(str1);
  TEST_EQ(true, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_NEQ((const InternString**)nullptr, set.data);
  TEST_NEQ((usize)0, set.el_capacity);
  TEST_EQ((usize)1, set.used);

  set.insert(str1);
  TEST_EQ(true, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_EQ((usize)1, set.used);

  set.insert(str2);
  TEST_EQ(true, set.contains(str1));
  TEST_EQ(true, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_EQ((usize)2, set.used);

  
  const auto save_data = set.data;
  const auto el_capacity = set.el_capacity;

  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);
  set.insert(str2);

  TEST_EQ(true, set.contains(str1));
  TEST_EQ(true, set.contains(str2));

  TEST_EQ(save_data, set.data);
  TEST_EQ(el_capacity, set.el_capacity);
  TEST_EQ((usize)2, set.used);
  
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  set.insert(str1);
  set.insert(str2);
  set.insert(str3);
  set.insert(str4);
  set.insert(str5);
  set.insert(str6);
  set.insert(str7);
  set.insert(str8);
  set.insert(str9);
  set.insert(str10);
  set.insert(str11);
  set.insert(str12);

  if (el_capacity == set.el_capacity) {
    test_errors->report_error("Test assert failed!\nLine: {}, Test: {}\nTest was invalid as it didn't make the set grow", __LINE__, test_errors->test_name);
    return;
  }

  TEST_EQ(true, set.contains(str1));
  TEST_EQ(true, set.contains(str2));
  TEST_EQ(true, set.contains(str3));
  TEST_EQ(true, set.contains(str4));
  TEST_EQ(true, set.contains(str5));
  TEST_EQ(true, set.contains(str6));
  TEST_EQ(true, set.contains(str7));
  TEST_EQ(true, set.contains(str8));
  TEST_EQ(true, set.contains(str9));
  TEST_EQ(true, set.contains(str10));
  TEST_EQ(true, set.contains(str11));
  TEST_EQ(true, set.contains(str12));

  TEST_NEQ(save_data, set.data);
  TEST_NEQ(el_capacity, set.el_capacity);
  TEST_EQ((usize)12, set.used);
}


TEST_FUNCTION(Interned_Strings, set) {
  StringInterner interner = {};

  const InternString* str1 = interner.intern("hello", 5);
  const InternString* str2 = interner.intern("world", 5);
  const InternString* str3 = interner.intern("a", 1);
  const InternString* str4 = interner.intern("ab", 2);
  const InternString* str5 = interner.intern("abc", 3);
  const InternString* str6 = interner.intern("abcd", 4);
  const InternString* str7 = interner.intern("abcde", 5);
  const InternString* str8 = interner.intern("abcdef", 6);
  const InternString* str9 = interner.intern("abcdefg", 7);
  const InternString* str10 = interner.intern("abcdefgh", 8);
  const InternString* str11 = interner.intern("abcdefghi", 9);
  const InternString* str12 = interner.intern("abcdefghij", 10);

  TEST_NEQ(str1, str2);
  TEST_NEQ(str1, str3);
  TEST_NEQ(str1, str4);
  TEST_NEQ(str1, str5);
  TEST_NEQ(str1, str6);
  TEST_NEQ(str1, str7);
  TEST_NEQ(str1, str8);
  TEST_NEQ(str1, str9);
  TEST_NEQ(str1, str10);
  TEST_NEQ(str1, str11);
  TEST_NEQ(str1, str12);
  TEST_NEQ(str2, str3);
  TEST_NEQ(str2, str4);
  TEST_NEQ(str2, str5);
  TEST_NEQ(str2, str6);
  TEST_NEQ(str2, str7);
  TEST_NEQ(str2, str8);
  TEST_NEQ(str2, str9);
  TEST_NEQ(str2, str10);
  TEST_NEQ(str2, str11);
  TEST_NEQ(str2, str12);
  TEST_NEQ(str3, str4);
  TEST_NEQ(str3, str5);
  TEST_NEQ(str3, str6);
  TEST_NEQ(str3, str7);
  TEST_NEQ(str3, str8);
  TEST_NEQ(str3, str9);
  TEST_NEQ(str3, str10);
  TEST_NEQ(str3, str11);
  TEST_NEQ(str3, str12);
  TEST_NEQ(str4, str5);
  TEST_NEQ(str4, str6);
  TEST_NEQ(str4, str7);
  TEST_NEQ(str4, str8);
  TEST_NEQ(str4, str9);
  TEST_NEQ(str4, str10);
  TEST_NEQ(str4, str11);
  TEST_NEQ(str4, str12);
  TEST_NEQ(str5, str6);
  TEST_NEQ(str5, str7);
  TEST_NEQ(str5, str8);
  TEST_NEQ(str5, str9);
  TEST_NEQ(str5, str10);
  TEST_NEQ(str5, str11);
  TEST_NEQ(str5, str12);
  TEST_NEQ(str6, str7);
  TEST_NEQ(str6, str8);
  TEST_NEQ(str6, str9);
  TEST_NEQ(str6, str10);
  TEST_NEQ(str6, str11);
  TEST_NEQ(str6, str12);
  TEST_NEQ(str7, str8);
  TEST_NEQ(str7, str9);
  TEST_NEQ(str7, str10);
  TEST_NEQ(str7, str11);
  TEST_NEQ(str7, str12);
  TEST_NEQ(str8, str9);
  TEST_NEQ(str8, str10);
  TEST_NEQ(str8, str11);
  TEST_NEQ(str8, str12);
  TEST_NEQ(str9, str10);
  TEST_NEQ(str9, str11);
  TEST_NEQ(str9, str12);
  TEST_NEQ(str10, str11);
  TEST_NEQ(str10, str12);
  TEST_NEQ(str11, str12);

  InternHashTable<int> set = {};
  TEST_EQ(false, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));
  TEST_EQ((uint8_t*)nullptr, set.data);
  TEST_EQ((usize)0, set.el_capacity);
  TEST_EQ((usize)0, set.used);

  set.insert(str1, 1);
  TEST_EQ(true, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_NEQ((uint8_t*)nullptr, set.data);
  TEST_NEQ((usize)0, set.el_capacity);
  TEST_EQ((usize)1, set.used);

  TEST_EQ(1, *set.get_val(str1));
  TEST_EQ(1, *set.get_or_create(str1));

  TEST_EQ((int*)nullptr, set.get_val(str2));
  TEST_EQ((int*)nullptr, set.get_val(str3));
  TEST_EQ((int*)nullptr, set.get_val(str4));
  TEST_EQ((int*)nullptr, set.get_val(str5));
  TEST_EQ((int*)nullptr, set.get_val(str6));
  TEST_EQ((int*)nullptr, set.get_val(str7));
  TEST_EQ((int*)nullptr, set.get_val(str8));
  TEST_EQ((int*)nullptr, set.get_val(str9));
  TEST_EQ((int*)nullptr, set.get_val(str10));
  TEST_EQ((int*)nullptr, set.get_val(str11));
  TEST_EQ((int*)nullptr, set.get_val(str12));

  set.insert(str1, 2);
  TEST_EQ(true, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_EQ((usize)1, set.used);

  TEST_EQ(2, *set.get_val(str1));
  TEST_EQ(2, *set.get_or_create(str1));

  *set.get_or_create(str2) = 3;
  TEST_EQ(true, set.contains(str1));
  TEST_EQ(true, set.contains(str2));
  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_EQ((usize)2, set.used);

  TEST_EQ(2, *set.get_val(str1));
  TEST_EQ(2, *set.get_or_create(str1));

  TEST_EQ(3, *set.get_val(str2));
  TEST_EQ(3, *set.get_or_create(str2));


  const auto save_data = set.data;
  const auto el_capacity = set.el_capacity;

  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);
  set.insert(str2, 3);

  TEST_EQ(true, set.contains(str1));
  TEST_EQ(true, set.contains(str2));

  TEST_EQ(save_data, set.data);
  TEST_EQ(el_capacity, set.el_capacity);
  TEST_EQ((usize)2, set.used);

  TEST_EQ(false, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(false, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(false, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(false, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(false, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_EQ(2, *set.get_val(str1));
  TEST_EQ(2, *set.get_or_create(str1));

  TEST_EQ(3, *set.get_val(str2));
  TEST_EQ(3, *set.get_or_create(str2));

  set.insert(str1, 0);
  set.insert(str2, 1);
  set.insert(str3, 2);
  set.insert(str4, 3);
  set.insert(str5, 4);
  set.insert(str6, 5);
  set.insert(str7, 6);
  set.insert(str8, 7);
  set.insert(str9, 8);
  set.insert(str10, 9);
  set.insert(str11, 10);
  set.insert(str12, 11);

  if (el_capacity == set.el_capacity) {
    test_errors->report_error("Test assert failed!\nLine: {}, Test: {}\nTest was invalid as it didn't make the set grow", __LINE__, test_errors->test_name);
    return;
  }

  TEST_EQ(true, set.contains(str1));
  TEST_EQ(true, set.contains(str2));
  TEST_EQ(true, set.contains(str3));
  TEST_EQ(true, set.contains(str4));
  TEST_EQ(true, set.contains(str5));
  TEST_EQ(true, set.contains(str6));
  TEST_EQ(true, set.contains(str7));
  TEST_EQ(true, set.contains(str8));
  TEST_EQ(true, set.contains(str9));
  TEST_EQ(true, set.contains(str10));
  TEST_EQ(true, set.contains(str11));
  TEST_EQ(true, set.contains(str12));

  TEST_EQ(0, *set.get_val(str1));
  TEST_EQ(1, *set.get_val(str2));
  TEST_EQ(2, *set.get_val(str3));
  TEST_EQ(3, *set.get_val(str4));
  TEST_EQ(4, *set.get_val(str5));
  TEST_EQ(5, *set.get_val(str6));
  TEST_EQ(6, *set.get_val(str7));
  TEST_EQ(7, *set.get_val(str8));
  TEST_EQ(8, *set.get_val(str9));
  TEST_EQ(9, *set.get_val(str10));
  TEST_EQ(10, *set.get_val(str11));
  TEST_EQ(11, *set.get_val(str12));

  TEST_EQ(0, *set.get_or_create(str1));
  TEST_EQ(1, *set.get_or_create(str2));
  TEST_EQ(2, *set.get_or_create(str3));
  TEST_EQ(3, *set.get_or_create(str4));
  TEST_EQ(4, *set.get_or_create(str5));
  TEST_EQ(5, *set.get_or_create(str6));
  TEST_EQ(6, *set.get_or_create(str7));
  TEST_EQ(7, *set.get_or_create(str8));
  TEST_EQ(8, *set.get_or_create(str9));
  TEST_EQ(9, *set.get_or_create(str10));
  TEST_EQ(10, *set.get_or_create(str11));
  TEST_EQ(11, *set.get_or_create(str12));

  TEST_NEQ(save_data, set.data);
  TEST_NEQ(el_capacity, set.el_capacity);
  TEST_EQ((usize)12, set.used);

  auto itr = set.itr();

  bool found[12] = {false};

  while (itr.is_valid()) {
    const InternString* k = itr.key();
    const int* v = itr.val();

    if (k == str1) {
      TEST_EQ(false, found[0]);
      found[0] = true;
      TEST_EQ(0, *v);
    }
    else if (k == str2) {
      TEST_EQ(false, found[1]);
      found[1] = true;
      TEST_EQ(1, *v);
    }
    else if (k == str3) {
      TEST_EQ(false, found[2]);
      found[2] = true;
      TEST_EQ(2, *v);
    }
    else if (k == str4) {
      TEST_EQ(false, found[3]);
      found[3] = true;
      TEST_EQ(3, *v);
    }
    else if (k == str5) {
      TEST_EQ(false, found[4]);
      found[4] = true;
      TEST_EQ(4, *v);
    }
    else if (k == str6) {
      TEST_EQ(false, found[5]);
      found[5] = true;
      TEST_EQ(5, *v);
    }
    else if (k == str7) {
      TEST_EQ(false, found[6]);
      found[6] = true;
      TEST_EQ(6, *v);
    }
    else if (k == str8) {
      TEST_EQ(false, found[7]);
      found[7] = true;
      TEST_EQ(7, *v);
    }
    else if (k == str9) {
      TEST_EQ(false, found[8]);
      found[8] = true;
      TEST_EQ(8, *v);
    }
    else if (k == str10) {
      TEST_EQ(false, found[9]);
      found[9] = true;
      TEST_EQ(9, *v);
    }
    else if (k == str11) {
      TEST_EQ(false, found[10]);
      found[10] = true;
      TEST_EQ(10, *v);
    }
    else if (k == str12) {
      TEST_EQ(false, found[11]);
      found[11] = true;
      TEST_EQ(11, *v);
    }
    else {
      test_errors->report_error("Test assert failed!\nLine: {}, Test: {}\nFound string inside the hash-table that I didn't put there", __LINE__, test_errors->test_name);
      return;
    }

    itr.next();
  }

  //Should have found all of them
  for (bool b : found) {
    TEST_EQ(true, b);
  }

  TEST_EQ((const InternString*)nullptr, itr.key());
  TEST_EQ((int*)nullptr, itr.val());

  for (int i = 0; i < 100; ++i) {//should never become valid
    itr.next();
    TEST_EQ((const InternString*)nullptr, itr.key());
    TEST_EQ((int*)nullptr, itr.val());
  }
}
