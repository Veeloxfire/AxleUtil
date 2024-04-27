#include <AxleTest/unit_tests.h>
#include <AxleUtil/io.h>

Axle::Array<AxleTest::UnitTest> &AxleTest::unit_tests_ref() {
  static Axle::Array<AxleTest::UnitTest> t = {};
  return t;
}

AxleTest::_testAdder::_testAdder(const Axle::ViewArr<const char> &test_name,
                                   AxleTest::TEST_FN fn) {
  unit_tests_ref().insert({test_name, fn});
}

#ifdef AXLE_TEST_SANITY
namespace FAIL_TESTS {
  static void report(AxleTest::TestErrors *test_errors) {
    test_errors->report_error("Should have errored");
  }

  static void test_eq(AxleTest::TestErrors *test_errors) { TEST_EQ(1, 2); }

  static void test_neq(AxleTest::TestErrors *test_errors) { TEST_NEQ(1, 1); }

  static void test_eq_ptr(AxleTest::TestErrors *test_errors) {
    int i = 0;
    int j = 0;
    TEST_EQ(&i, &j);
  }

  static void test_neq_ptr(AxleTest::TestErrors *test_errors) {
    int i = 0;
    TEST_NEQ(&i, &i);
  }

  static void test_arr_eq_size(AxleTest::TestErrors *test_errors) {
    int arr1[] = {1, 2, 3};
    int arr2[] = {1, 2};

    TEST_ARR_EQ(arr1, Axle::array_size(arr1), arr2, Axle::array_size(arr2));
  }

  static void test_arr_eq_values(AxleTest::TestErrors *test_errors) {
    int arr1[] = {1, 2, 3};
    int arr2[] = {1, 2, 4};

    TEST_ARR_EQ(arr1, Axle::array_size(arr1), arr2, Axle::array_size(arr2));
  }

  static void test_str_eq_view(AxleTest::TestErrors *test_errors) {
    TEST_STR_EQ(Axle::lit_view_arr("HELLO"), Axle::lit_view_arr("hello"));
  }

  static void test_str_eq_owned(AxleTest::TestErrors *test_errors) {
    Axle::OwnedArr arr1 = Axle::copy_arr("HELLO", 6);
    Axle::OwnedArr arr2 = Axle::copy_arr("hello", 6);
    TEST_STR_EQ(arr1, arr2);
  }
}

template<void(*func)(AxleTest::TestErrors*)>
void fail_test(AxleTest::TestErrors* errors) {
  func(errors);

  if (errors->is_panic()) {
    errors->first_error = Axle::OwnedArr<const char>{};
  }
  else {
    errors->first_error = Axle::copy_arr("Fail test didn't fail");
  }
}

const AxleTest::_testAdder fail_tests[] = {
  {Axle::lit_view_arr("FAIL_TESTS::Report Error"), fail_test<FAIL_TESTS::report>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_EQ"), fail_test<FAIL_TESTS::test_eq>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_NEQ"), fail_test<FAIL_TESTS::test_neq>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_EQ pointer"), fail_test<FAIL_TESTS::test_eq_ptr>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_NEQ pointer"),
   fail_test<FAIL_TESTS::test_neq_ptr>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_ARR_EQ size"),
   fail_test<FAIL_TESTS::test_arr_eq_size>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_ARR_EQ values"),
   fail_test<FAIL_TESTS::test_arr_eq_values>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_STR_EQ ViewArr"),
   fail_test<FAIL_TESTS::test_str_eq_view>},
  {Axle::lit_view_arr("FAIL_TESTS::TEST_STR_EQ Axle::OwnedArr"),
   fail_test<FAIL_TESTS::test_str_eq_owned>},
};
#endif
