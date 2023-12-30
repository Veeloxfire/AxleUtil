#include <AxleTest/unit_tests.h>

#define STACKTRACE_ENABLE
#include <AxleUtil/stacktrace.h>

using namespace Axle::Stacktrace;

static Axle::OwnedArr<char> func3() {
  STACKTRACE_FUNCTION();

  Axle::Array<char> str = {};
  const TraceNode* tn = EXECUTION_TRACE;
  while(tn != nullptr) {
    str.concat(tn->name);
    tn = tn->prev;
  }

  return Axle::bake_arr(std::move(str));
}

static Axle::OwnedArr<char> func2() {
  STACKTRACE_SCOPE("Test");
  return func3();
}

static Axle::OwnedArr<char> func1() {
  STACKTRACE_FUNCTION();

  return func2();
}

TEST_FUNCTION(Stacktrace, gather) {
  ASSERT(Axle::Stacktrace::EXECUTION_TRACE == nullptr);

  Axle::OwnedArr<char> trace_names = func1();

  const Axle::ViewArr<const char> expected = 
    Axle::lit_view_arr("func3Testfunc1");

  TEST_STR_EQ(expected, trace_names);
}

TEST_FUNCTION(Stacktrace, throwing) {
  ASSERT(Axle::Stacktrace::EXECUTION_TRACE == nullptr);
  try {
    Axle::throw_testing_assertion("hello");

    test_errors->report_error("Test did not throw");
  }
  catch(const std::exception& e) {
    const char* msg = e.what();
    usize msg_len = Axle::strlen_ts(msg);

    const Axle::ViewArr<const char> msg_view{msg, msg_len};
    const Axle::ViewArr<const char> expected = Axle::lit_view_arr("hello");
    TEST_STR_EQ(expected, msg_view);
  }

  try {
    Axle::Stacktrace::ScopedExecTrace scope1(Axle::lit_view_arr("scope1"));
    Axle::Stacktrace::ScopedExecTrace scope2(Axle::lit_view_arr("scope2"));
    Axle::Stacktrace::ScopedExecTrace scope3(Axle::lit_view_arr("scope3"));
    Axle::throw_testing_assertion("hello");

    test_errors->report_error("Test did not throw");
  }
  catch(const std::exception& e) {
    TEST_EQ(static_cast<const Axle::Stacktrace::TraceNode*>(nullptr), Axle::Stacktrace::EXECUTION_TRACE);

    const char* msg = e.what();
    usize msg_len = Axle::strlen_ts(msg);

    const Axle::ViewArr<const char> msg_view{msg, msg_len};
    const Axle::ViewArr<const char> expected = Axle::lit_view_arr("hello\nStacktrace:\n- scope3\n- scope2\n- scope1");
    TEST_STR_EQ(expected, msg_view);
  }
}
