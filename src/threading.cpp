#include <AxleUtil/safe_lib.h>
#include <AxleUtil/threading.h>
#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/memory.h>
#include <AxleUtil/stacktrace.h>
#include <AxleUtil/tracing_wrapper.h>

#include <intrin.h>

namespace Axle {
static volatile u32 thread_id_counter = 1;

static constexpr u32 MAX_SPIN_BEFORE_YIELD = 10;
static constexpr u32 MAX_YIELD_BEFORE_WAIT = 2;

static void wait_until_zero_or_value(volatile u32* held, u32 val) noexcept { 
  u32 spin_counter = 0;
  u32 yield_counter = 0;

  // 3 options depending on how long
  //  we were waiting
  // 1. spin
  // 2. yeild
  // 3. wait

  while (true) {
    u32 res = _InterlockedCompareExchange(held, val, 0u);
    if (res == 0u || res == val) {
      return;
    }

    if (yield_counter < MAX_YIELD_BEFORE_WAIT) {
      if (spin_counter < MAX_SPIN_BEFORE_YIELD) {
        spin_counter += 1;
      }
      else {
        spin_counter = 0;
        yield_counter += 1;
        SwitchToThread();// yield
      }
    }
    else {
      yield_counter = 0;
      WaitOnAddress(&held, &res, 4, INFINITE);
    }
  }
}

static void wait_until_zero(volatile u32* held) noexcept { 
  u32 spin_counter = 0;
  u32 yield_counter = 0;

  // 3 options depending on how long
  //  we were waiting
  // 1. spin
  // 2. yeild
  // 3. wait

  while (true) {
    u32 res = _InterlockedCompareExchange(held, 0u, 0u);
    if (res == 0u) {
      return;
    }

    if (yield_counter < MAX_YIELD_BEFORE_WAIT) {
      if (spin_counter < MAX_SPIN_BEFORE_YIELD) {
        spin_counter += 1;
      }
      else {
        spin_counter = 0;
        yield_counter += 1;
        SwitchToThread();// yield
      }
    }
    else {
      yield_counter = 0;
      WaitOnAddress(&held, &res, 4, INFINITE);
    }
  }
}



u32 Mutex::try_hold() noexcept {
  return _InterlockedCompareExchange(&held, THREAD_ID.id, 0u);
}

bool Mutex::acquire_if_free() noexcept {
  const u32 res = try_hold();
  return res == 0 || res == static_cast<u32>(THREAD_ID.id);
}

bool Mutex::is_free() noexcept {
  return _InterlockedCompareExchange(&held, 0u, 0u) == 0u;
}

void Mutex::acquire() noexcept {
  AXLE_UTIL_TELEMETRY_FUNCTION();

  wait_until_zero_or_value(&held, THREAD_ID.id);
}

void Mutex::wait_until_free() noexcept {
  AXLE_UTIL_TELEMETRY_FUNCTION();

  wait_until_zero(&held);
}

void Mutex::release() noexcept {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  u32 res = _InterlockedCompareExchange(&held, 0u, THREAD_ID.id);
  ASSERT(res == THREAD_ID.id);

  WakeByAddressAll(const_cast<u32*>(&held));
}

void Signal::set() noexcept {
  _InterlockedCompareExchange8(&held, '\1', '\0');
}
void Signal::unset() noexcept {
  _InterlockedCompareExchange8(&held, '\0', '\1');
}

bool Signal::test() const noexcept {
  return _InterlockedCompareExchange8(&held, '\0', '\0') == '\1';
}

void WriteMutex::acquire_read() noexcept {
  while(true) {
    _InterlockedIncrement(&readers);

    if (write.is_free()) {
      // ZOOM
      break;
    }

    // write is not free - need to wait
    // and need to do these to release write
    release_read();
    write.wait_until_free();
  }
}

void WriteMutex::release_read() noexcept {
  u32 res = _InterlockedDecrement(&readers);
  if (res == 0) {
    WakeByAddressAll(const_cast<u32*>(&readers));
  }
}

void WriteMutex::acquire_write() noexcept {
  write.acquire();
  wait_until_zero(&readers);
}

void WriteMutex::release_write() noexcept {
  write.release();
}

struct ThreadHandle {
  DWORD id;
  HANDLE handle;
};

namespace {
  struct ThreadingInfo {
    ThreadHandle* handle;
    void* data;
    THREAD_PROC proc;
  };
  
  DWORD WINAPI generic_thread_proc(
    _In_ LPVOID lpParameter
  ) {
    AXLE_UTIL_TELEMETRY_FUNCTION();
    ThreadingInfo* info = (ThreadingInfo*)lpParameter;

    THREAD_ID.id = InterlockedIncrement(&thread_id_counter);
    info->proc(info->handle, info->data);

    free_destruct_single<ThreadingInfo>(info);
    return 0;
  }
}

const ThreadHandle* start_thread(THREAD_PROC thread_proc, void* data) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  ThreadHandle* handle = allocate_default<ThreadHandle>();
  ThreadingInfo* info = allocate_default<ThreadingInfo>();
  info->data = data;
  info->proc = thread_proc;
  info->handle = handle;

  handle->handle = CreateThread(NULL, 0, &generic_thread_proc, info, 0, &handle->id);
  return handle;
}

void wait_for_thread_end(const ThreadHandle* thread) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  WaitForSingleObject(thread->handle, INFINITE);
  
  free_destruct_single<const ThreadHandle>(thread);
}
}
