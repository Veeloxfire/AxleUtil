#include <AxleUtil/os/os_windows_files.h>


namespace Axle::FILES::Base {
  template<>
  void handle_close<HANDLE>(HANDLE t) {
    CloseHandle(t);
  }

  template<>
  void handle_seek_from_start<HANDLE>(HANDLE h, usize ptr) {
    LARGE_INTEGER li = { .QuadPart = static_cast<LONGLONG>(ptr) };

    SetFilePointerEx(h, li, &li, FILE_BEGIN);

    ASSERT(static_cast<usize>(li.QuadPart) == ptr);
  }

  template<>
  usize handle_file_size<HANDLE>(HANDLE h) {
    LARGE_INTEGER li = { .QuadPart = 0 };
    GetFileSizeEx(h, &li);
    return li.QuadPart;
  }

  template<>
  void handle_write<HANDLE>(HANDLE h, const u8* data, usize size) {
    ASSERT(static_cast<usize>(static_cast<DWORD>(size)) == size);

    DWORD bytes_written = 0;
    BOOL wrote = WriteFile(h, data, static_cast<DWORD>(size), &bytes_written, NULL);

    ASSERT(wrote);
    ASSERT(static_cast<usize>(bytes_written) == size);
  }

  template<>
  void handle_read<HANDLE>(HANDLE h, u8* data, usize size) {
    ASSERT(static_cast<usize>(static_cast<DWORD>(size)) == size);
    
    DWORD bytes_read = 0;
    BOOL read = ReadFile(h, data, static_cast<DWORD>(size), &bytes_read, NULL);
    ASSERT(read);
    ASSERT(static_cast<usize>(bytes_read) == size);
  }
}

