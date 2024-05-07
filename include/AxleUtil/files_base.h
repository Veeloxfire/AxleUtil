#ifndef AXLEUTIL_FILES_BASE_H_
#define AXLEUTIL_FILES_BASE_H_

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/formattable.h>

namespace Axle::FILES {
#define FILE_ERROR_CODES_X \
modify(OK)\
modify(COULD_NOT_CREATE_FILE)\
modify(COULD_NOT_OPEN_FILE)\
modify(COULD_NOT_CLOSE_FILE)\
modify(COULD_NOT_DELETE_FILE)\

  enum struct ErrorCode : u8 {
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

  enum struct OPEN_MODE : u8 {
    READ = 'r', WRITE = 'w'
  };

  enum struct DirectoryElementType {
    File, Directory,
  };

  struct DirectoryElement {
    DirectoryElementType type;
    ViewArr<const char> name;
  };

  struct DirectoryIteratorEnd {};

  namespace Base {
  // Need to implemented per file handle
  template<typename T>
  void handle_close(T t);
  template<typename T>
  void handle_seek_from_start(T t, usize size);
  template<typename T>
  usize handle_file_size(T t);
  template<typename T>
  void handle_write(T t, const u8* data, usize size);
  template<typename T>
  void handle_read(T t, u8* data, usize size);


  template<typename F>
  struct FileData {
    static constexpr usize BUFFER_SIZE = 1024;
  
    F file_handle;

    usize real_file_ptr;
    usize real_file_size;

    usize abstract_file_ptr;
    usize abstract_file_size;

    usize real_buffer_ptr;

    bool in_sync;
    u32 buffer_size;
    u8 buffer[BUFFER_SIZE];

    FileData(F f) noexcept;
    ~FileData() noexcept;
  };

  template<typename T>
  void real_seek(FileData<T>* file, usize ptr) {
    if (file->real_file_ptr != ptr) {
      handle_seek_from_start<T>(file->file_handle, ptr);
      file->real_file_ptr = ptr;
    }
  }

  template<typename T>
  void force_sync_buffer(FileData<T>* file) {
    real_seek(file, file->real_buffer_ptr);

    handle_write(file->file_handle, file->buffer, file->buffer_size);

    file->in_sync = true;
    file->real_file_ptr = file->real_buffer_ptr + file->buffer_size;
    file->real_file_size = file->abstract_file_size;
  }

  template<typename T>
  void sync_buffer(FileData<T>* file) {
    if (!file->in_sync) {
      force_sync_buffer(file);
    }
  }

  struct BufferRange {
    usize ptr_start;
    u32 buffer_start;
    u32 size;
  };

  template<typename T>
  static BufferRange get_loaded_range(FileData<T>* file, usize ptr, usize num_bytes) {
    BufferRange range = {};

    if (ptr < file->real_buffer_ptr) {
      if (ptr + num_bytes <= file->real_buffer_ptr) {
        //No overlap
        return {};
      }

      range.ptr_start = file->real_buffer_ptr - ptr;
      range.buffer_start = 0;

      usize total_after = file->real_buffer_ptr - (ptr + num_bytes);
      if (total_after < (usize)file->buffer_size) {
        range.size = (u32)total_after;
      }
      else {
        range.size = file->buffer_size;
      }
    }
    else {
      usize buffer_end_ptr = file->real_buffer_ptr + (usize)file->buffer_size;
      if (buffer_end_ptr <= ptr) {
        //No overlap
        return {};
      }

      range.ptr_start = 0;
      range.buffer_start = (u32)(ptr - file->real_buffer_ptr);

      if (buffer_end_ptr - ptr < num_bytes) {
        range.size = (u32)(buffer_end_ptr - ptr);
      }
      else {
        range.size = (u32)num_bytes;
      }
    }

    return range;
  }

  template<typename T>
  void small_buffer_read(FileData<T>* const file, usize abstract_ptr, uint8_t* bytes, size_t num_bytes) {
    ASSERT(num_bytes <= FileData<T>::BUFFER_SIZE);
    usize space_in_file = file->real_file_size - file->real_buffer_ptr;
    usize can_read_size = FileData<T>::BUFFER_SIZE > space_in_file ? space_in_file : FileData<T>::BUFFER_SIZE;

    ASSERT(num_bytes <= can_read_size);

    handle_read(file->file_handle, file->buffer, can_read_size);
    file->real_file_ptr += can_read_size;

    file->real_buffer_ptr = abstract_ptr;
    file->buffer_size = (u32)can_read_size;
    memcpy_s(bytes, num_bytes, file->buffer, num_bytes);
  }

