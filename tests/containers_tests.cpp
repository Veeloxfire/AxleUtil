#include <AxleUtil/utility.h>

#include <AxleTest/unit_tests.h>
using namespace Axle;

static constexpr int RANDOM_ARR[] = {
  65, 55, 71, 23, 92,
  57, 51, 84, 86, 30,
  1, 37, 100, 97, 86,
  14, 98, 38, 88, 45,
};

static constexpr int SORTED_RANDOM_ARR[] = {
  1, 14, 23, 30, 37,
  38, 45, 51, 55, 57,
  65, 71, 84, 86, 86,
  88, 92, 97, 98, 100
};

static constexpr usize RANDOM_ARR_SIZE = array_size(RANDOM_ARR);

TEST_FUNCTION(Util_ArraySize, array_sizes) {

  static_assert(ArraySize<int[3]>::SIZE == 3);

  {
    constexpr int arr[3] { 0,0,0 };
    static_assert(Axle::array_size(arr) == 3);
  }

  {
    int arr[3];
    TEST_EQ(static_cast<usize>(3), Axle::array_size(arr));
  }

  static_assert(ArraySize<ConstArray<int, 5>>::SIZE == 5);

  {
    constexpr ConstArray<int, 5> arr { {0,0,0,0,0} };
    static_assert(Axle::array_size(arr) == 5);
  }

  {
    ConstArray<int, 5> arr;
    TEST_EQ(static_cast<usize>(5), Axle::array_size(arr));
    TEST_EQ(static_cast<usize>(5), Axle::array_size(arr.data));
  }
}

TEST_FUNCTION(Util_ConstArray, fill) {
  Axle::ConstArray arr = fill_array<int, 4>(12);
  
  TEST_EQ(static_cast<usize>(4), Axle::array_size(arr));
  TEST_EQ(12, arr[0]);
  TEST_EQ(12, arr[1]);
  TEST_EQ(12, arr[2]);
  TEST_EQ(12, arr[3]);
}

TEST_FUNCTION(Util_Array, Default) {
  Array<int> a;
  TEST_EQ((usize)0, a.size);
  TEST_EQ((usize)0, a.capacity);
  TEST_EQ((int*)nullptr, a.data);
}

TEST_FUNCTION(Util_Array, Insert_Remove) {
  Array<usize> a = {};

  for (usize i = 0; i < 1000; i++) {
    a.insert(i ^ (i + 1));
  }

  TEST_EQ((usize)1000, a.size);
  TEST_EQ((usize)(999 ^ 1000), *a.back());

  for (usize i = 0; i < 1000; i++) {
    TEST_EQ((i ^ (i + 1)), a.data[i]);
  }

  {
    auto b = a.begin();
    const auto end = a.end();
    for (usize i = 0; i < 1000; i++) {
      TEST_EQ((i ^ (i + 1)), *b);
      b++;
    }

    TEST_EQ(b, end);

    b = a.mut_begin();

    for (usize i = 0; i < 1000; i++) {
      TEST_EQ((i ^ (i + 1)), *b);
      b++;
    }

    TEST_EQ(end, b);
  }


  a.remove_at(500);

  TEST_EQ((usize)999, a.size);
  TEST_EQ((usize)(999 ^ 1000), a.data[998]);
  TEST_EQ((usize)(499 ^ 500), a.data[499]);
  TEST_EQ((usize)(501 ^ 502), a.data[500]);

  a.insert_at(500, (500 ^ 501));

  for (usize i = 0; i < 1000; i++) {
    TEST_EQ((i ^ (i + 1)), a.data[i]);
  }

  a.insert_at(0, 0);
  TEST_EQ((usize)0, a.data[0]);
  a.remove_at(0);
  a.insert_at(1000, 0);
  TEST_EQ((usize)0, a.data[1000]);
  a.remove_at(1000);

  for (usize i = 0; i < 1000; i++) {
    TEST_EQ((i ^ (i + 1)), a.data[i]);
  }

  for (usize i = 0; i < 1000; i++) {
    a.pop();
  }

  TEST_EQ((usize)0, a.size);
}

TEST_FUNCTION(Util_ArrayMax, Default) {
  ArrayMax<int> a;

  TEST_EQ((usize)0, a.size);
  TEST_EQ((usize)0, a.capacity);
  TEST_EQ((int*)nullptr, a.data);
}