namespace Axle::Windows::FILES {

ErrorCode open(FileData*& data,
               const NativePath& name,
               OPEN_MODE open_mode) {
  DWORD access;
  DWORD share;

  switch (open_mode) {
    case OPEN_MODE::READ: {
        access = GENERIC_READ;
        share = FILE_SHARE_READ;
        break;
      }
    case OPEN_MODE::WRITE: {
        access = GENERIC_WRITE;
        share = 0;
        break;
      }
    default: {
      INVALID_CODE_PATH("Invalid Open Mode");
    }
  }


  HANDLE h = CreateFileA(name.c_str(), access, share, 0, OPEN_EXISTING, 0, 0);
  if (h == INVALID_HANDLE_VALUE) {
    return ErrorCode::COULD_NOT_OPEN_FILE;
  }
  else {
    if(data == nullptr) {
      data = allocate_single_constructed<FileData>(h);
    }
    else {
      Axle::reset_type<FileData>(data, h);
    }

    return ErrorCode::OK;
  }
}

ErrorCode create(FileData*& data,
                 const NativePath& name,
                 OPEN_MODE open_mode) {
  DWORD access;
  DWORD share;

  switch (open_mode) {
    case OPEN_MODE::READ: {
        access = GENERIC_READ;
        share = FILE_SHARE_READ;
        break;
      }
    case OPEN_MODE::WRITE: {
        access = GENERIC_WRITE;
        share = 0;
        break;
      }
    default: {
      INVALID_CODE_PATH("Invalid Open Mode");
    }
  }


  HANDLE h = CreateFileA(name.c_str(), access, share, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE) {
    return ErrorCode::COULD_NOT_OPEN_FILE;
  }
  else {
    if(data == nullptr) {
      data = allocate_single_constructed<FileData>(h);
    }
    else {
      Axle::reset_type<FileData>(data, h);
    }

    return ErrorCode::OK;
  }
}

ErrorCode replace(FileData*& data,
                  const NativePath& name,
                  OPEN_MODE open_mode) {
  DWORD access;
  DWORD share;

  switch (open_mode) {
    case OPEN_MODE::READ: {
        access = GENERIC_READ;
        share = FILE_SHARE_READ;
        break;
      }
    case OPEN_MODE::WRITE: {
        access = GENERIC_WRITE;
        share = 0;
        break;
      }
    default: {
      INVALID_CODE_PATH("Invalid Open Mode");
    }
  }

  HANDLE h = CreateFileA(name.c_str(), access, share, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE) {
    return ErrorCode::COULD_NOT_OPEN_FILE;
  }
  else {
    if(data == nullptr) {
      data = allocate_single_constructed<FileData>(h);
    }
    else {
      Axle::reset_type<FileData>(data, h);
    }

    return ErrorCode::OK;
  }
}

FILES::ErrorCode create_empty_directory(const NativePath& name) {
    BOOL res = CreateDirectoryA(name.c_str(), NULL);
    if(res != 0) return ErrorCode::OK;
    else {
      // Some error - for now just assume okay
      return ErrorCode::OK;
    }
}

bool exists(const NativePath& name) {
  return GetFileAttributesA(name.c_str()) != INVALID_FILE_ATTRIBUTES;
}

OwnedArr<u8> read_full_file(const NativePath& file_name) {
  HANDLE h = CreateFileA(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (h == INVALID_HANDLE_VALUE) return {};
  DEFER(h) { CloseHandle(h); };

  LARGE_INTEGER li = {};
  GetFileSizeEx(h, &li);

  u8* data = allocate_default<u8>(li.QuadPart);
  BOOL read = ReadFile(h, data, (DWORD)li.QuadPart, NULL, NULL);
  ASSERT(read);

  return { data, static_cast<usize>(li.QuadPart) };
}

  DirectoryIterator::DirectoryIterator(DirectoryIterator&& d) noexcept : data(std::move(d.data)), find_handle(std::exchange(d.find_handle, INVALID_HANDLE_VALUE)) {}

  DirectoryIterator& DirectoryIterator::operator=(DirectoryIterator&& d) noexcept {
    if(this == &d) return *this;

    data = std::move(d.data);
    find_handle = std::exchange(d.find_handle, INVALID_HANDLE_VALUE);

    return *this;
  }

  DirectoryIterator::~DirectoryIterator() noexcept {
    if(find_handle != INVALID_HANDLE_VALUE) {
      FindClose(find_handle);
    }
  }

  void DirectoryIterator::find_next() noexcept {
    BOOL b = FindNextFileA(find_handle, &data);
    if(b == 0) {
      ASSERT(GetLastError() == ERROR_NO_MORE_FILES);
      FindClose(find_handle);
      find_handle = INVALID_HANDLE_VALUE;
    }
  }
  
  bool DirectoryIterator::valid_find() noexcept {
    if(find_handle == INVALID_HANDLE_VALUE) return true;

    bool is_self = (data.cFileName[0] == '.' && data.cFileName[1] == '\0');
    bool is_parent = (data.cFileName[0] == '.' && data.cFileName[1] == '.' && data.cFileName[2] == '\0');
    return !(is_self || is_parent);
  }

  void DirectoryIterator::operator++() noexcept {
    do {
      find_next();
    } while(!valid_find());
  }
  
  bool DirectoryIterator::operator<(Axle::FILES::DirectoryIteratorEnd) const noexcept {
    return find_handle != INVALID_HANDLE_VALUE;
  }

  Axle::FILES::DirectoryElement DirectoryIterator::operator*() const noexcept {
    constexpr auto attr_to_type = [](decltype(WIN32_FIND_DATAA::dwFileAttributes) attr) {
      if((attr & FILE_ATTRIBUTE_DIRECTORY) > 0) {
        return Axle::FILES::DirectoryElementType::Directory;
      }
      else {
        return Axle::FILES::DirectoryElementType::File;
      }
    };

    return {
      attr_to_type(data.dwFileAttributes),
      ViewArr<const char>{data.cFileName, strlen_ts(data.cFileName)},
    };
  }
}
