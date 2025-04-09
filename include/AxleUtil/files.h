#ifndef AXLEUTIL_FILES_H_
#define AXLEUTIL_FILES_H_

#include <AxleUtil/utility.h>
#include <AxleUtil/formattable.h>

#include <AxleUtil/os/os_windows_files.h>

namespace Axle {
namespace FILES {
  using FileData = Axle::Windows::FILES::FileData;
  using DirectoryIterator = Axle::Windows::FILES::DirectoryIterator;
  using FileHandle = Base::FileHandle<FileData>; 
  using OpenedFile = Base::OpenedFile<FileData>;

  OpenedFile open(const ViewArr<const char>& name,
                  OPEN_MODE open_mode);
  OpenedFile create(const ViewArr<const char>& name,
                    OPEN_MODE open_mode);
  OpenedFile replace(const ViewArr<const char>& name,
                     OPEN_MODE open_mode);

  void close(FileHandle);

  ErrorCode create_empty_directory(const ViewArr<const char>& name);
  ErrorCode delete_full_directory(const ViewArr<const char>& name);

  bool exists(const ViewArr<const char>& name);

  ErrorCode read_to_bytes(FileHandle file, uint8_t* bytes, size_t num_bytes);
  uint8_t read_byte(FileHandle file);

  template<typename T>
  ErrorCode read(FileHandle file, T* ptr, size_t num) {
    return read_to_bytes(file, (uint8_t*)ptr, sizeof(T) * num);
  }

  size_t size_of_file(FileHandle file);
  void seek_from_start(FileHandle file, size_t offset);
  size_t get_current_pos(FileHandle file);

  OwnedArr<u8> read_full_file(const ViewArr<const char>& file_name);

  ErrorCode write(FileHandle file, const uint8_t* arr, size_t length);
  ErrorCode write_padding_bytes(FileHandle file, uint8_t byte, size_t num);

  ErrorCode write_aligned_array(FileHandle file, const u8* arr, const size_t size, const size_t align);
  ErrorCode write_aligned_array(FileHandle file, const ViewArr<const u8>& arr, const size_t align);

  template<typename T>
  ErrorCode write_obj(FileHandle file, const T& t) {
    return write(file, (const uint8_t*)&t, sizeof(T));
  }

  template<typename T>
  ErrorCode write_obj_arr(FileHandle file, const ViewArr<T>& t) {
    return write(file, (const uint8_t*)t.data, t.size * sizeof(T));
  }

  template<usize N>
  ErrorCode write_str(FileHandle file, const char(&str)[N]) {
    return write(file, (const uint8_t*)str, N - 1);
  }

  void flush(FileHandle);

  DirectoryIterator directory_iterator(const ViewArr<const char>& name) noexcept;

  struct FileFormatter {
    FileHandle handle = {};
    ErrorCode errors = ErrorCode::OK;

    constexpr bool is_ok() { return errors == ErrorCode::OK; }

    template<usize N>
    void load_string_lit(const char(&str)[N]) {
      if(!is_ok()) return;
      ASSERT(str[N - 1] == '\0');
      errors = write(handle, reinterpret_cast<const u8*>(str), N - 1);
    }

    template<usize N>
    void load_string_exact(const char(&str)[N]) {
      if(!is_ok()) return;
      errors = write(handle, reinterpret_cast<const u8*>(str), N);
    }

    inline void load_string(const char* str, usize N) {
      if(!is_ok()) return;
      ASSERT(N > 0);
      errors = write(handle, reinterpret_cast<const u8*>(str), N);
    }

    inline void load_char(char c) {
      if(!is_ok()) return;
      ASSERT(c != '\0');
      errors = write(handle, reinterpret_cast<const u8*>(&c), 1);
    }
  };

  template<typename ... T>
  ErrorCode format_write(FileHandle handle, const Format::FormatString<T...>& format, const T& ... ts) {
    FileFormatter result;
    result.handle = handle;

    Format::format_to(result, format, ts...);

    return result.errors;
  }

}

namespace Format {
  template<>
  struct FormatArg<FILES::ErrorCode> {
    template<Formatter F>
    constexpr static void load_string(F& res, FILES::ErrorCode er) {
      ViewArr<const char> err_str = FILES::error_code_string(er);
      res.load_string(err_str.data, err_str.size);
    }
  };
}

template<ByteOrder Ord>
struct Serializer<FILES::FileHandle, Ord> {
  FILES::FileHandle handle;

  constexpr Serializer(FILES::FileHandle h) : handle(h) {}

  inline void read_bytes(const ViewArr<u8>& bytes) {
    FILES::read_to_bytes(handle, bytes.data, bytes.size); 
  }

  inline void write_bytes(const ViewArr<const u8>& bytes) {
    FILES::write(handle, bytes.data, bytes.size); 
  }
};

struct InternString;
struct StringInterner;

struct Directory {
  const InternString* directory = nullptr;
};

struct FileLocation {
  const InternString* directory = nullptr;
  const InternString* extension = nullptr;
  const InternString* full_name = nullptr;

  constexpr bool operator== (const FileLocation& a) const {
    return full_name == a.full_name;
  }
};

struct AllocFilePath {
  OwnedArr<const char> raw = {};
  usize directory_size = 0;
  usize file_name_start = 0;
  usize file_name_size = 0;
  usize extension_start = 0;
  usize extension_size = 0;
};

constexpr bool is_absolute_path(const ViewArr<const char>& r) {
  return Windows::FILES::is_absolute_path(r);
}

AllocFilePath format_file_path(const ViewArr<const char>& path_str,
                               const ViewArr<const char>& file_str,
                               const ViewArr<const char>& extension);

AllocFilePath format_file_path(const ViewArr<const char>& path_str,
                               const ViewArr<const char>& file_str);

OwnedArr<const char> normalize_path(const ViewArr<const char>& base_directory);
OwnedArr<const char> normalize_path(const ViewArr<const char>& current,
                                    const ViewArr<const char>& relative);

FileLocation parse_file_location(const ViewArr<const char>& path,
                                 const ViewArr<const char>& file,
                                 StringInterner* strings);

FileLocation parse_file_location(const AllocFilePath& path,
                                 StringInterner* strings);
}
#endif
