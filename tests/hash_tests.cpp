#include <AxleUtil/strings.h>
#include <AxleUtil/stdext/compare.h>

#include <AxleTest/unit_tests.h>
using namespace Axle;

TEST_FUNCTION(Hash, InternString_HashSet) {
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

  set.remove(str2);
  set.remove(str4);
  set.remove(str6);
  set.remove(str8);
  set.remove(str10);
  set.remove(str12);

  TEST_EQ(true, set.contains(str1));
  TEST_EQ(false, set.contains(str2));
  TEST_EQ(true, set.contains(str3));
  TEST_EQ(false, set.contains(str4));
  TEST_EQ(true, set.contains(str5));
  TEST_EQ(false, set.contains(str6));
  TEST_EQ(true, set.contains(str7));
  TEST_EQ(false, set.contains(str8));
  TEST_EQ(true, set.contains(str9));
  TEST_EQ(false, set.contains(str10));
  TEST_EQ(true, set.contains(str11));
  TEST_EQ(false, set.contains(str12));

  TEST_NEQ(save_data, set.data);
  TEST_NEQ(el_capacity, set.el_capacity);
  TEST_EQ((usize)6, set.used);
}


TEST_FUNCTION(Hash, InternString_HashTable) {
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

  InternHashTable<int> table = {};
  TEST_EQ(false, table.contains(str1));
  TEST_EQ(false, table.contains(str2));
  TEST_EQ(false, table.contains(str3));
  TEST_EQ(false, table.contains(str4));
  TEST_EQ(false, table.contains(str5));
  TEST_EQ(false, table.contains(str6));
  TEST_EQ(false, table.contains(str7));
  TEST_EQ(false, table.contains(str8));
  TEST_EQ(false, table.contains(str9));
  TEST_EQ(false, table.contains(str10));
  TEST_EQ(false, table.contains(str11));
  TEST_EQ(false, table.contains(str12));
  TEST_EQ((uint8_t*)nullptr, table.data);
  TEST_EQ((usize)0, table.el_capacity);
  TEST_EQ((usize)0, table.used);

  table.insert(str1, 1);
  TEST_EQ(true, table.contains(str1));
  TEST_EQ(false, table.contains(str2));
  TEST_EQ(false, table.contains(str3));
  TEST_EQ(false, table.contains(str4));
  TEST_EQ(false, table.contains(str5));
  TEST_EQ(false, table.contains(str6));
  TEST_EQ(false, table.contains(str7));
  TEST_EQ(false, table.contains(str8));
  TEST_EQ(false, table.contains(str9));
  TEST_EQ(false, table.contains(str10));
  TEST_EQ(false, table.contains(str11));
  TEST_EQ(false, table.contains(str12));

  TEST_NEQ((uint8_t*)nullptr, table.data);
  TEST_NEQ((usize)0, table.el_capacity);
  TEST_EQ((usize)1, table.used);

  TEST_EQ(1, *table.get_val(str1));
  TEST_EQ(1, *table.get_or_create(str1));

  TEST_EQ((int*)nullptr, table.get_val(str2));
  TEST_EQ((int*)nullptr, table.get_val(str3));
  TEST_EQ((int*)nullptr, table.get_val(str4));
  TEST_EQ((int*)nullptr, table.get_val(str5));
  TEST_EQ((int*)nullptr, table.get_val(str6));
  TEST_EQ((int*)nullptr, table.get_val(str7));
  TEST_EQ((int*)nullptr, table.get_val(str8));
  TEST_EQ((int*)nullptr, table.get_val(str9));
  TEST_EQ((int*)nullptr, table.get_val(str10));
  TEST_EQ((int*)nullptr, table.get_val(str11));
  TEST_EQ((int*)nullptr, table.get_val(str12));

  table.insert(str1, 2);
  TEST_EQ(true, table.contains(str1));
  TEST_EQ(false, table.contains(str2));
  TEST_EQ(false, table.contains(str3));
  TEST_EQ(false, table.contains(str4));
  TEST_EQ(false, table.contains(str5));
  TEST_EQ(false, table.contains(str6));
  TEST_EQ(false, table.contains(str7));
  TEST_EQ(false, table.contains(str8));
  TEST_EQ(false, table.contains(str9));
  TEST_EQ(false, table.contains(str10));
  TEST_EQ(false, table.contains(str11));
  TEST_EQ(false, table.contains(str12));

  TEST_EQ((usize)1, table.used);

  TEST_EQ(2, *table.get_val(str1));
  TEST_EQ(2, *table.get_or_create(str1));

  *table.get_or_create(str2) = 3;
  TEST_EQ(true, table.contains(str1));
  TEST_EQ(true, table.contains(str2));
  TEST_EQ(false, table.contains(str3));
  TEST_EQ(false, table.contains(str4));
  TEST_EQ(false, table.contains(str5));
  TEST_EQ(false, table.contains(str6));
  TEST_EQ(false, table.contains(str7));
  TEST_EQ(false, table.contains(str8));
  TEST_EQ(false, table.contains(str9));
  TEST_EQ(false, table.contains(str10));
  TEST_EQ(false, table.contains(str11));
  TEST_EQ(false, table.contains(str12));

  TEST_EQ((usize)2, table.used);

  TEST_EQ(2, *table.get_val(str1));
  TEST_EQ(2, *table.get_or_create(str1));

  TEST_EQ(3, *table.get_val(str2));
  TEST_EQ(3, *table.get_or_create(str2));


  const auto save_data = table.data;
  const auto el_capacity = table.el_capacity;

  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);
  table.insert(str2, 3);

  TEST_EQ(true, table.contains(str1));
  TEST_EQ(true, table.contains(str2));

  TEST_EQ(save_data, table.data);
  TEST_EQ(el_capacity, table.el_capacity);
  TEST_EQ((usize)2, table.used);

  TEST_EQ(false, table.contains(str3));
  TEST_EQ(false, table.contains(str4));
  TEST_EQ(false, table.contains(str5));
  TEST_EQ(false, table.contains(str6));
  TEST_EQ(false, table.contains(str7));
  TEST_EQ(false, table.contains(str8));
  TEST_EQ(false, table.contains(str9));
  TEST_EQ(false, table.contains(str10));
  TEST_EQ(false, table.contains(str11));
  TEST_EQ(false, table.contains(str12));

  TEST_EQ(2, *table.get_val(str1));
  TEST_EQ(2, *table.get_or_create(str1));

  TEST_EQ(3, *table.get_val(str2));
  TEST_EQ(3, *table.get_or_create(str2));

  table.remove(str1);

  TEST_EQ(false, table.contains(str1));
  TEST_EQ(true, table.contains(str2));
  TEST_EQ(static_cast<int*>(nullptr), table.get_val(str1));

  TEST_EQ(3, *table.get_val(str2));
  TEST_EQ(3, *table.get_or_create(str2));

  table.insert(str1, 2);


  {
    int i = table.take(str1);
    TEST_EQ(2, i);

    TEST_EQ(false, table.contains(str1));
    TEST_EQ(true, table.contains(str2));
    TEST_EQ(static_cast<int*>(nullptr), table.get_val(str1));

    TEST_EQ(3, *table.get_val(str2));
    TEST_EQ(3, *table.get_or_create(str2));

    table.insert(str1, 2);
  }

  table.insert(str1, 0);
  table.insert(str2, 1);
  table.insert(str3, 2);
  table.insert(str4, 3);
  table.insert(str5, 4);
  table.insert(str6, 5);
  table.insert(str7, 6);
  table.insert(str8, 7);
  table.insert(str9, 8);
  table.insert(str10, 9);
  table.insert(str11, 10);
  table.insert(str12, 11);

  if (el_capacity == table.el_capacity) {
    test_errors->report_error("Test assert failed!\nLine: {}, Test: {}\nTest was invalid as it didn't make the table grow", __LINE__, test_errors->test_name);
    return;
  }

  TEST_EQ(true, table.contains(str1));
  TEST_EQ(true, table.contains(str2));
  TEST_EQ(true, table.contains(str3));
  TEST_EQ(true, table.contains(str4));
  TEST_EQ(true, table.contains(str5));
  TEST_EQ(true, table.contains(str6));
  TEST_EQ(true, table.contains(str7));
  TEST_EQ(true, table.contains(str8));
  TEST_EQ(true, table.contains(str9));
  TEST_EQ(true, table.contains(str10));
  TEST_EQ(true, table.contains(str11));
  TEST_EQ(true, table.contains(str12));

  TEST_EQ(0, *table.get_val(str1));
  TEST_EQ(1, *table.get_val(str2));
  TEST_EQ(2, *table.get_val(str3));
  TEST_EQ(3, *table.get_val(str4));
  TEST_EQ(4, *table.get_val(str5));
  TEST_EQ(5, *table.get_val(str6));
  TEST_EQ(6, *table.get_val(str7));
  TEST_EQ(7, *table.get_val(str8));
  TEST_EQ(8, *table.get_val(str9));
  TEST_EQ(9, *table.get_val(str10));
  TEST_EQ(10, *table.get_val(str11));
  TEST_EQ(11, *table.get_val(str12));

  TEST_EQ(0, *table.get_or_create(str1));
  TEST_EQ(1, *table.get_or_create(str2));
  TEST_EQ(2, *table.get_or_create(str3));
  TEST_EQ(3, *table.get_or_create(str4));
  TEST_EQ(4, *table.get_or_create(str5));
  TEST_EQ(5, *table.get_or_create(str6));
  TEST_EQ(6, *table.get_or_create(str7));
  TEST_EQ(7, *table.get_or_create(str8));
  TEST_EQ(8, *table.get_or_create(str9));
  TEST_EQ(9, *table.get_or_create(str10));
  TEST_EQ(10, *table.get_or_create(str11));
  TEST_EQ(11, *table.get_or_create(str12));

  TEST_NEQ(save_data, table.data);
  TEST_NEQ(el_capacity, table.el_capacity);
  TEST_EQ((usize)12, table.used);

  auto itr = table.itr();

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


