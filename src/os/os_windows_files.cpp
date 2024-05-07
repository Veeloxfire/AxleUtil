#include <AxleUtil/os/os_windows_files.h>


namespace Axle::FILES::Base {
  void handle_close<HANDLE>(HANDLE t) {
    CloseHandle(t);
  }

  void handle_seek_from_start<HANDLE>(HANDLE h, usize ptr) {
    LARGE_INTEGER li = {};
    li.QuadPart = ptr; 

    SetFilePointerEx(h, li, &li, FILE_BEGIN);

    ASSERT(static_cast<usize>(li.QuadPart) == ptr);
  }

  usize handle_file_size<HANDLE>(HANDLE h) {
    LARGE_INTEGER li = { {0} };
    GetFileSizeEx(h, &li);
    return li.QuadPart;
  }

  void handle_write<HANDLE>(HANDLE h, const u8* data, usize size) {
    ASSERT(static_cast<usize>(static_cast<DWORD>(size)) == size);

    DWORD bytes_written = 0;
    BOOL wrote = WriteFile(h, data, static_cast<DWORD>(size), &bytes_written, NULL);

    ASSERT(wrote);
    ASSERT(static_cast<usize>(bytes_written) == size);
  }

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
}
