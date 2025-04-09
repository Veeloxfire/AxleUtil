#include <AxleUtil/io.h>
#include <AxleUtil/threading.h>
#include <AxleUtil/tracing_wrapper.h>

#include <cstdio>
namespace Axle {
static SpinLockMutex io_mutex = {};

void IO_Single::lock() {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  io_mutex.acquire();
}

void IO_Single::unlock() {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  io_mutex.release();
}

void IO_Single::print(const char* string, usize n) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fwrite(string, 1, n, stdout);
}

void IO_Single::print(const ViewArr<char>& string) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fwrite(string.data, 1, string.size, stdout);
}

void IO_Single::print(const ViewArr<const char>& string) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fwrite(string.data, 1, string.size, stdout);
}

void IO_Single::print(const char c) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fputc(c, stdout);
}

void IO_Single::err_print(const char* string, usize n) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fwrite(string, 1, n, stderr);
}

void IO_Single::err_print(const ViewArr<char>& string) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fwrite(string.data, 1, string.size, stderr);
}

void IO_Single::err_print(const ViewArr<const char>& string) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fwrite(string.data, 1, string.size, stderr);
}

void IO_Single::err_print(const char c) {
  AXLE_UTIL_TELEMETRY_FUNCTION();
  fputc(c, stderr);
}
}