TEST_FUNCTION(Hash, InternString_HashTable_Multiple) {
  StringInterner interner = {};

  const InternString* str1 = interner.intern("hello", 5);
  const InternString* str2 = interner.intern("world", 5);
  const InternString* str3 = interner.intern("a", 1);
  const InternString* str4 = interner.intern("ab", 2);
  const InternString* str5 = interner.intern("abc", 3);
  const InternString* str6 = interner.intern("abcd", 4);
  const InternString* str7 = interner.intern("abcde", 5);
  const InternString* str8 = interner.intern("abcde", 5);
  const InternString* str9 = interner.intern("9", 1);
  const InternString* str10 = interner.intern("10", 2);
  const InternString* str11 = interner.intern("11", 2);
  const InternString* str12 = interner.intern("12", 2);
  const InternString* str13 = interner.intern("13", 2);
  const InternString* str14 = interner.intern("14", 2);
  const InternString* str15 = interner.intern("15", 2);
  const InternString* str16 = interner.intern("16", 2);
  const InternString* str17 = interner.intern("17", 2);
  const InternString* str18 = interner.intern("18", 2);
  const InternString* str19 = interner.intern("19", 2);
  const InternString* str20 = interner.intern("20", 2);

  InternHashTable<int> table = {};

  table.insert(str1, 0);
  table.insert(str2, 1);

  {
    const ConstArray<int*, 3> found = table.get_val_multiple({ str1, str2, str3 });

    TEST_NEQ(static_cast<int*>(nullptr), found[0]);
    TEST_EQ(0, *(found[0]));

    TEST_NEQ(static_cast<int*>(nullptr), found[1]);
    TEST_EQ(1, *(found[1]));

    TEST_EQ(static_cast<int*>(nullptr), found[2]);
  }

  {
    ASSERT(table.needs_resize(18));
    const ConstArray<int*, 18> created = table.get_or_create_multiple({
        str3, str4, str5, str6, str7, str8, str9,
        str10, str11, str12, str13, str14, str15,
        str16, str17, str18, str19, str20,});

    for(int* i : created) {
      TEST_NEQ(static_cast<int*>(nullptr), i);
      TEST_EQ(0, *i);
    }
  }
}