TEST_FUNCTION(Util_ArrayMax, Insert_Remove) {
  ArrayMax<usize> a = new_arr_max<usize>(1001);

  for (usize i = 0; i < 1000; i++) {
    a.insert(i ^ (i + 1));
  }

  TEST_EQ((usize)1000, a.size);
  TEST_EQ((usize)(999 ^ 1000), *a.back());

  for (usize i = 0; i < 1000; i++) {
    TEST_EQ((i ^ (i + 1)), a.data[i]);
  }

  {
    auto b = a.begin();
    const auto end = a.end();
    for (usize i = 0; i < 1000; i++) {
      TEST_EQ((i ^ (i + 1)), *b);
      b++;
    }

    TEST_EQ(b, end);

    b = a.mut_begin();

    for (usize i = 0; i < 1000; i++) {
      TEST_EQ((i ^ (i + 1)), *b);
      b++;
    }

    TEST_EQ(end, b);
  }


  a.remove_at(500);

  TEST_EQ((usize)999, a.size);
  TEST_EQ((usize)(999 ^ 1000), a.data[998]);
  TEST_EQ((usize)(499 ^ 500), a.data[499]);
  TEST_EQ((usize)(501 ^ 502), a.data[500]);

  a.insert_at(500, (500 ^ 501));

  for (usize i = 0; i < 1000; i++) {
    TEST_EQ((i ^ (i + 1)), a.data[i]);
  }

  a.insert_at(0, 0);
  TEST_EQ((usize)0, a.data[0]);
  a.remove_at(0);
  a.insert_at(1000, 0);
  TEST_EQ((usize)0, a.data[1000]);
  a.remove_at(1000);

  for (usize i = 0; i < 1000; i++) {
    TEST_EQ((i ^ (i + 1)), a.data[i]);
  }

  for (usize i = 0; i < 1000; i++) {
    a.pop();
  }

  TEST_EQ((usize)0, a.size);
}


TEST_FUNCTION(Util_Queue, Insert_Remove) {
  Queue<usize> a = {};

  for (usize i = 0; i < 100; i++) {
    a.push_back(i ^ (i + 1));
  }

  TEST_EQ((usize)100, a.size);

  for (usize i = 0; i < 100; i++) {
    TEST_EQ((i ^ (i + 1)), a.pop_front());
  }

  for (usize i = 0; i < 100; i++) {
    a.push_front(i ^ (i + 1));
  }

  TEST_EQ((usize)100, a.size);

  TEST_EQ((usize)(0 ^ 1), a.pop_back());

  for (usize i = 1; i < 100; i++) {
    TEST_EQ((i ^ (i + 1)), a.pop_back());
  }
}

static constexpr auto sort_fn = []<typename T>(const T& l, const T& r) {
  ASSERT(&l != &r);
  return l <=> r;
};

TEST_FUNCTION(Util, sort) {
  {
    int small_random_arr[] = {
      65, 55, 71, 23, 92, 10
    };

    constexpr int SMALL_SORTED_RANDOM_ARR[] = {
      10, 23, 55, 65, 71, 92
    };

    sort_view(view_arr(small_random_arr), sort_fn);

    TEST_ARR_EQ(SMALL_SORTED_RANDOM_ARR, 6, small_random_arr, 6);
  }

  {
    int small_sorted_arr[] = {
      10, 23, 55, 65, 71, 92
    };

    constexpr int SMALL_SORTED_RANDOM_ARR[] = {
      10, 23, 55, 65, 71, 92
    };

    sort_view(view_arr(small_sorted_arr), sort_fn);

    TEST_ARR_EQ(SMALL_SORTED_RANDOM_ARR, 6, small_sorted_arr, 6);
  }
  {
    int small_eq_arr[] = {
      10, 10, 10, 10, 10, 10, 10,
    };

    constexpr int SMALL_SORTED_RANDOM_ARR[] = {
      10, 10, 10, 10, 10, 10, 10,
    };

    sort_view(view_arr(small_eq_arr), sort_fn);

    TEST_ARR_EQ(SMALL_SORTED_RANDOM_ARR, 6, small_eq_arr, 6);
  }
  {
    Array<int> ints = {};

    for (int i : RANDOM_ARR) {
      ints.insert(i);
    }

    sort_view(view_arr(ints), sort_fn);

    TEST_ARR_EQ(SORTED_RANDOM_ARR, RANDOM_ARR_SIZE, ints.data, ints.size);
  }
}

