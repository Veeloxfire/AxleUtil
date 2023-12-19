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
#endif

void AxleTest::TestContext::run_test(const UnitTest& t) {
  Axle::IO::format("Starting test: {}", t.test_name);

  AxleTest::TestErrors errors = {};
  errors.test_name = t.test_name;

  try {
    t.test_func(&errors);
  }
  catch (const std::exception &e) {
    const char *message = e.what();
    const Axle::ViewArr<const char> message_view = {message, Axle::strlen_ts(message)};
    errors.report_error("Test failed with exception: {}", message_view);
  }
  catch (...) {
    errors.report_error("Test failed with unknown thrown type...");
  }

  if (errors.is_panic()) {
    Axle::IO::print("\t - failed\n");
    failed_tests.insert(std::move(errors));
  } 
  else {
    Axle::IO::print("\t - passed\n");
  }
}

void AxleTest::print_failed_tests(const AxleTest::TestContext& context) {
  if (context.failed_tests.size == 0) {
    Axle::IO::print("\nAll tests succeeded!");
  } else {
    Axle::IO::err_print("\nSome tests failed!\n");

    for (const auto &t : context.failed_tests) {
      Axle::OwnedArr ts = Axle::format_type_set(Axle::view_arr(t.first_error), 2, 80);

      Axle::IO::err_format("\n===========\n{} failed with errors:\n{}\n", 
                     t.test_name, ts);
    }
  }
}

void AxleTest::test_main() {
  #ifdef AXLE_TEST_SANITY
  {
    //Test that the tests fail correctly

    usize failed_count = 0;

    const AxleTest::UnitTest fail_tests[] = {
        {Axle::lit_view_arr("FAIL_TESTS::Report Error"), FAIL_TESTS::report},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_EQ"), FAIL_TESTS::test_eq},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_NEQ"), FAIL_TESTS::test_neq},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_EQ pointer"), FAIL_TESTS::test_eq_ptr},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_NEQ pointer"),
         FAIL_TESTS::test_neq_ptr},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_ARR_EQ size"),
         FAIL_TESTS::test_arr_eq_size},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_ARR_EQ values"),
         FAIL_TESTS::test_arr_eq_values},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_STR_EQ ViewArr"),
         FAIL_TESTS::test_str_eq_view},
        {Axle::lit_view_arr("FAIL_TESTS::TEST_STR_EQ Axle::OwnedArr"),
         FAIL_TESTS::test_str_eq_owned},
    };
    for (const auto &fail_test : fail_tests) {
      // Test the tester
      AxleTest::TestErrors errors = {};
      errors.test_name = fail_test.test_name;

      Axle::IO::format("Starting fail test: {}", fail_test.test_name);

      if (errors.is_panic()) {
        Axle::IO::format("\t - Failed Before entering the test!\n");
        failed_count += 1;
        continue;
      }

      fail_test.test_func(&errors);

      if (!errors.is_panic()) {
        Axle::IO::format("\t - Failed\n");
        failed_count += 1;
      } else {
        Axle::IO::format("\t - Passed\n");
      }
    }

    if (failed_count > 0) {
      Axle::IO::format("{} sanity tests failed. Stopping ...\n", failed_count);
      return;
    }
  }
  #endif

  AxleTest::TestContext context = {};

  for (const auto &t : AxleTest::unit_tests_ref()) {
    context.run_test(t);
  }

  AxleTest::print_failed_tests(context);
}