namespace {
  struct FakeKey {
    u64 i;
  };

  struct FakeKeyTrait {
    using value_t = FakeKey;
    using param_t = FakeKey;

    constexpr static const FakeKey EMPTY = FakeKey { 0 };
    constexpr static const FakeKey TOMBSTONE = FakeKey { 1 };

    constexpr static u64 hash(FakeKey k) {
      return k.i;
    }

    constexpr static bool eq(FakeKey k0, FakeKey k1) {
      return k0.i == k1.i;
    }
  };

  struct KeyGenerator {
    u64 i = 2;
    FakeKey operator()() noexcept {
      u64 r = i;
      i += 1;
      return FakeKey{ r };
    }
  };

  struct DestructCounter {
    u64* counter = nullptr;

    DestructCounter() = default;
    DestructCounter(u64* p) : counter(p) {}
    DestructCounter(const DestructCounter&) = default;
    DestructCounter& operator=(const DestructCounter&) = default;

    ~DestructCounter() {
      *counter += 1;
    }
  };

  struct OwnedDestructCounter {
    u64* counter;

    OwnedDestructCounter() : counter(nullptr) {}
    OwnedDestructCounter(u64* p) : counter(p) {}

    OwnedDestructCounter(OwnedDestructCounter&& od)
    : counter(std::exchange(od.counter,nullptr)) {}

