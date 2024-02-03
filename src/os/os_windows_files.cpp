#include <AxleUtil/os/os_windows_files.h>

namespace Axle::Windows::FILES {
using Axle::FILES::FileData;
using Axle::FILES::FileHandle;

FILES::OpenedFile FILES::open(const NativePath& name,
                              OPEN_MODE open_mode) {
  OpenedFile opened_file = {};

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
      return {};
    }
  }


  HANDLE h = CreateFileA(name.c_str(), access, share, 0, OPEN_EXISTING, 0, 0);
  if (h == INVALID_HANDLE_VALUE) {
    opened_file.error_code = ErrorCode::COULD_NOT_OPEN_FILE;
    opened_file.file = { nullptr };
    return opened_file;
  }
  else {
    FileData* file = allocate_single_constructed<FileData>(h);

    opened_file.error_code = ErrorCode::OK;
    opened_file.file = { file };
    return opened_file;
  }
}

FILES::OpenedFile create(const NativePath& name,
                                OPEN_MODE open_mode) {
  OpenedFile opened_file = {};

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
      return {};
    }
  }


  HANDLE h = CreateFileA(name.c_str(), access, share, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE) {
    opened_file.error_code = ErrorCode::COULD_NOT_OPEN_FILE;
    opened_file.file = { nullptr };
    return opened_file;
  }
  else {
    FileData* file = allocate_single_constructed<FileData>(h);

    opened_file.error_code = ErrorCode::OK;
    opened_file.file = { file };
    return opened_file;
  }
}

FILES::OpenedFile replace(const NativePath& name,
                                 OPEN_MODE open_mode) {
  OpenedFile opened_file = {};

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
      return {};
    }
  }

  HANDLE h = CreateFileA(name.c_str(), access, share, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE) {
    opened_file.error_code = ErrorCode::COULD_NOT_OPEN_FILE;
    opened_file.file = { nullptr };
    return opened_file;
  }
  else {
    FileData* file = allocate_single_constructed<FileData>(h);

    opened_file.error_code = ErrorCode::OK;
    opened_file.file = { file };
    return opened_file;
  }
}

FILES::ErrorCode create_empty_directory(const NativePath& name) {
    Windows::NativePath path = name;

    BOOL res = CreateDirectoryA(path.c_str(), NULL);
    if(res != 0) return ErrorCode::OK;
    else {
      // Some error - for now just assume okay
      return ErrorCode::OK;
    }
}

bool exist(const NativePath& name) {
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
