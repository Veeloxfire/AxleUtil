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

  struct RawFile {
    HANDLE handle;
  };
}

namespace Axle {
  template<ByteOrder Ord>
  struct Serializer<Axle::Windows::FILES::RawFile, Ord> {
    Axle::Windows::FILES::RawFile rf;
    constexpr Serializer(Axle::Windows::FILES::RawFile h) : rf{h} {}

    inline bool read_bytes(const ViewArr<u8>& bytes) {
      DWORD read = 0;
      BOOL res = ReadFile(rf.handle, bytes.data, static_cast<u32>(bytes.size), &read, NULL);
      return (res != 0 && static_cast<usize>(read) == bytes.size);
    }

    inline void write_bytes(const ViewArr<const u8>& bytes) {
      DWORD written = 0;
      BOOL res = WriteFile(rf.handle, bytes.data, static_cast<u32>(bytes.size), &written, NULL); 
      ASSERT(res != 0);
      ASSERT(written == bytes.size);
    }
  };
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