    OwnedDestructCounter& operator=(OwnedDestructCounter&& od) {
      if (counter != nullptr) {
        *counter += 1;
      }

      counter = std::exchange(od.counter, nullptr);
      return *this;
    }

    ~OwnedDestructCounter() {
      if (counter != nullptr) {
        *counter += 1;
      }
    }
  };
}

TEST_FUNCTION(Hash, HashTable_destruction) {
  KeyGenerator gen = {};

  u64 expected_out_count = 0;
  u64 counter = 0;
  {
    Hash::InternalHashTable<FakeKey, DestructCounter, FakeKeyTrait> table = {};

    table.insert(gen(), { &counter });
    TEST_EQ(static_cast<u64>(1), counter);
    counter -= 1;
    TEST_EQ(static_cast<usize>(1), table.used);
    
    while (!table.needs_resize(1)) {
      usize past_used = table.used;
      table.insert(gen(), { &counter });
      TEST_EQ(static_cast<u64>(1), counter);
      counter -= 1;

      TEST_EQ(past_used + 1, table.used);
    }

    const usize old_used = table.used;
    const usize old_capacity = table.el_capacity;
    
    {
      ASSERT(table.needs_resize(1));// next insert will resize
      
      TEST_EQ(static_cast<u64>(0), counter);

      auto key = gen();
      table.insert(key, { &counter });
      counter -= 1;// for the temporary
      
      // destroyed all the elements when resizing because they cannot move 
      TEST_EQ(static_cast<u64>(old_used), counter);

      TEST_EQ(old_used + 1, table.used);
      TEST_NEQ(old_capacity, table.el_capacity);
      ASSERT(old_capacity < table.el_capacity);

      table.remove(key);
      TEST_EQ(static_cast<u64>(old_used + 1), counter);
    }
   
    expected_out_count = old_used + 1 /* for removed */ + table.used;
  }
  TEST_EQ(expected_out_count, counter);
}

