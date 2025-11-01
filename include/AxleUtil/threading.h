#ifndef AXLEUTIL_THREADING_H_
#define AXLEUTIL_THREADING_H_

#include <AxleUtil/safe_lib.h>

namespace Axle {
struct ThreadID {
  u32 id;
};
inline thread_local ThreadID THREAD_ID = {1};

struct Mutex {
  volatile u32 held = 0;
  u32 try_hold() noexcept;

  void acquire() noexcept;
  bool acquire_if_free() noexcept;
  void release() noexcept;

  bool is_free() noexcept;
  void wait_until_free() noexcept;
};

struct Signal {
  mutable volatile char held = 0;

  void set() noexcept;
  void unset() noexcept;

  bool test() const noexcept;
};

struct WriteMutex {
  Mutex write;
  volatile u32 readers = 0;

  void acquire_read() noexcept;
  void release_read() noexcept;
  void acquire_write() noexcept;
  void release_write() noexcept;
};

template<typename T>
struct AtomicLock {
  Mutex* _mutex = nullptr;
  T* _ptr = nullptr;

  T* operator->() const {
    return _ptr;
  }

  T& operator*() const {
    return *_ptr;
  }

  void release() {
    _mutex->release();
    _mutex = nullptr;
    _ptr = nullptr;
  }

  bool is_valid() const {
    return _mutex != nullptr;
  }

  ~AtomicLock() {
    if (_mutex) _mutex->release();
  }
};

template<typename T>
struct AtomicPtr {
  mutable Mutex _mutex;
  T* _ptr = nullptr;

  void set(T* t) {
    _mutex.acquire();
    _ptr = t;
    _mutex.release();
  }

  AtomicLock<T> get() const {
    _mutex.acquire();
    return { &_mutex, _ptr };
  }

  void get_load(AtomicLock<T>* lock) const {
    _mutex.acquire();
    lock->_mutex = &_mutex;
    lock->_ptr = _ptr;
  }

  AtomicLock<T> get_if_free(bool* is_acquired) const {
    bool acquired = _mutex.acquire_if_free();
    if (acquired) {
      *is_acquired = true;
      return { &_mutex, _ptr };
    }
    else {
      return { nullptr, nullptr };
    }
  }
};

struct ThreadHandle;

using THREAD_PROC = void(*)(const ThreadHandle*, void*);

const ThreadHandle* start_thread(THREAD_PROC thread_proc, void* data);

template<auto thread_proc, typename T> requires(requires(const ThreadHandle* h, T* t) {
  { thread_proc(h, t) } -> IS_SAME_TYPE<void>;
})
const ThreadHandle* start_thread(T* data) {
  return start_thread(
    +[](const ThreadHandle* h, void* d) { thread_proc(h, reinterpret_cast<T*>(d)); },
    reinterpret_cast<void*>(data)
  );
}

void wait_for_thread_end(const ThreadHandle* thread);
}
#endif
