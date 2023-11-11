#ifndef AXLETEST_UNIT_TEST_H_
#define AXLETEST_UNIT_TEST_H_

#include <AxleUtil/format.h>
#include <AxleUtil/utility.h>

namespace UNIT_TESTS {
  struct TestErrors {
    ViewArr<const char> test_name;
    OwnedArr<const char> first_error;
    
    template<typename ... T>
    void report_error(const Format::FormatString& fstring, const T& ... t) {
      if(first_error.data != nullptr) {
        return;
      }

      first_error = format(fstring, t...);
    }

    constexpr bool is_panic() const { return first_error.data != nullptr; }
  };
  using TEST_FN = void(*)(TestErrors* _test_errors);

  struct UnitTest {
    ViewArr<const char> test_name;
    TEST_FN test_func;
  };

  Array<UnitTest>& unit_tests_ref();

  struct _testAdder {
    _testAdder(const ViewArr<const char>& test_name, TEST_FN fn);
  };

  struct TestContext {
    Array<UNIT_TESTS::TestErrors> failed_tests;
  
    void run_test(const UnitTest& test);
  };

  void print_failed_tests(const UNIT_TESTS::TestContext& context);
  void test_main();

  template<typename T>
  struct TestFormat {
    const T& t;
  };

  template<typename T>
  void test_eq(TestErrors* errors, usize line,
               const ViewArr<const char>& expected_str, const T& expected,
               const ViewArr<const char>& actual_str, const T& actual) {
    if (expected != actual) {
      errors->report_error("Test assert failed!\nLine: {}, Test: {}\nExpected: {} = {}\nActual: {} = {}",
                           line, errors->test_name, 
                           expected_str, TestFormat(expected),
                           actual_str, TestFormat(actual));
    }
  }

  template<typename T>
  void test_neq(TestErrors* errors, usize line, 
                const ViewArr<const char>& expected_str, const T& expected,
                const ViewArr<const char>& actual_str, const T& actual) {
    if (expected == actual) {
      errors->report_error("Test assert failed!\nLine: {}, Test: {}\n{} = {}\n{} = {}\nThese should not be equal",
                           line, errors->test_name, 
                           expected_str, TestFormat(expected),
                           actual_str, TestFormat(actual));
    }
  }
 
  template<typename T>
  void test_eq_arr(TestErrors* errors, usize line,
                   const ViewArr<const char>& expected_str, const T* expected,
                   const ViewArr<const char>& esize_str, usize e_size,
                   const ViewArr<const char>& actual_str, const T* actual,
                   const ViewArr<const char>& asize_str, usize a_size) {
    if (e_size != a_size) {
      goto ERROR;
    }

    for (usize n = 0; n < e_size; ++n) {
      const auto& l = expected[n];
      const auto& r = actual[n];
      if (l != r) {
        goto ERROR;
      }
    }

    return;

  ERROR:
    constexpr auto t_format = []<typename U>(Format::Formatter auto& res,
                                             const U& t) {
      Format::FormatArg<TestFormat<U>>::load_string(res, {t});
    };

    errors->report_error("Test assert failed!\nLine: {}, Test: {}\n"
                         "Expected Size: {} = {}\nActual Size: {} = {}\n"
                         "Expected Array: {} = {}\n"
                         "Actual Array: {} = {}",
                         line, errors->test_name, esize_str, e_size, asize_str, a_size,
                         expected_str, PrintListCF{expected, e_size, t_format},
                         actual_str, PrintListCF{actual, a_size, t_format});
    return;
  }

  inline void test_eq_str_impl(TestErrors* errors, usize line,
                               const ViewArr<const char>& expected_str, const ViewArr<const char>& expected,
                               const ViewArr<const char>& actual_str, const ViewArr<const char>& actual) {
    const size_t e_size = expected.size;
    const size_t a_size = actual.size;
    if (e_size != a_size) {
      goto ERROR;
    }

    for (usize n = 0; n < e_size; ++n) {
      const auto& l = expected[n];
      const auto& r = actual[n];
      if (l != r) {
        goto ERROR;
      }
    }

    return;

  ERROR:
    errors->report_error("Test assert failed!\nLine: {}, Test: {}\n"
                         "Expected String: {} = \"{}\"\n"
                         "Actual String: {} = \"{}\"",
                         line, errors->test_name,
                         expected_str, DisplayString{ expected.data, expected.size },
                         actual_str, DisplayString{ actual.data, actual.size });
    return;
  }

  template<typename L, typename R>
  void test_eq_str(TestErrors* errors, usize line,
                   const ViewArr<const char>& expected_str, const L& expected,
                   const ViewArr<const char>& actual_str, const R& actual) {
    return test_eq_str_impl(errors, line,
                            expected_str, view_arr<L, const char>(expected),
                            actual_str, view_arr<R, const char>(actual));
  }
}


namespace Format {
  template<typename T>
  struct FormatArg<UNIT_TESTS::TestFormat<T>> {
    template<Formatter F>
    constexpr static void load_string(F& res, UNIT_TESTS::TestFormat<T> tf) {
      Format::FormatArg<T>::load_string(res, tf.t);
    }
  };

  template<typename T>
  struct FormatArg<UNIT_TESTS::TestFormat<T*>> {
    template<Formatter F>
    constexpr static void load_string(F& res, UNIT_TESTS::TestFormat<T*> tf) {
      Format::FormatArg<PrintPtr>::load_string(res, {tf.t});
    }
  };

}

#define TEST_EQ(expected, actual) do {\
UNIT_TESTS::test_eq(test_errors, __LINE__, lit_view_arr(#expected), expected, lit_view_arr(#actual), actual);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_NEQ(expected, actual) do {\
UNIT_TESTS::test_neq(test_errors, __LINE__, lit_view_arr(#expected), expected, lit_view_arr(#actual), actual);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_ARR_EQ(expected, e_size, actual, a_size) do { \
UNIT_TESTS::test_eq_arr(test_errors, __LINE__, lit_view_arr(#expected), expected, lit_view_arr(#e_size), e_size, lit_view_arr(#actual), actual, lit_view_arr(#a_size), a_size);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_STR_EQ(expected, actual) do { \
UNIT_TESTS::test_eq_str(test_errors, __LINE__, lit_view_arr(#expected), expected, lit_view_arr(#actual), actual);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_FUNCTION(space, name) namespace UNIT_TESTS:: JOIN(_anon_ns_ ## space, __LINE__) { static void JOIN(_anon_tf_ ## name, __LINE__) (UNIT_TESTS::TestErrors*); } static UNIT_TESTS::_testAdder JOIN(_test_adder_, __LINE__) = {lit_view_arr(#space "::" #name), UNIT_TESTS:: JOIN( _anon_ns_ ## space, __LINE__) :: JOIN(_anon_tf_ ## name, __LINE__) }; static void UNIT_TESTS:: JOIN( _anon_ns_ ## space, __LINE__) :: JOIN(_anon_tf_ ## name, __LINE__) (UNIT_TESTS::TestErrors* test_errors)

#define TEST_CHECK_ERRORS() do { if(test_errors->is_panic()) return; } while(false)

#endif
