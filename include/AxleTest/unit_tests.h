#ifndef AXLETEST_UNIT_TEST_H_
#define AXLETEST_UNIT_TEST_H_

#include <AxleUtil/format.h>
#include <AxleUtil/utility.h>

#include <AxleTest/ipc.h>
#include <bit>

namespace AxleTest {
  using Axle::usize;

  struct TestErrors {
    Axle::ViewArr<const char> test_name;
    Axle::OwnedArr<const char> first_error;
    
    template<typename ... T>
    void report_error(const Axle::Format::FormatString& fstring, const T& ... t) {
      if(first_error.data != nullptr) {
        return;
      }

      first_error = Axle::format(fstring, t...);
    }

    constexpr bool is_panic() const { return first_error.data != nullptr; }
  };

  using INTERNAL_TEST_FN = void(*)(TestErrors* test_errors, const IPC::OpaqueContext& context);

  template<typename T>
  using ACTUAL_TEST_FN = void(*)(TestErrors* test_errors, const T* context);


  struct UnitTest {
    Axle::ViewArr<const char> test_name;
    Axle::ViewArr<const char> context_name;
    INTERNAL_TEST_FN test_func;
  };

  Axle::Array<UnitTest>& unit_tests_ref();

  template<typename T>
  struct ContextName;

  template<void(*TEST_FN)(TestErrors*)>
  constexpr void call_nocontext_test(TestErrors* errors, const IPC::OpaqueContext& oc) {
      ASSERT(oc.data.size == 0);
      ASSERT(oc.name.size == 0);
      TEST_FN(errors); 
  }

  template<typename Ctx, void(*TEST_FN)(TestErrors*, const Ctx*)>
  constexpr void call_context_test(TestErrors* errors, const IPC::OpaqueContext& oc) {
      ASSERT(oc.data.size == sizeof(Ctx));
      ASSERT(memeq_ts(oc.name, {ContextName<Ctx>::Name}));

      u8 holder[sizeof(Ctx)] = {};
      memcpy_ts(view_arr(holder), oc.data);  
      Ctx ctx = std::bit_cast<Ctx>(holder);
      TEST_FN(errors, &ctx);
  }

  
  template<typename Ctx, void(*TEST_FN)(TestErrors*, const Ctx*)>
  struct _testAdder {
    _testAdder(const Axle::ViewArr<const char>& test_name, const Axle::ViewArr<const char>& context_name) {
      unit_tests_ref().insert({test_name, context_name, call_nocontext_test<TEST_FN>});
    }
  };
  
  template<void(*TEST_FN)(TestErrors*)>
  struct _testAdderNoContext {
    _testAdderNoContext(const Axle::ViewArr<const char> &test_name) {
      unit_tests_ref().insert({test_name, {}, call_nocontext_test<TEST_FN>});
    }
  };

  template<typename T>
  struct TestFormat {
    const T& t;
  };

  template<typename T>
  void test_eq(TestErrors* errors, usize line,
               const Axle::ViewArr<const char>& expected_str, const T& expected,
               const Axle::ViewArr<const char>& actual_str, const T& actual) {
    if (expected != actual) {
      errors->report_error("Test assert failed!\nLine: {}, Test: {}\nExpected: {} = {}\nActual: {} = {}",
                           line, errors->test_name, 
                           expected_str, TestFormat(expected),
                           actual_str, TestFormat(actual));
    }
  }

  template<typename T>
  void test_neq(TestErrors* errors, usize line, 
                const Axle::ViewArr<const char>& expected_str, const T& expected,
                const Axle::ViewArr<const char>& actual_str, const T& actual) {
    if (expected == actual) {
      errors->report_error("Test assert failed!\nLine: {}, Test: {}\n{} = {}\n{} = {}\nThese should not be equal",
                           line, errors->test_name, 
                           expected_str, TestFormat(expected),
                           actual_str, TestFormat(actual));
    }
  }
 
  template<typename T>
  void test_eq_arr(TestErrors* errors, usize line,
                   const Axle::ViewArr<const char>& expected_str, const T* expected,
                   const Axle::ViewArr<const char>& esize_str, usize e_size,
                   const Axle::ViewArr<const char>& actual_str, const T* actual,
                   const Axle::ViewArr<const char>& asize_str, usize a_size) {
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
    constexpr auto t_format = []<typename U>(Axle::Format::Formatter auto& res,
                                             const U& t) {
      Axle::Format::FormatArg<TestFormat<U>>::load_string(res, {t});
    };

    errors->report_error("Test assert failed!\nLine: {}, Test: {}\n"
                         "Expected Size: {} = {}\nActual Size: {} = {}\n"
                         "Expected Array: {} = {}\n"
                         "Actual Array: {} = {}",
                         line, errors->test_name, esize_str, e_size, asize_str, a_size,
                         expected_str, Axle::PrintListCF{expected, e_size, t_format},
                         actual_str, Axle::PrintListCF{actual, a_size, t_format});
    return;
  }

