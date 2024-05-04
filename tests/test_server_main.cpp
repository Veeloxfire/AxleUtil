#include <AxleTest/ipc.h>
#include "test_contexts.h"

int main() {
  TestContexts::Integer i = {0x1234};

  const AxleTest::IPC::OpaqueContext contexts[] = {
    AxleTest::IPC::as_context(i),
  };

  bool r = AxleTest::IPC::server_main(Axle::lit_view_arr(AXLE_TEST_CLIENT_EXE), Axle::view_arr(contexts), 1000);
  if(!r) return -1;
}
