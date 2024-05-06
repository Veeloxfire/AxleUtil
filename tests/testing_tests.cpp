#include <AxleTest/unit_tests.h>
#include "test_contexts.h"

TEST_FUNCTION(AxleTest, always_pass) {
  (void)test_errors;
}

TEST_FUNCTION(AxleTest, always_fail) {
  (void)test_errors;
  Axle::Panic::panic("Expected to fail");
}

TEST_FUNCTION_CTX(AxleTest, recieves_context, TestContexts::Integer) {
  TEST_EQ(static_cast<u32>(0x1234), context->i);
}

TEST_FUNCTION(AxleTest, loop) {
  volatile unsigned int i = 0;
  while(true) { i += 1; }

  test_errors->report_error("Should have infinite looped");
}

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
struct FailTestAdder {
  static void fail_test(AxleTest::TestErrors* errors, const AxleTest::IPC::OpaqueContext&) {
    func(errors);

    if (errors->is_panic()) {
      errors->first_error = Axle::OwnedArr<const char>{};
    }
    else {
      errors->first_error = Axle::copy_arr("Fail test didn't fail");
    }
  }

  FailTestAdder(const Axle::ViewArr<const char> &test_name) {
    AxleTest::unit_tests_ref().insert(AxleTest::UnitTest{test_name, {}, fail_test});
  }
};

// Add all the tests
const FailTestAdder<FAIL_TESTS::report> fail_test_0 {Axle::lit_view_arr("FAIL_TESTS::Report Error")}; 
const FailTestAdder<FAIL_TESTS::test_eq> fail_test_1 {Axle::lit_view_arr("FAIL_TESTS::TEST_EQ")};
const FailTestAdder<FAIL_TESTS::test_neq> fail_test_2 {Axle::lit_view_arr("FAIL_TESTS::TEST_NEQ")};
const FailTestAdder<FAIL_TESTS::test_eq_ptr> fail_test_3 {Axle::lit_view_arr("FAIL_TESTS::TEST_EQ pointer")};
const FailTestAdder<FAIL_TESTS::test_neq_ptr> fail_test_4 {Axle::lit_view_arr("FAIL_TESTS::TEST_NEQ pointer")};
const FailTestAdder<FAIL_TESTS::test_arr_eq_size> fail_test_5 {Axle::lit_view_arr("FAIL_TESTS::TEST_ARR_EQ size")};
const FailTestAdder<FAIL_TESTS::test_arr_eq_values> fail_test_6 {Axle::lit_view_arr("FAIL_TESTS::TEST_ARR_EQ values")};
const FailTestAdder<FAIL_TESTS::test_str_eq_view> fail_test_7 {Axle::lit_view_arr("FAIL_TESTS::TEST_STR_EQ ViewArr")};
const FailTestAdder<FAIL_TESTS::test_str_eq_owned> fail_test_8 {Axle::lit_view_arr("FAIL_TESTS::TEST_STR_EQ Axle::OwnedArr")};

