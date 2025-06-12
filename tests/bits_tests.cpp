#include <AxleUtil/bits.h>

#include <AxleTest/unit_tests.h>
using namespace Axle;

TEST_FUNCTION(BIT_ARRAY, construct) {
  BitArray arr = {};

  TEST_EQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(0), arr.size);
  TEST_EQ(static_cast<usize>(0), arr.capacity);

  arr.insert_set();
  TEST_NEQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(1), arr.size);
  TEST_EQ(true, arr.test(0));

  BitArray arr2 = std::move(arr);
  TEST_EQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(0), arr.size);
  TEST_EQ(static_cast<usize>(0), arr.capacity);

  TEST_NEQ(static_cast<u8*>(nullptr), arr2.data);
  TEST_EQ(static_cast<usize>(1), arr2.size);
  TEST_EQ(true, arr2.test(0));

  BitArray arr3 = {};
  TEST_EQ(static_cast<u8*>(nullptr), arr3.data);
  TEST_EQ(static_cast<usize>(0), arr3.size);
  TEST_EQ(static_cast<usize>(0), arr3.capacity);

  arr3 = std::move(arr2);

  TEST_EQ(static_cast<u8*>(nullptr), arr2.data);
  TEST_EQ(static_cast<usize>(0), arr2.size);
  TEST_EQ(static_cast<usize>(0), arr2.capacity);

  TEST_NEQ(static_cast<u8*>(nullptr), arr3.data);
  TEST_EQ(static_cast<usize>(1), arr3.size);
  TEST_EQ(true, arr3.test(0));
}

TEST_FUNCTION(BIT_ARRAY, insert) {
  BitArray arr = {};

  TEST_EQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(0), arr.size);
  TEST_EQ(static_cast<usize>(0), arr.capacity);

  arr.insert_set();
  TEST_NEQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(1), arr.size);
  TEST_EQ(true, arr.test(0));

  arr.insert_unset();
  TEST_NEQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(2), arr.size);
  TEST_EQ(true, arr.test(0));
  TEST_EQ(false, arr.test(1));

  arr.insert(true);
  arr.insert(false);
  TEST_NEQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(4), arr.size);
  TEST_EQ(true, arr.test(0));
  TEST_EQ(false, arr.test(1));
  TEST_EQ(true, arr.test(2));
  TEST_EQ(false, arr.test(3));
  
  arr.clear();
  TEST_EQ(static_cast<u8*>(nullptr), arr.data);
  TEST_EQ(static_cast<usize>(0), arr.size);
  TEST_EQ(static_cast<usize>(0), arr.capacity);
}

TEST_FUNCTION(SET_BIT_ARRAY, insert) {
  SetBitArray arr = SetBitArray(3);

  TEST_EQ(static_cast<usize>(3), arr.length);
  TEST_EQ(static_cast<usize>(0), arr.highest_set);

  arr.set(1);
  {
    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(1), arr.highest_set);
    TEST_EQ(true, arr.test(1));

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }

  arr.set(2);
  {
    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(2), arr.highest_set);
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test_all());
  }

  arr.set(0);
  {
    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(2), arr.highest_set);

    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));

    TEST_EQ(true, arr.test_all());
  }

  {
    usize set = arr.count_set();
    usize unset = arr.count_unset();

    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(2), arr.highest_set);
    TEST_EQ(static_cast<usize>(3), set);
    TEST_EQ(static_cast<usize>(0), unset);

    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));
    TEST_EQ(true, arr.test_all());
  }

  {
    auto unset_itr = arr.unset_itr();

    usize a = unset_itr.next();
    usize b = unset_itr.next();

    TEST_EQ(static_cast<usize>(3), a);
    TEST_EQ(static_cast<usize>(3), b);


    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(2), arr.highest_set);
    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));
    TEST_EQ(true, arr.test_all());
  }

  arr.clear();
  {
    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(0), arr.highest_set);

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
  }

  {
    usize set = arr.count_set();
    usize unset = arr.count_unset();

    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(0), arr.highest_set);
    TEST_EQ(static_cast<usize>(0), set);
    TEST_EQ(static_cast<usize>(3), unset);

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }

  {
    auto unset_itr = arr.unset_itr();

    usize a = unset_itr.next();
    usize b = unset_itr.next();
    usize c = unset_itr.next();
    usize d = unset_itr.next();
    usize e = unset_itr.next();

    TEST_EQ(static_cast<usize>(0), a);
    TEST_EQ(static_cast<usize>(1), b);
    TEST_EQ(static_cast<usize>(2), c);
    TEST_EQ(static_cast<usize>(3), d);
    TEST_EQ(static_cast<usize>(3), e);


    TEST_EQ(static_cast<usize>(3), arr.length);
    TEST_EQ(static_cast<usize>(0), arr.highest_set);
    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }
}