TEST_FUNCTION(Util_OwnedArr, Default) {
  {
    OwnedArr<int> a;

    TEST_EQ((usize)0, a.size);
    TEST_EQ((int*)nullptr, a.data);
  }
  {
    OwnedArr<const int> a;

    TEST_EQ((usize)0, a.size);
    TEST_EQ((const int*)nullptr, a.data);
  }
}

TEST_FUNCTION(Util_OwnedArr, bake) {
  Array<int> ints = {};

  for (int i : RANDOM_ARR) {
    ints.insert(i);
  }

  OwnedArr<int> ints_arr = bake_arr(std::move(ints));

  static_assert(IS_SAME_TYPE<decltype(ints_arr[0]), int&>, "Must return ref");

  TEST_EQ((int*)nullptr, ints.data);
  TEST_EQ((usize)0, ints.size);
  TEST_EQ((usize)0, ints.capacity);
  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, ints_arr.data, ints_arr.size);

  const OwnedArr<int> ints_arr2 = std::move(ints_arr);

  static_assert(IS_SAME_TYPE<decltype(ints_arr2[0]), int&>, "Must return ref");

  TEST_EQ((int*)nullptr, ints_arr.data);
  TEST_EQ((usize)0, ints_arr.size);
  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, ints_arr2.data, ints_arr2.size); 
}

TEST_FUNCTION(Util_OwnedArr, copy_bake) {
  Array<int> ints = {};

  for (int i : RANDOM_ARR) {
    ints.insert(i);
  }

  OwnedArr<int> ints_arr = copy_bake_arr(ints);

  static_assert(IS_SAME_TYPE<decltype(ints_arr[0]), int&>, "Must return ref");

  TEST_NEQ((int*)nullptr, ints.data);
  TEST_NEQ((usize)0, ints.size);
  TEST_NEQ((usize)0, ints.capacity);
  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, ints_arr.data, ints_arr.size);
  TEST_ARR_EQ(ints.data, ints.size, ints_arr.data, ints_arr.size);
}

TEST_FUNCTION(Util_OwnedArr, bake_const) {
  Array<int> ints = {};

  for (int i : RANDOM_ARR) {
    ints.insert(i);
  }
  const usize prev_size = ints.size;
  OwnedArr<const int> ints_arr = bake_const_arr(std::move(ints));

  static_assert(IS_SAME_TYPE<decltype(ints_arr[0]), const int&>, "Must return const ref");

  TEST_EQ(prev_size, ints_arr.size);
  TEST_EQ((int*)nullptr, ints.data);
  TEST_EQ((usize)0, ints.size);
  TEST_EQ((usize)0, ints.capacity);
  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, ints_arr.data, ints_arr.size);

  const OwnedArr<const int> ints_arr2 = std::move(ints_arr);

  static_assert(IS_SAME_TYPE<decltype(ints_arr2[0]), const int&>, "Must return const ref");

  TEST_EQ((const int*)nullptr, ints_arr.data);
  TEST_EQ((usize)0, ints_arr.size);
  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, ints_arr2.data, ints_arr2.size);
}

TEST_FUNCTION(Util_OwnedArr, copy) {
  OwnedArr<int> arr = copy_arr(RANDOM_ARR);

  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, arr.data, arr.size);

  OwnedArr<int> arr2 = copy_arr(arr);

  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, arr2.data, arr2.size);

  OwnedArr<int> arr3 = copy_arr(RANDOM_ARR, RANDOM_ARR_SIZE);

  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, arr3.data, arr3.size);

  OwnedArr<int> arr4 = copy_arr(view_arr(RANDOM_ARR));

  TEST_ARR_EQ(RANDOM_ARR, RANDOM_ARR_SIZE, arr4.data, arr4.size);
}

namespace {
  struct CheckDelete {
    int* i;

    constexpr CheckDelete() = default;
    constexpr CheckDelete(int* o) : i(o) {}
    constexpr CheckDelete(CheckDelete&& o) noexcept : i(std::exchange(o.i, nullptr)) {}
    constexpr CheckDelete(const CheckDelete& o) noexcept : i(o.i) {}

    constexpr CheckDelete& operator=(CheckDelete&& o) noexcept {
      if(&o == this) return *this;
      i = std::exchange(o.i, nullptr);
      return *this;
    }

    constexpr CheckDelete& operator=(const CheckDelete& o) noexcept {
      i = o.i;
      return *this;
    }

    constexpr ~CheckDelete() {
      if (i != nullptr) {
        *i += 1;
      }
    }
  };
}