  inline void test_eq_str_impl(TestErrors* errors, usize line,
                               const Axle::ViewArr<const char>& expected_str, const Axle::ViewArr<const char>& expected,
                               const Axle::ViewArr<const char>& actual_str, const Axle::ViewArr<const char>& actual) {
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
                         expected_str, Axle::DisplayString{ expected.data, expected.size },
                         actual_str, Axle::DisplayString{ actual.data, actual.size });
    return;
  }

  template<typename L, typename R>
  void test_eq_str(TestErrors* errors, usize line,
                   const Axle::ViewArr<const char>& expected_str, const L& expected,
                   const Axle::ViewArr<const char>& actual_str, const R& actual) {
    return test_eq_str_impl(errors, line,
                            expected_str, view_arr<const char>(expected),
                            actual_str, view_arr<const char>(actual));
  }
}

namespace Axle::Format {
  template<typename T>
  struct FormatArg<AxleTest::TestFormat<T>> {
    template<Formatter F>
    constexpr static void load_string(F& res, AxleTest::TestFormat<T> tf) {
      Format::FormatArg<T>::load_string(res, tf.t);
    }
  };

  template<typename T>
  struct FormatArg<AxleTest::TestFormat<T*>> {
    template<Formatter F>
    constexpr static void load_string(F& res, AxleTest::TestFormat<T*> tf) {
      Format::FormatArg<PrintPtr>::load_string(res, {tf.t});
    }
  };
}

#define TEST_EQ(expected, actual) do {\
AxleTest::test_eq(test_errors, __LINE__, Axle::lit_view_arr(#expected), expected, Axle::lit_view_arr(#actual), actual);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_NEQ(expected, actual) do {\
AxleTest::test_neq(test_errors, __LINE__, Axle::lit_view_arr(#expected), expected, Axle::lit_view_arr(#actual), actual);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_ARR_EQ(expected, e_size, actual, a_size) do { \
AxleTest::test_eq_arr(test_errors, __LINE__, Axle::lit_view_arr(#expected), expected, Axle::lit_view_arr(#e_size), e_size, Axle::lit_view_arr(#actual), actual, Axle::lit_view_arr(#a_size), a_size);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_STR_EQ(expected, actual) do { \
AxleTest::test_eq_str(test_errors, __LINE__, Axle::lit_view_arr(#expected), expected, Axle::lit_view_arr(#actual), actual);\
if (test_errors->is_panic()) return; } while (false)

#define TEST_FUNCTION_CTX(space, name, ctx_ty) namespace AxleTest:: JOIN(_anon_ns_ ## space, __LINE__) { static void JOIN(_anon_tf_ ## name, __LINE__) (AxleTest::TestErrors*); } static AxleTest::_testAdder<ctx_ty, AxleTest:: JOIN( _anon_ns_ ## space, __LINE__) :: JOIN(_anon_tf_ ## name, __LINE__)> JOIN(_test_adder_, __LINE__) = {Axle::lit_view_arr(#space "::" #name), Axle::lit_view_arr(#ctx_ty) }; static void AxleTest:: JOIN( _anon_ns_ ## space, __LINE__) :: JOIN(_anon_tf_ ## name, __LINE__) (AxleTest::TestErrors* test_errors)

#define TEST_FUNCTION(space, name) namespace AxleTest:: JOIN(_anon_ns_ ## space, __LINE__) { static void JOIN(_anon_tf_ ## name, __LINE__) (AxleTest::TestErrors*); } static AxleTest::_testAdderNoContext<AxleTest:: JOIN( _anon_ns_ ## space, __LINE__) :: JOIN(_anon_tf_ ## name, __LINE__)> JOIN(_test_adder_, __LINE__) = {Axle::lit_view_arr(#space "::" #name) }; static void AxleTest:: JOIN( _anon_ns_ ## space, __LINE__) :: JOIN(_anon_tf_ ## name, __LINE__) (AxleTest::TestErrors* test_errors)

#define TEST_CHECK_ERRORS() do { if(test_errors->is_panic()) return; } while(false)

#endif
