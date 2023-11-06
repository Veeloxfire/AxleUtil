#include <AxleUtil/io.h>
#include <AxleUtil/threading.h>

#include <Tracer/trace.h>
#include <cstdio>

static SpinLockMutex io_mutex = {};

void IO_Single::lock() {
  io_mutex.acquire();
}

void IO_Single::unlock() {
  io_mutex.release();
}

void IO_Single::print(const char* string, usize n) {
  TRACING_FUNCTION();
  fwrite(string, 1, n, stdout);
}

void IO_Single::print(const ViewArr<char>& string) {
  TRACING_FUNCTION();
  fwrite(string.data, 1, string.size, stdout);
}

void IO_Single::print(const ViewArr<const char>& string) {
  TRACING_FUNCTION();
  fwrite(string.data, 1, string.size, stdout);
}

void IO_Single::print(const char c) {
  TRACING_FUNCTION();
  fputc(c, stdout);
}

void IO_Single::err_print(const char* string, usize n) {
  TRACING_FUNCTION();
  fwrite(string, 1, n, stderr);
}

void IO_Single::err_print(const ViewArr<char>& string) {
  TRACING_FUNCTION();
  fwrite(string.data, 1, string.size, stderr);
}

void IO_Single::err_print(const ViewArr<const char>& string) {
  TRACING_FUNCTION();
  fwrite(string.data, 1, string.size, stderr);
}

void IO_Single::err_print(const char c) {
  TRACING_FUNCTION();
  fputc(c, stderr);
}