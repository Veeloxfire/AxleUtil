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
  ASSERT(tn != nullptr);
  str.concat(tn->name);
  tn = tn->prev;
  while(tn != nullptr) {
    str.insert(' ');
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
  ASSERT(Axle::Stacktrace::EXECUTION_TRACE != nullptr);
  TEST_STR_EQ(Axle::lit_view_arr("Stacktrace::gather"),
      Axle::Stacktrace::EXECUTION_TRACE->name);

  Axle::OwnedArr<char> trace_names = func1();

  const Axle::ViewArr<const char> expected = 
    Axle::lit_view_arr("func3 Test func1 Stacktrace::gather");

  TEST_STR_EQ(expected, trace_names);
}
