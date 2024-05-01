#include <AxleTest/unit_tests.h>

#ifndef STACKTRACE_ENABLE
#define STACKTRACE_ENABLE
#endif
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
