#include <AxleTest/ipc.h>

int main() {
  const Axle::ViewArr<AxleTest::IPC::OpaqueContext> contexts = {};
  bool r = AxleTest::IPC::server_main(Axle::lit_view_arr(AXLE_TEST_CLIENT_EXE), contexts);
  if(!r) return -1;
}
