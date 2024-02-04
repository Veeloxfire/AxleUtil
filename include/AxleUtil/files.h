#ifndef AXLEUTIL_FILES_H_
#define AXLEUTIL_FILES_H_

#include <AxleUtil/utility.h>
#include <AxleUtil/formattable.h>
namespace Axle {
namespace FILES {
#define FILE_ERROR_CODES_X \
modify(OK)\
modify(COULD_NOT_CREATE_FILE)\
modify(COULD_NOT_OPEN_FILE)\
modify(COULD_NOT_CLOSE_FILE)\
modify(COULD_NOT_DELETE_FILE)\

  enum struct ErrorCode : uint8_t {
  #define modify(NAME) NAME,
    FILE_ERROR_CODES_X
  #undef modify
  };

  namespace ErrorCodeString {
  #define modify(NAME) inline constexpr char NAME[] = #NAME;
    FILE_ERROR_CODES_X
  #undef modify
  }

  constexpr ViewArr<const char> error_code_string(const ErrorCode code) {
    switch (code) {
  #define modify(NAME) case ErrorCode :: NAME :\
return lit_view_arr(ErrorCodeString :: NAME);
      FILE_ERROR_CODES_X
  #undef modify
    }

    return {};
  }

#undef FILE_ERROR_CODES_X

  enum struct OPEN_MODE : uint8_t {
    READ = 'r', WRITE = 'w'
  };

  struct FileData;

  struct FileHandle {
    FileData* data;
  };

  void close(FileHandle file);

  struct ScopedFile : FileHandle {
    constexpr ScopedFile() = default;
    constexpr ScopedFile(const ScopedFile&) = delete;
    constexpr ScopedFile& operator=(const ScopedFile&) = delete;
    
    constexpr void close_this() {
      if(data != nullptr) { close(*this); } 
    }

    constexpr ScopedFile(FileData* d) : FileHandle{d} {}
    constexpr ScopedFile(FileHandle h) : FileHandle(h) {}
    constexpr ScopedFile(ScopedFile&& sp) 
      : FileHandle{std::exchange(sp.data, nullptr)} 
    {}

    constexpr ScopedFile& operator=(ScopedFile&& sp) {
        if(this == &sp) return *this;

        close_this();
        data = std::exchange(sp.data, nullptr);
        return *this;
    }

    constexpr ~ScopedFile() {
      close_this(); 
    }
  };

  struct OpenedFile {
    ScopedFile file;
    ErrorCode error_code;
  };

  OpenedFile open(const ViewArr<const char>& name,
                  OPEN_MODE open_mode);
  OpenedFile create(const ViewArr<const char>& name,
                    OPEN_MODE open_mode);
  OpenedFile replace(const ViewArr<const char>& name,
                     OPEN_MODE open_mode);

  ErrorCode create_empty_directory(const ViewArr<const char>& name);
  ErrorCode delete_full_directory(const ViewArr<const char>& name);

  bool exist(const ViewArr<const char>& name);

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

  ErrorCode write_aligned_array(FileHandle file, const uint8_t* arr, const size_t size, const size_t align);
  ErrorCode write_aligned_array(FileHandle file, const Array<uint8_t>& arr, const size_t align);

  template<typename T>
  ErrorCode write_obj(FileHandle file, const T& t) {
    return write(file, (const uint8_t*)&t, sizeof(T));
  }

  template<usize N>
  ErrorCode write_str(FileHandle file, const char(&str)[N]) {
    return write(file, (const uint8_t*)str, N - 1);
  }

  struct FileFormatter {
    FileHandle handle;
    ErrorCode errors = ErrorCode::OK;

    constexpr bool is_ok() { return errors == ErrorCode::OK; }

    template<usize N>
    void load_string_lit(const char(&str)[N]) {
      if(!is_ok()) return;
      ASSERT(str[N] == '\0');
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
  ErrorCode format_write(FileHandle handle, const Format::FormatString& format, const T& ... ts) {
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
      res.load_string_raw(err_str.data, err_str.size);
    }
  };
}

struct InternString;
struct StringInterner;

struct Directory {
  const InternString* directory;
};

struct FileLocation {
  const InternString* directory;
  const InternString* extension;
  const InternString* full_name;

  constexpr bool operator== (const FileLocation& a) const {
    return full_name == a.full_name;
  }
};

struct AllocFilePath {
  OwnedArr<const char> raw;
  usize directory_size;
  usize file_name_start;
  usize file_name_size;
  usize extension_start;
  usize extension_size;
};

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
