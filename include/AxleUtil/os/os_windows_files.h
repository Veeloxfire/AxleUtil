#ifndef AXLEUTIL_OS_WINDOWS_FILES_H_
#define AXLEUTIL_OS_WINDOWS_FILES_H_
#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/files_base.h>

#include <AxleUtil/utility.h>

namespace Axle::FILES::Base {
  template<>
  void handle_close<HANDLE>(HANDLE t);
  template<>
  void handle_seek_from_start<HANDLE>(HANDLE t, usize size);
  template<>
  usize handle_file_size<HANDLE>(HANDLE t);
  template<>
  void handle_write<HANDLE>(HANDLE t, const u8* data, usize size);
  template<>
  void handle_read<HANDLE>(HANDLE t, u8* data, usize size);
}

namespace Axle::Windows::FILES {
  using FileData = Axle::FILES::Base::FileData<HANDLE>;

  using Axle::FILES::ErrorCode;
  using Axle::FILES::OPEN_MODE;

  ErrorCode open(FileData*& data,
                 const NativePath& name,
                 OPEN_MODE open_mode);
  ErrorCode create(FileData*& data,
                   const NativePath& name,
                   OPEN_MODE open_mode);
  ErrorCode replace(FileData*& data,
                    const NativePath& name,
                    OPEN_MODE open_mode);

  ErrorCode create_empty_directory(const NativePath& name);

  OwnedArr<u8> read_full_file(const NativePath& file_name);

  bool exists(const NativePath& name);

  constexpr bool is_absolute_path(const ViewArr<const char>& r) {
    constexpr auto is_character = [](char c) {
      return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'z');
    };

    return (r.size >= 3)
      && is_character(r.data[0])
      && r.data[1] == ':'
      && (r.data[2] == '\\' || r.data[2] == '/');
  }

  struct RawFile {
    HANDLE handle;
  };

  struct TimeoutFile {
    HANDLE hwaitevent;
    HANDLE handle;
    u32 timeout;
  };

  struct DirectoryIterator {
    WIN32_FIND_DATAA data = {};
    HANDLE find_handle = INVALID_HANDLE_VALUE;

    DirectoryIterator(const DirectoryIterator&) = delete;
    DirectoryIterator& operator=(const DirectoryIterator&) = delete;
    
    DirectoryIterator() noexcept = default;
    DirectoryIterator(DirectoryIterator&&) noexcept;
    DirectoryIterator& operator=(DirectoryIterator&&) noexcept;
    ~DirectoryIterator() noexcept;

    bool valid_find() noexcept;
    void find_next() noexcept;

    void operator++() noexcept;

    bool operator<(Axle::FILES::DirectoryIteratorEnd) const noexcept;

    Axle::FILES::DirectoryElement operator*() const noexcept;
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

  template<ByteOrder Ord>
  struct Serializer<Axle::Windows::FILES::TimeoutFile, Ord> {
    Axle::Windows::FILES::TimeoutFile rf;
    constexpr Serializer(Axle::Windows::FILES::TimeoutFile h) : rf{h} {}

    inline bool read_bytes(const ViewArr<u8>& bytes) {
      OVERLAPPED overlapped;
      memset(&overlapped, 0, sizeof(overlapped));
      overlapped.hEvent = rf.hwaitevent;

      DWORD read = 0;
      BOOL res = ReadFile(rf.handle, bytes.data, static_cast<u32>(bytes.size), &read, &overlapped);

      if(res != 0) {
        return static_cast<usize>(read) == bytes.size;
      }
      else {
        DWORD qwait = GetLastError();
        if(qwait != ERROR_IO_PENDING) return false;

        DWORD wait_res = WaitForSingleObject(overlapped.hEvent, rf.timeout);
        if(wait_res != WAIT_OBJECT_0) return false;

        res = GetOverlappedResult(rf.handle, &overlapped, &read, false);
        return res != 0 && static_cast<usize>(read) == bytes.size;
      }
    }

    inline void write_bytes(const ViewArr<const u8>& bytes) {
      OVERLAPPED overlapped;
      memset(&overlapped, 0, sizeof(overlapped));
      overlapped.hEvent = rf.hwaitevent;

      DWORD written = 0;
      BOOL res = WriteFile(rf.handle, bytes.data, static_cast<u32>(bytes.size), &written, &overlapped); 

      if(res != 0) {
        ASSERT(written == bytes.size);
        return;
      }
      else {
        DWORD qwait = GetLastError();
        ASSERT(qwait == ERROR_IO_PENDING);

        DWORD wait_res = WaitForSingleObject(overlapped.hEvent, rf.timeout);
        ASSERT(wait_res == WAIT_OBJECT_0);

        res = GetOverlappedResult(rf.handle, &overlapped, &written, false);
        ASSERT(res != 0 && static_cast<usize>(written) == bytes.size);
      }
    }
  };
}

#endif
