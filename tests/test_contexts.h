#ifndef AXLETEST_TEST_CONTEXT_H_
#define AXLETEST_TEST_CONTEXT_H_

#include <AxleTest/ipc.h>

namespace TestContexts {
  struct Integer {
    Axle::u32 i;
  };
}

namespace AxleTest::IPC {
  template<>
  struct ContextName<TestContexts::Integer> {
    static constexpr Axle::ViewArr<const char> NAME = Axle::lit_view_arr("TestContexts::Integer");
  };
}

#endif
