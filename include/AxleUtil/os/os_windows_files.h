#ifndef AXLEUTIL_OS_WINDOWS_FILES_H_
#define AXLEUTIL_OS_WINDOWS_FILES_H_
#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/files.h>

namespace Axle::Windows::FILES {
  using Axle::FILES::ErrorCode;
  using Axle::FILES::OPEN_MODE;
  using Axle::FILES::OpenedFile;

  OpenedFile open(const NativePath& name,
                  OPEN_MODE open_mode);
  OpenedFile create(const NativePath& name,
                    OPEN_MODE open_mode);
  OpenedFile replace(const NativePath& name,
                     OPEN_MODE open_mode);

  ErrorCode create_empty_directory(const NativePath& name);

  OwnedArr<u8> read_full_file(const NativePath& file_name);

  bool exist(const NativePath& name);
}

namespace Axle::FILES {
  struct FileData {
    static constexpr usize BUFFER_SIZE = 1024;
  
    HANDLE handle;
    usize real_file_ptr;
    usize real_file_size;

    usize abstract_file_ptr;
    usize abstract_file_size;

    usize real_buffer_ptr;

    bool in_sync;
    u32 buffer_size;
    u8 buffer[BUFFER_SIZE];

    FileData(HANDLE h);
    ~FileData() noexcept(false);
  };
}

#endif