TEST_FUNCTION(SET_BIT_ARRAY, big_insert) {

  SetBitArray arr = SetBitArray(134);
  TEST_EQ(static_cast<usize>(134), arr.length);
  TEST_EQ(static_cast<usize>(0), arr.highest_set);

  arr.set(57);
  TEST_EQ(true, arr.test(57));
  TEST_EQ(static_cast<usize>(57), arr.highest_set);

  {
    for (usize i = 0; i < 57; ++i) {
      TEST_EQ(false, arr.test(i));
    }

    for (usize i = 58; i < 134; ++i) {
      TEST_EQ(false, arr.test(i));
    }
  }

  TEST_EQ(false, arr.test_all()); 
  arr.clear();

  TEST_EQ(static_cast<usize>(0), arr.highest_set);
  for (usize i = 0; i < 134; ++i) {
    TEST_EQ(false, arr.test(i));
  }

  {
    for (usize i = 1; i < 34; ++i) {
      arr.set(i);
    }

    for (usize i = 35; i < 57; ++i) {
      arr.set(i);
    }

    for (usize i = 58; i < 134; ++i) { 
      arr.set(i);
    }
  }

  {
    usize set = arr.count_set();
    usize unset = arr.count_unset();

    TEST_EQ(static_cast<usize>(131), set);
    TEST_EQ(static_cast<usize>(3), unset);
  }

  {
    auto unset_itr = arr.unset_itr();

    usize a = unset_itr.next();
    usize b = unset_itr.next();
    usize c = unset_itr.next();
    usize d = unset_itr.next();
    usize e = unset_itr.next();

    TEST_EQ(static_cast<usize>(0), a);
    TEST_EQ(static_cast<usize>(34), b);
    TEST_EQ(static_cast<usize>(57), c);
    TEST_EQ(static_cast<usize>(134), d);
    TEST_EQ(static_cast<usize>(134), e);
  }

  TEST_EQ(false, arr.test_all());
  arr.set(0);
  arr.set(34);
  arr.set(57);
  TEST_EQ(true, arr.test_all());
  TEST_EQ(static_cast<usize>(133), arr.highest_set);
  arr.clear();
  TEST_EQ(false, arr.test_all());
  TEST_EQ(static_cast<usize>(0), arr.highest_set);
  TEST_EQ(static_cast<usize>(134), arr.length);
}

TEST_FUNCTION(SET_BIT_ARRAY, intersects) {

  SetBitArray arr = SetBitArray(134);
  TEST_EQ(static_cast<usize>(134), arr.length);
  TEST_EQ(static_cast<usize>(0), arr.highest_set);

  arr.set(57);
  TEST_EQ(true, arr.test(57));
  TEST_EQ(static_cast<usize>(57), arr.highest_set);

  {
    for (usize i = 0; i < 57; ++i) {
      TEST_EQ(false, arr.test(i));
    }

    for (usize i = 58; i < 134; ++i) {
      TEST_EQ(false, arr.test(i));
    }
  }

  SetBitArray arr2 = SetBitArray(134);
  TEST_EQ(static_cast<usize>(134), arr2.length);
  TEST_EQ(static_cast<usize>(0), arr2.highest_set);

  arr2.set(12);

  TEST_EQ(true, arr2.test(12));
  TEST_EQ(static_cast<usize>(12), arr2.highest_set);

  {
    for (usize i = 0; i < 12; ++i) {
      TEST_EQ(false, arr2.test(i));
    }

    for (usize i = 13; i < 134; ++i) {
      TEST_EQ(false, arr2.test(i));
    }
  }

  TEST_EQ(false, arr2.intersects(arr));

  arr2.set(57);
  TEST_EQ(true, arr2.test(12));
  TEST_EQ(true, arr2.test(57));
  TEST_EQ(static_cast<usize>(57), arr2.highest_set);

  {
    for (usize i = 0; i < 12; ++i) {
      TEST_EQ(false, arr.test(i));
    }

    for (usize i = 13; i < 57; ++i) {
      TEST_EQ(false, arr.test(i));
    }

    for (usize i = 58; i < 134; ++i) {
      TEST_EQ(false, arr.test(i));
    }
  }

  TEST_EQ(true, arr2.intersects(arr));
}

