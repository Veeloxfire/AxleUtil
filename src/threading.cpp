#include <AxleUtil/safe_lib.h>
#include <AxleUtil/threading.h>
#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/memory.h>
#include <AxleUtil/stacktrace.h>
#include <AxleUtil/tracing_wrapper.h>

#include <intrin.h>

namespace Axle {
volatile u32 thread_id_counter = 1;

bool SpinLockMutex::acquire_if_free() {
  const u32 res = _InterlockedCompareExchange(&held, THREAD_ID.id, 0u);
  return res == 0 || res == static_cast<u32>(THREAD_ID.id);
}

void SpinLockMutex::acquire() {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  while (!acquire_if_free()) {}
}

void SpinLockMutex::release() {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  u32 res = _InterlockedCompareExchange(&held, 0u, THREAD_ID.id);
  ASSERT(res == THREAD_ID.id);
}

void Signal::set() {
  _InterlockedCompareExchange8(&held, '\1', '\0');
}
void Signal::unset() {
  _InterlockedCompareExchange8(&held, '\0', '\1');
}

bool Signal::test() const {
  return _InterlockedCompareExchange8(&held, '\0', '\0') == '\1';
}

struct ThreadingInfo {
  ThreadHandle* handle;
  void* data;
  THREAD_PROC proc;
};

struct ThreadHandle {
  DWORD id;
  HANDLE handle;
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