  template<typename T>
  void big_buffer_read(FileData<T>* const file, usize abstract_ptr, uint8_t* bytes, size_t num_bytes) {
    ASSERT(num_bytes > FileData<T>::BUFFER_SIZE);
    ASSERT(num_bytes <= file->real_file_size - file->real_buffer_ptr);

    handle_read(file->file_handle, bytes, num_bytes);
    file->real_file_ptr += num_bytes;

    //Take the back bits
    file->real_buffer_ptr = abstract_ptr + (num_bytes - FileData<T>::BUFFER_SIZE);
    memcpy_s(file->buffer, FileData<T>::BUFFER_SIZE, bytes + (num_bytes - FileData<T>::BUFFER_SIZE), FileData<T>::BUFFER_SIZE);
    file->buffer_size = FileData<T>::BUFFER_SIZE;
  }

  template<typename T>
  void generic_buffer_read(FileData<T>* const file, usize abstract_ptr, uint8_t* bytes, size_t num_bytes) {
    if (num_bytes <= FileData<T>::BUFFER_SIZE) {
      small_buffer_read(file, abstract_ptr, bytes, num_bytes);
    }
    else {
      big_buffer_read(file, abstract_ptr, bytes, num_bytes);
    }
  }

  template<typename T>
  void write_new_buffer(FileData<T>* const file, const uint8_t* bytes, size_t num_bytes) {
    handle_write(file->file_handle, bytes, num_bytes);

    file->real_file_ptr += num_bytes;
    if (file->real_file_ptr > file->real_file_size) {
      file->real_file_size = file->real_file_ptr;
    }

    usize size = num_bytes > FileData<T>::BUFFER_SIZE ? FileData<T>::BUFFER_SIZE : num_bytes;

    file->real_buffer_ptr = file->abstract_file_ptr + (num_bytes - size);
    file->in_sync = true;
    file->buffer_size = (u32)size;
    memcpy_s(file->buffer, size, bytes + (num_bytes - size), size);
  }

  template<typename T>
  void seek_from_start_internal(FileData<T>* file, size_t location) {
    ASSERT(location <= file->abstract_file_size);
    file->abstract_file_ptr = location;
  }

  template<typename F>
  FileData<F>::FileData(F h) noexcept : file_handle(h) {
    real_file_size = handle_file_size(h);

    handle_seek_from_start(h, 0);
    real_file_ptr = 0;

    abstract_file_ptr = real_file_ptr;
    abstract_file_size = real_file_size;

    real_buffer_ptr = 0;
    buffer_size = 0;
    in_sync = true;
  }

  template<typename T>
  FileData<T>::~FileData() noexcept {
    sync_buffer(this);
    handle_close(this->file_handle);
  }

  template<typename T>
  struct FileHandle {
    T* data = nullptr;
  };

  template<typename T>
  struct ScopedFile : FileHandle<T> {
    static constexpr T* take(T*& t) {
      T* s = t;
      t = nullptr;
      return s;
    }

    constexpr ScopedFile() = default;
    constexpr ScopedFile(const ScopedFile&) = delete;
    constexpr ScopedFile& operator=(const ScopedFile&) = delete;
    
    constexpr void close_this() {
      if(this->data != nullptr) { free_destruct_single<T>(this->data); } 
    }

    constexpr ScopedFile(T*&& d) : FileHandle<T>{take(d)} {}
    constexpr ScopedFile(FileHandle<T>&& h) : FileHandle<T>{take(h.data)} {}
    constexpr ScopedFile(ScopedFile&& sp) 
      : FileHandle<T>{take(sp.data)} 
    {}

    constexpr ScopedFile& operator=(ScopedFile&& sp) {
        if(this == &sp) return *this;

        close_this();
        this->data = take(sp.data);
        return *this;
    }

    constexpr ~ScopedFile() {
      close_this(); 
    }
  };

  template<typename T>
  struct OpenedFile {
    ScopedFile<T> file;
    ErrorCode error_code;
  };



  }
}

namespace Axle::Format {
  template<>
  struct FormatArg<Axle::FILES::DirectoryElementType> {
    using DirectoryElementType = Axle::FILES::DirectoryElementType;
    template<Formatter F>
    constexpr static void load_string(F& res, DirectoryElementType det) {
      switch(det) {
        case DirectoryElementType::File: return res.load_string_lit("DirectoryElementType::Files");
        case DirectoryElementType::Directory: return res.load_string_lit("DirectoryElementType::Directory");
      }
      INVALID_CODE_PATH("Unsupported DirectoryElementType");
    }
  };
}
#endif
