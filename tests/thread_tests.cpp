#include <AxleTest/unit_tests.h>

using namespace Axle::Primitives;

TEST_FUNCTION(Threading, DefaultID) {
  TEST_EQ(static_cast<u32>(1), Axle::THREAD_ID.id);
}

TEST_FUNCTION(Threading, Mutex) {
  Axle::SpinLockMutex mutex = {};

  TEST_EQ(static_cast<u32>(0), static_cast<u32>(mutex.held));

  mutex.acquire();
  TEST_EQ(Axle::THREAD_ID.id, static_cast<u32>(mutex.held));

  mutex.acquire();
  TEST_EQ(Axle::THREAD_ID.id, static_cast<u32>(mutex.held));

  mutex.release(); 
  TEST_EQ(static_cast<u32>(0), static_cast<u32>(mutex.held));

  {
    bool ac = mutex.acquire_if_free();
    TEST_EQ(true, ac);
    TEST_EQ(Axle::THREAD_ID.id, static_cast<u32>(mutex.held));
  }

  {
    bool ac = mutex.acquire_if_free();
    TEST_EQ(true, ac);
    TEST_EQ(Axle::THREAD_ID.id, static_cast<u32>(mutex.held));
  }

  mutex.release();
  TEST_EQ(static_cast<u32>(0), static_cast<u32>(mutex.held));
}

TEST_FUNCTION(Threading, Signal) {
  Axle::Signal signal = {};

  TEST_EQ(false, signal.test());
  signal.unset();
  TEST_EQ(false, signal.test());
  signal.set();
  TEST_EQ(true, signal.test());
  signal.set();
  TEST_EQ(true, signal.test());
  signal.unset();
  TEST_EQ(false, signal.test());
}

struct Shared {
  AxleTest::TestErrors* test_errors;
  Axle::Signal signal;
};

static void other_thread_1(const Axle::ThreadHandle* handle, void* data) {
  Shared* shared = reinterpret_cast<Shared*>(data);
  AxleTest::TestErrors* test_errors = shared->test_errors;

  TEST_NEQ(static_cast<const Axle::ThreadHandle*>(nullptr), handle);
  TEST_EQ(static_cast<u32>(2), Axle::THREAD_ID.id);

  shared->signal.set();
}

static void other_thread_2(const Axle::ThreadHandle* handle, void* data) {
  Shared* shared = reinterpret_cast<Shared*>(data);
  AxleTest::TestErrors* test_errors = shared->test_errors;

  TEST_NEQ(static_cast<const Axle::ThreadHandle*>(nullptr), handle);
  TEST_EQ(static_cast<u32>(3), Axle::THREAD_ID.id);
  
  shared->signal.set();
}

TEST_FUNCTION(Threading, Thread) {
  {
    Shared shared = {
      test_errors, {}
    };
    const Axle::ThreadHandle* handle = Axle::start_thread(other_thread_1, &shared);
    TEST_NEQ(static_cast<const Axle::ThreadHandle*>(nullptr), handle);

    Axle::wait_for_thread_end(handle);
    
    TEST_EQ(true, shared.signal.test());
  }

  {
    Shared shared = {
      test_errors, {}
    };
    const Axle::ThreadHandle* handle = Axle::start_thread(other_thread_2, &shared);
    TEST_NEQ(static_cast<const Axle::ThreadHandle*>(nullptr), handle);

    Axle::wait_for_thread_end(handle);

    TEST_EQ(true, shared.signal.test());
  }
}
