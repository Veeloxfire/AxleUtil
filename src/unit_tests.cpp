#include <AxleTest/unit_tests.h>
#include <AxleUtil/io.h>

Array<UNIT_TESTS::UnitTest> &UNIT_TESTS::unit_tests_ref() {
  static Array<UNIT_TESTS::UnitTest> t = {};
  return t;
}

UNIT_TESTS::_testAdder::_testAdder(const ViewArr<const char> &test_name,
                                   UNIT_TESTS::TEST_FN fn) {
  unit_tests_ref().insert({test_name, fn});
}

#ifdef AXLE_TEST_SANITY
namespace FAIL_TESTS {
  static void report(UNIT_TESTS::TestErrors *test_errors) {
    test_errors->report_error("Should have errored");
  }

  static void test_eq(UNIT_TESTS::TestErrors *test_errors) { TEST_EQ(1, 2); }

  static void test_neq(UNIT_TESTS::TestErrors *test_errors) { TEST_NEQ(1, 1); }

  static void test_eq_ptr(UNIT_TESTS::TestErrors *test_errors) {
    int i = 0;
    int j = 0;
    TEST_EQ(&i, &j);
  }

  static void test_neq_ptr(UNIT_TESTS::TestErrors *test_errors) {
    int i = 0;
    TEST_NEQ(&i, &i);
  }

  static void test_arr_eq_size(UNIT_TESTS::TestErrors *test_errors) {
    int arr1[] = {1, 2, 3};
    int arr2[] = {1, 2};

    TEST_ARR_EQ(arr1, array_size(arr1), arr2, array_size(arr2));
  }

  static void test_arr_eq_values(UNIT_TESTS::TestErrors *test_errors) {
    int arr1[] = {1, 2, 3};
    int arr2[] = {1, 2, 4};

    TEST_ARR_EQ(arr1, array_size(arr1), arr2, array_size(arr2));
  }

  static void test_str_eq_view(UNIT_TESTS::TestErrors *test_errors) {
    TEST_STR_EQ(lit_view_arr("HELLO"), lit_view_arr("hello"));
  }

  static void test_str_eq_owned(UNIT_TESTS::TestErrors *test_errors) {
    OwnedArr arr1 = copy_arr("HELLO", 6);
    OwnedArr arr2 = copy_arr("hello", 6);
    TEST_STR_EQ(arr1, arr2);
  }
}
#endif

void UNIT_TESTS::TestContext::run_test(const UnitTest& t) {
    IO::format("Starting test: {}", t.test_name);

    UNIT_TESTS::TestErrors errors = {};
    errors.test_name = t.test_name;

    try {
      t.test_func(&errors);
    }
    catch (const std::exception &e) {
      const char *message = e.what();
      const ViewArr<const char> message_view = {message, strlen_ts(message)};
      errors.report_error("Test failed with exception: {}", message_view);
    }
    catch (...) {
      errors.report_error("Test failed with unknown thrown type...");
    }

    if (errors.is_panic()) {
      IO::print("\t - failed\n");
      failed_tests.insert(std::move(errors));
    } else {
      IO::print("\t - passed\n");
    }

}

void UNIT_TESTS::print_failed_tests(const UNIT_TESTS::TestContext& context) {
  if (context.failed_tests.size == 0) {
    IO::print("\nAll tests succeeded!");
  } else {
    IO::err_print("\nSome tests failed!\n");

    for (const auto &t : context.failed_tests) {
      OwnedArr ts = format_type_set(view_arr(t.first_error), 2, 20);

      IO::err_format("\n===========\n{} failed with errors:\n{}\n", 
                     t.test_name, ts);
    }
  }
}

void UNIT_TESTS::test_main() {
  #ifdef AXLE_TEST_SANITY
  {
    //Test that the tests fail correctly

    usize failed_count = 0;

    const UNIT_TESTS::UnitTest fail_tests[] = {
        {lit_view_arr("FAIL_TESTS::Report Error"), FAIL_TESTS::report},
        {lit_view_arr("FAIL_TESTS::TEST_EQ"), FAIL_TESTS::test_eq},
        {lit_view_arr("FAIL_TESTS::TEST_NEQ"), FAIL_TESTS::test_neq},
        {lit_view_arr("FAIL_TESTS::TEST_EQ pointer"), FAIL_TESTS::test_eq_ptr},
        {lit_view_arr("FAIL_TESTS::TEST_NEQ pointer"),
         FAIL_TESTS::test_neq_ptr},
        {lit_view_arr("FAIL_TESTS::TEST_ARR_EQ size"),
         FAIL_TESTS::test_arr_eq_size},
        {lit_view_arr("FAIL_TESTS::TEST_ARR_EQ values"),
         FAIL_TESTS::test_arr_eq_values},
        {lit_view_arr("FAIL_TESTS::TEST_STR_EQ ViewArr"),
         FAIL_TESTS::test_str_eq_view},
        {lit_view_arr("FAIL_TESTS::TEST_STR_EQ OwnedArr"),
         FAIL_TESTS::test_str_eq_owned},
    };
    for (const auto &fail_test : fail_tests) {
      // Test the tester
      UNIT_TESTS::TestErrors errors = {};
      errors.test_name = fail_test.test_name;

      IO::format("Starting fail test: {}", fail_test.test_name);

      if (errors.is_panic()) {
        IO::format("\t - Failed Before entering the test!\n");
        failed_count += 1;
        continue;
      }

      fail_test.test_func(&errors);

      if (!errors.is_panic()) {
        IO::format("\t - Failed\n");
        failed_count += 1;
      } else {
        IO::format("\t - Passed\n");
      }
    }

    if (failed_count > 0) {
      IO::format("{} sanity tests failed. Stopping ...\n", failed_count);
      return;
    }
  }
  #endif

  UNIT_TESTS::TestContext context = {};

  for (const auto &t : UNIT_TESTS::unit_tests_ref()) {
    context.run_test(t);
  }

  UNIT_TESTS::print_failed_tests(context);
}
