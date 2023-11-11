#include <AxleUtil/io.h>
#include <AxleUtil/threading.h>

#ifdef AXLE_TRACING
#include <Tracer/trace.h>
#endif

#include <cstdio>

static SpinLockMutex io_mutex = {};

void IO_Single::lock() {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  io_mutex.acquire();
}

void IO_Single::unlock() {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  io_mutex.release();
}

void IO_Single::print(const char* string, usize n) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fwrite(string, 1, n, stdout);
}

void IO_Single::print(const ViewArr<char>& string) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fwrite(string.data, 1, string.size, stdout);
}

void IO_Single::print(const ViewArr<const char>& string) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fwrite(string.data, 1, string.size, stdout);
}

void IO_Single::print(const char c) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fputc(c, stdout);
}

void IO_Single::err_print(const char* string, usize n) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fwrite(string, 1, n, stderr);
}

void IO_Single::err_print(const ViewArr<char>& string) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fwrite(string.data, 1, string.size, stderr);
}

void IO_Single::err_print(const ViewArr<const char>& string) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fwrite(string.data, 1, string.size, stderr);
}

void IO_Single::err_print(const char c) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif
  fputc(c, stderr);
}