TEST_FUNCTION(Hash, HashTable_destruction_owned) {
  KeyGenerator gen = {};

  u64 expected_out_count = 0;
  u64 counter = 0;
  {
    Hash::InternalHashTable<FakeKey, OwnedDestructCounter, FakeKeyTrait> table = {};

    table.insert(gen(), { &counter });
    TEST_EQ(static_cast<u64>(0), counter);
    TEST_EQ(static_cast<usize>(1), table.used);
    
    while (!table.needs_resize(1)) {
      usize past_used = table.used;
      table.insert(gen(), { &counter });
      TEST_EQ(static_cast<u64>(0), counter);

      TEST_EQ(past_used + 1, table.used);
    }

    TEST_EQ(static_cast<u64>(0), counter);

    ASSERT(table.needs_resize(1));
    const usize old_used = table.used;
    const usize old_capacity = table.el_capacity;
    
    table.insert(gen(), { &counter });

    TEST_EQ(static_cast<u64>(0), counter);

    TEST_EQ(old_used + 1, table.used);
    TEST_NEQ(old_capacity, table.el_capacity);
    ASSERT(old_capacity < table.el_capacity);
    
    expected_out_count = table.used;
  }
  TEST_EQ(expected_out_count, counter);
}

TEST_FUNCTION(Hash, HashTable_insert_duplicate) {
  KeyGenerator gen = {};

  u64 counter_a = 0;
  u64 counter_b = 0;
  {
    Hash::InternalHashTable<FakeKey, OwnedDestructCounter, FakeKeyTrait> table = {};
  
    FakeKey k = gen();

    table.insert(k, {&counter_a});
    TEST_EQ(static_cast<u64>(0), counter_a);
    TEST_EQ(static_cast<u64>(0), counter_b);

    table.insert(k, {&counter_b});
    TEST_EQ(static_cast<u64>(1), counter_a);
    TEST_EQ(static_cast<u64>(0), counter_b);

    TEST_EQ(&counter_b, table.get_val(k)->counter);
    TEST_EQ(static_cast<u64>(1), counter_a);
    TEST_EQ(static_cast<u64>(0), counter_b);

    TEST_EQ(&counter_b, table.get_or_create(k)->counter);
    TEST_EQ(static_cast<u64>(1), counter_a);
    TEST_EQ(static_cast<u64>(0), counter_b);
  }

  TEST_EQ(static_cast<u64>(1), counter_a);
  TEST_EQ(static_cast<u64>(1), counter_b);
}


TEST_FUNCTION(Hash, HashTable_move) {
  KeyGenerator gen = {};

  u64 counter = 0;
  {
    Hash::InternalHashTable<FakeKey, OwnedDestructCounter, FakeKeyTrait> table = {};

    table.insert(gen(), { &counter });
    TEST_EQ(static_cast<u64>(0), counter);
    TEST_EQ(static_cast<usize>(1), table.used);
    TEST_NEQ(static_cast<usize>(0), table.el_capacity);
    
    Hash::InternalHashTable<FakeKey, OwnedDestructCounter, FakeKeyTrait> table2 = std::move(table);

    TEST_EQ(static_cast<u64>(0), counter);
    TEST_EQ(static_cast<usize>(0), table.used);
    TEST_EQ(static_cast<usize>(0), table.el_capacity);
    TEST_EQ(static_cast<usize>(1), table2.used);
    TEST_NEQ(static_cast<usize>(0), table2.el_capacity);
    
    Hash::InternalHashTable<FakeKey, OwnedDestructCounter, FakeKeyTrait> table3;

    TEST_EQ(static_cast<u64>(0), counter);
    TEST_EQ(static_cast<usize>(0), table.used);
    TEST_EQ(static_cast<usize>(1), table2.used);
    TEST_EQ(static_cast<usize>(0), table3.used);
    TEST_EQ(static_cast<usize>(0), table3.el_capacity);

    table3 = std::move(table2);

    TEST_EQ(static_cast<u64>(0), counter);
    TEST_EQ(static_cast<usize>(0), table.used);
    TEST_EQ(static_cast<usize>(0), table2.used);
    TEST_EQ(static_cast<usize>(0), table2.el_capacity);
    TEST_EQ(static_cast<usize>(1), table3.used);
    TEST_NEQ(static_cast<usize>(0), table3.el_capacity);
  }

  TEST_EQ(static_cast<u64>(1), counter);
}
