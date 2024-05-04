#include <AxleUtil/os/os_windows.h>
#include <processthreadsapi.h>

#include <Windows.h>

#include <utility>

namespace Axle {
Windows::NativePath Windows::get_current_directory() {
  NativePath str = {};
  memset(str.path, 0, MAX_PATH + 1);

  GetCurrentDirectoryA(MAX_PATH + 1, str.path);

  return str;
}

void Windows::set_current_directory(const ViewArr<const char>& path) {
  NativePath str = path;

  SetCurrentDirectoryA(str.c_str());
}


void Windows::OwnedHandle::close() noexcept {
  if(h != INVALID_HANDLE_VALUE) CloseHandle(h);
  h = INVALID_HANDLE_VALUE;
}

bool Windows::OwnedHandle::is_valid() const noexcept  {
  return h != INVALID_HANDLE_VALUE;
}

Windows::OwnedHandle::operator HANDLE() && noexcept {
  return std::exchange(h, INVALID_HANDLE_VALUE);
}

Windows::OwnedHandle::OwnedHandle() noexcept : h(INVALID_HANDLE_VALUE) {}
Windows::OwnedHandle::OwnedHandle(HANDLE&& h_) noexcept : h(std::exchange(h_, INVALID_HANDLE_VALUE)) {}
Windows::OwnedHandle::OwnedHandle(OwnedHandle&& h_) noexcept : h(std::exchange(h_.h, INVALID_HANDLE_VALUE)) {}

Windows::OwnedHandle& Windows::OwnedHandle::operator=(HANDLE&& h_) noexcept {
  h = std::exchange(h_, INVALID_HANDLE_VALUE);
  return *this;
}

Windows::OwnedHandle& Windows::OwnedHandle::operator=(OwnedHandle&& h_) noexcept {
  if(this == &h_) return *this;

  h = std::exchange(h_.h, INVALID_HANDLE_VALUE);
  return *this;
}

Windows::OwnedHandle::~OwnedHandle() noexcept {
  if(h != INVALID_HANDLE_VALUE) CloseHandle(h);
}
}