TEST_FUNCTION(Util_OwnedArr, destruction) {
  {
    int i = 0;
    constexpr int COUNTER = 20;
    {
      OwnedArr<CheckDelete> owned;
      {
        Array<CheckDelete> deleter = {};
        for (int counter = 0; counter < COUNTER; counter += 1) {
          deleter.insert(CheckDelete{ &i });
        }

        owned = bake_arr(std::move(deleter));
      }

      TEST_EQ(0, i);

      {
        Array<CheckDelete> deleter = {};
        for (int counter = 0; counter < 20; counter += 1) {
          deleter.insert(CheckDelete{ &i });
        }

        owned = bake_arr(std::move(deleter));
      }

      TEST_EQ(COUNTER, i);
    }

    TEST_EQ(COUNTER * 2, i);
  }

  {
    int i = 0;
    constexpr int COUNTER = 20;
    {
      OwnedArr<const CheckDelete> owned;
      {
        Array<CheckDelete> deleter = {};
        for (int counter = 0; counter < COUNTER; counter += 1) {
          deleter.insert(CheckDelete{ &i });
        }

        owned = bake_arr(std::move(deleter));
      }

      TEST_EQ(0, i);

      {
        Array<CheckDelete> deleter = {};
        for (int counter = 0; counter < 20; counter += 1) {
          deleter.insert(CheckDelete{ &i });
        }

        owned = bake_arr(std::move(deleter));
      }

      TEST_EQ(COUNTER, i);
    }

    TEST_EQ(COUNTER * 2, i);
  }
}

TEST_FUNCTION(Util_ViewArr, Default) {
  {
    ViewArr<int> a;

    TEST_EQ((usize)0, a.size);
    TEST_EQ((int*)nullptr, a.data);
  }
  {
    ViewArr<const int> a;

    TEST_EQ((usize)0, a.size);
    TEST_EQ((const int*)nullptr, a.data);
  }
}

TEST_FUNCTION(Util_ViewArr, view_Array) {
  Array<int> arr = {};
  arr.reserve_extra(RANDOM_ARR_SIZE);

  for (int i : RANDOM_ARR) {
    arr.insert(i);
  }

  {
    ViewArr<int> v = view_arr(arr);

    TEST_EQ(arr.data, v.data);
    TEST_EQ(arr.size, v.size);

    ViewArr<const int> v2 = v;
    TEST_EQ((const int*)arr.data, v2.data);
    TEST_EQ(arr.size, v2.size);
  }

  {
    ViewArr<int> v = view_arr(arr, 3, 5);

    TEST_EQ(arr.data + 3, v.data);
    TEST_EQ((usize)5, v.size);

    ViewArr<const int> v2 = v;
    TEST_EQ((const int*)arr.data + 3, v2.data);
    TEST_EQ((usize)5, v2.size);
  }

  {
    ViewArr<const int> v = const_view_arr(arr);

    TEST_EQ((const int*)arr.data, v.data);
    TEST_EQ(arr.size, v.size);
  }

  {
    ViewArr<const int> v = const_view_arr(arr, 3, 5);

    TEST_EQ((const int*)arr.data + 3, v.data);
    TEST_EQ((usize)5, v.size);
  }
}