TEST_FUNCTION(CONST_BIT_ARRAY, insert) {
  ConstBitArray<3> arr = {};

  arr.set(1);
  {
    TEST_EQ(true, arr.test(1));

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }

  arr.set(2);
  {
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test_all());
  }

  arr.set(0);
  {
    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));

    TEST_EQ(true, arr.test_all());
  }

  {
    usize set = arr.count_set();
    usize unset = arr.count_unset();

    TEST_EQ(static_cast<usize>(3), set);
    TEST_EQ(static_cast<usize>(0), unset);

    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));
    TEST_EQ(true, arr.test_all());
  }

  {
    auto unset_itr = arr.unset_itr();

    usize a = unset_itr.next();
    usize b = unset_itr.next();

    TEST_EQ(static_cast<usize>(3), a);
    TEST_EQ(static_cast<usize>(3), b);

    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));
    TEST_EQ(true, arr.test_all());
  }

  arr.unset(1);
  {
    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(2));

    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test_all());
  }

  arr.unset(0);
  {
    TEST_EQ(true, arr.test(2));

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test_all());
  }

  arr.unset(2);
  {
    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }
  
  arr.set(0);
  arr.set(1);
  arr.set(2);
  {
    TEST_EQ(true, arr.test(0));
    TEST_EQ(true, arr.test(1));
    TEST_EQ(true, arr.test(2));

    TEST_EQ(true, arr.test_all());
  }
  
  arr.clear();
  {
    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }

  {
    usize set = arr.count_set();
    usize unset = arr.count_unset();

    TEST_EQ(static_cast<usize>(0), set);
    TEST_EQ(static_cast<usize>(3), unset);

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }

  {
    auto unset_itr = arr.unset_itr();

    usize a = unset_itr.next();
    usize b = unset_itr.next();
    usize c = unset_itr.next();
    usize d = unset_itr.next();
    usize e = unset_itr.next();

    TEST_EQ(static_cast<usize>(0), a);
    TEST_EQ(static_cast<usize>(1), b);
    TEST_EQ(static_cast<usize>(2), c);
    TEST_EQ(static_cast<usize>(3), d);
    TEST_EQ(static_cast<usize>(3), e);

    TEST_EQ(false, arr.test(0));
    TEST_EQ(false, arr.test(1));
    TEST_EQ(false, arr.test(2));
    TEST_EQ(false, arr.test_all());
  }
}

TEST_FUNCTION(CONST_BIT_ARRAY, big_insert) {

  ConstBitArray<134> arr = {};

  arr.set(57);
  TEST_EQ(true, arr.test(57));

  {
    for (usize i = 0; i < 57; ++i) {
      TEST_EQ(false, arr.test(i));
    }

    for (usize i = 58; i < 134; ++i) {
      TEST_EQ(false, arr.test(i));
    }
  }

  TEST_EQ(false, arr.test_all()); 
  arr.clear();

  for (usize i = 0; i < 134; ++i) {
    TEST_EQ(false, arr.test(i));
  }

  {
    for (usize i = 1; i < 34; ++i) {
      arr.set(i);
    }

    for (usize i = 35; i < 57; ++i) {
      arr.set(i);
    }

    for (usize i = 58; i < 134; ++i) { 
      arr.set(i);
    }
  }

  {
    usize set = arr.count_set();
    usize unset = arr.count_unset();

    TEST_EQ(static_cast<usize>(131), set);
    TEST_EQ(static_cast<usize>(3), unset);
  }

  {
    auto unset_itr = arr.unset_itr();

    usize a = unset_itr.next();
    usize b = unset_itr.next();
    usize c = unset_itr.next();
    usize d = unset_itr.next();
    usize e = unset_itr.next();

    TEST_EQ(static_cast<usize>(0), a);
    TEST_EQ(static_cast<usize>(34), b);
    TEST_EQ(static_cast<usize>(57), c);
    TEST_EQ(static_cast<usize>(134), d);
    TEST_EQ(static_cast<usize>(134), e);
  }

  TEST_EQ(false, arr.test_all());
  arr.set(0);
  arr.set(34);
  arr.set(57);
  TEST_EQ(true, arr.test_all());
  arr.clear();
  TEST_EQ(false, arr.test_all());
}


