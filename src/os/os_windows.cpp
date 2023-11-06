#include <AxleUtil/os/os_windows.h>
#include <processthreadsapi.h>

#include <Windows.h>

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