TEST_FUNCTION(Util_ViewArr, c_array_views) {
  {
    ViewArr<const char> view = view_arr("hello world");

    const char expected[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'};

    TEST_ARR_EQ(expected, array_size(expected), view.data, view.size);
  }

  {
    ViewArr<const char> view = view_arr("hello world", 1, 9);

    const char expected[] = { 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', };

    TEST_ARR_EQ(expected, array_size(expected), view.data, view.size);
  }

  {
    ViewArr<const char> view = lit_view_arr("hello world");

    const char expected[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', };

    TEST_ARR_EQ(expected, array_size(expected), view.data, view.size);
  }
}

TEST_FUNCTION(Util_ViewArr, rvalues) {
  //Should be able to view from an r-value

  const u8 bytes[4] {1, 2, 3, 4};
  const ViewArr<const u8> full_nc = view_arr(ViewArr<const u8>{bytes, 4});
  TEST_ARR_EQ(bytes, array_size(bytes), full_nc.data, full_nc.size);

  const ViewArr<const u8> middle_nc = view_arr(ViewArr<const u8>{bytes, 4}, 1, 2);
  TEST_ARR_EQ(bytes + 1, static_cast<usize>(2), middle_nc.data, middle_nc.size);

  const ViewArr<const u8> full_c = const_view_arr(ViewArr<const u8>{bytes, 4});
  TEST_ARR_EQ(bytes, array_size(bytes), full_c.data, full_c.size);

  const ViewArr<const u8> middle_c = const_view_arr(ViewArr<const u8>{bytes, 4}, 1, 2);
  TEST_ARR_EQ(bytes + 1, static_cast<usize>(2), middle_c.data, middle_c.size);
}

TEST_FUNCTION(Util_ViewArr, memcpy_ViewArray) {
  char dest[10];
  const char src[10]= {'h','e','l','l','o','w','o','r','l','d'};

  memcpy_ts(view_arr(dest), view_arr(src));

  TEST_ARR_EQ(src, array_size(src), dest, array_size(dest));
}

TEST_FUNCTION(Util_ViewArr, memeq_ViewArray) {
  char dest[10] = {'w','o','r','l','d','h','e','l','l','o'};
  const char src[10]= {'h','e','l','l','o','w','o','r','l','d'};

  TEST_EQ(false, memeq_ts(view_arr(dest), view_arr(src)));
  TEST_EQ(true, memeq_ts(view_arr(dest, 5, 5), view_arr(src, 0, 5)));
  TEST_EQ(true, memeq_ts(view_arr(dest, 0, 5), view_arr(src, 5, 5)));

  TEST_EQ(true, memeq_ts(view_arr(dest), view_arr(dest)));
  TEST_EQ(true, memeq_ts(view_arr(dest), const_view_arr(dest)));
  TEST_EQ(true, memeq_ts(const_view_arr(dest), view_arr(dest)));
  TEST_EQ(true, memeq_ts(const_view_arr(dest), const_view_arr(dest)));

  TEST_EQ(false, memeq_ts(view_arr(dest), view_arr(dest, 1, 1)));
  TEST_EQ(false, memeq_ts(view_arr(dest), view_arr(dest, 0, 1)));

  {
    int arr_mut[1] = {1};
    const int arr_const[1] = {1};

    TEST_EQ(true, memeq_ts<int>(view_arr(arr_mut), view_arr(arr_const)));
    TEST_EQ(true, memeq_ts<int>(view_arr(arr_const), view_arr(arr_mut)));
    TEST_EQ(true, memeq_ts<int>(view_arr(arr_mut), view_arr(arr_mut)));
    TEST_EQ(true, memeq_ts<int>(view_arr(arr_const), view_arr(arr_const)));
  }
}

TEST_FUNCTION(BucketArray, basic) {
  BucketArray<int> arr = {};

  for(int i: RANDOM_ARR) {
    arr.insert(i);
  }

  {
    usize index = 0;
    FOR(arr, it) {
      TEST_EQ(*it, RANDOM_ARR[index]);
      index += 1;
    }
  }

  {
    usize index = 0;
    FOR_MUT(arr, it) {
      TEST_EQ(*it, RANDOM_ARR[index]);
      index += 1;
    }
  }

  const int* first = arr.begin().get();

  for(usize i = 0; i < BucketArray<int>::BLOCK::BLOCK_SIZE * 2; ++i) {
    arr.insert(static_cast<int>(i));
  }

  const int* first2 = arr.begin().get();
  TEST_EQ(first, first2);//should be stable iterators
}

TEST_FUNCTION(Containers, Self_move_assign) {
  {
    int i = 0;
    Array<CheckDelete> arr = {};

    arr.insert({&i});

    arr = static_cast<Array<CheckDelete>&&>(arr);

    TEST_EQ(0, i);

    for(usize itr = 0; itr < 200; ++itr) {
      arr.insert(arr[0]);
    }
    TEST_EQ(0, i);
  }

  {
    int i = 0;
    CheckDelete cd = {&i};

    OwnedArr<CheckDelete> arr = copy_arr(&cd, 1);

    arr = static_cast<OwnedArr<CheckDelete>&&>(arr);

    TEST_EQ(0, i);
  }

  {
    int i = 0;
    CheckDelete cd = {&i};

    OwnedArr<const CheckDelete> arr = copy_arr(&cd, 1);

    arr = static_cast<OwnedArr<const CheckDelete>&&>(arr);

    TEST_EQ(0, i);
  }

  {
    int i = 0;
    Queue<CheckDelete> arr = {};

    arr.push_back({&i});
    arr = static_cast<Queue<CheckDelete>&&>(arr);

    TEST_EQ(0, i);

    for(usize itr = 0; itr < 200; ++itr) {
      arr.push_back(arr.holder[arr.start]);
    }
    
    TEST_EQ(0, i);
  }
}
