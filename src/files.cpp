#include <AxleUtil/files.h>
#include <AxleUtil/strings.h>
#include <AxleUtil/os/os_windows_files.h>

#include <shellapi.h>

#ifdef AXLE_TRACING
#include <Tracer/trace.h>
#endif

namespace Axle {
FILES::FileData::FileData(HANDLE h) : handle(h) {
  LARGE_INTEGER li = { {0} };
  GetFileSizeEx(h, &li);

  real_file_size = (usize)li.QuadPart;

  li.QuadPart = 0;
  SetFilePointerEx(h, li, &li, FILE_CURRENT);

  real_file_ptr = (usize)li.QuadPart;

  abstract_file_ptr = real_file_ptr;
  abstract_file_size = real_file_size;

  real_buffer_ptr = 0;
  buffer_size = 0;
  in_sync = true;
}

FILES::OpenedFile FILES::open(const ViewArr<const char>& name,
                              OPEN_MODE open_mode) {
 #ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif

 Windows::NativePath path = name;
  return Windows::FILES::open(path, open_mode);
}

FILES::OpenedFile FILES::create(const ViewArr<const char>& name,
                                OPEN_MODE open_mode) {
 #ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif

 Windows::NativePath path = name;
  return Windows::FILES::create(path, open_mode);
}

FILES::OpenedFile FILES::replace(const ViewArr<const char>& name,
                                 OPEN_MODE open_mode) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif

  Windows::NativePath path = name;
  return Windows::FILES::replace(path, open_mode);
}

FILES::ErrorCode FILES::create_empty_directory(const ViewArr<const char>& name) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif

  Windows::NativePath path = name;
  return Windows::FILES::create_empty_directory(path);  
}

FILES::ErrorCode FILES::delete_full_directory(const ViewArr<const char>& name) {
  char pFrom[MAX_PATH + 2] = {};
  {
    //double null terminated for some reason
    ASSERT(name.size <= MAX_PATH);
    
    usize i = 0;
    for(; i < name.size; ++i) {
      pFrom[i] = name[i];
    }

    pFrom[name.size] = '\0';
    pFrom[name.size + 1] = '\0';
  }

  SHFILEOPSTRUCTA op = {
    NULL,
    FO_DELETE,
    pFrom, NULL,
    FOF_NO_UI,
    0,
    NULL,
    NULL
  };

  int res = SHFileOperationA(&op);
  if(res == 0) return ErrorCode::OK;
  else return ErrorCode::COULD_NOT_DELETE_FILE;
}

bool FILES::exist(const ViewArr<const char>& name) {
  Windows::NativePath path = name;
  return Windows::FILES::exist(path);
}

void FILES::close(FileHandle file) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif

  free_destruct_single<FileData>(file.data);
}

static void real_seek(FILES::FileData* file, usize ptr) {
  if (file->real_file_ptr != ptr) {
    LARGE_INTEGER li = {};
    li.QuadPart = (LONGLONG)ptr - (LONGLONG)file->real_file_ptr;

    SetFilePointerEx(file->handle, li, &li, FILE_CURRENT);

    ASSERT(static_cast<usize>(li.QuadPart) == ptr);
    file->real_file_ptr = ptr;
  }
}

static void force_sync_buffer(FILES::FileData* file) {
  real_seek(file, file->real_buffer_ptr);

  DWORD bytes_written = 0;
  BOOL wrote = WriteFile(file->handle, file->buffer, file->buffer_size, &bytes_written, NULL);

  ASSERT(wrote);
  ASSERT(bytes_written == file->buffer_size);
  file->in_sync = true;
  file->real_file_ptr = file->real_buffer_ptr + file->buffer_size;
  file->real_file_size = file->abstract_file_size;
}

static void sync_buffer(FILES::FileData* file) {
  if (!file->in_sync) {
    force_sync_buffer(file);
  }
}

FILES::FileData::~FileData() noexcept(false) {
  sync_buffer(this);
  CloseHandle(handle);
}

struct BufferRange {
  usize ptr_start;
  u32 buffer_start;
  u32 size;
};

static BufferRange get_loaded_range(FILES::FileData* file, usize ptr, usize num_bytes) {
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


static void small_buffer_read(FILES::FileData* const file, usize abstract_ptr, uint8_t* bytes, size_t num_bytes) {
  ASSERT(num_bytes <= FILES::FileData::BUFFER_SIZE);
  usize space_in_file = file->real_file_size - file->real_buffer_ptr;
  usize can_read_size = FILES::FileData::BUFFER_SIZE > space_in_file ? space_in_file : FILES::FileData::BUFFER_SIZE;

  ASSERT(num_bytes <= can_read_size);

  BOOL read = ReadFile(file->handle, file->buffer, (DWORD)can_read_size, NULL, NULL);
  ASSERT(read);
  file->real_file_ptr += can_read_size;

  file->real_buffer_ptr = abstract_ptr;
  file->buffer_size = (u32)can_read_size;
  memcpy_s(bytes, num_bytes, file->buffer, num_bytes);
}

static void big_buffer_read(FILES::FileData* const file, usize abstract_ptr, uint8_t* bytes, size_t num_bytes) {
  ASSERT(num_bytes > FILES::FileData::BUFFER_SIZE);
  ASSERT(num_bytes <= file->real_file_size - file->real_buffer_ptr);

  BOOL read = ReadFile(file->handle, bytes, (DWORD)num_bytes, NULL, NULL);
  ASSERT(read);
  file->real_file_ptr += num_bytes;

  //Take the back bits
  file->real_buffer_ptr = abstract_ptr + (num_bytes - FILES::FileData::BUFFER_SIZE);
  memcpy_s(file->buffer, FILES::FileData::BUFFER_SIZE, bytes + (num_bytes - FILES::FileData::BUFFER_SIZE), FILES::FileData::BUFFER_SIZE);
  file->buffer_size = FILES::FileData::BUFFER_SIZE;
}

static void generic_buffer_read(FILES::FileData* const file, usize abstract_ptr, uint8_t* bytes, size_t num_bytes) {
  if (num_bytes <= FILES::FileData::BUFFER_SIZE) {
    small_buffer_read(file, abstract_ptr, bytes, num_bytes);
  }
  else {
    big_buffer_read(file, abstract_ptr, bytes, num_bytes);
  }
}

static void write_new_buffer(FILES::FileData* const file, const uint8_t* bytes, size_t num_bytes) {
  BOOL wrote = WriteFile(file->handle, bytes, (DWORD)num_bytes, NULL, NULL);
  ASSERT(wrote);

  file->real_file_ptr += num_bytes;
  if (file->real_file_ptr > file->real_file_size) {
    file->real_file_size = file->real_file_ptr;
  }

  usize size = num_bytes > FILES::FileData::BUFFER_SIZE ? FILES::FileData::BUFFER_SIZE : num_bytes;

  file->real_buffer_ptr = file->abstract_file_ptr + (num_bytes - size);
  file->in_sync = true;
  file->buffer_size = (u32)size;
  memcpy_s(file->buffer, size, bytes + (num_bytes - size), size);
}

static void seek_from_start_internal(FILES::FileData* file, size_t location) {
  ASSERT(location <= file->abstract_file_size);
  file->abstract_file_ptr = location;
}

FILES::ErrorCode FILES::read_to_bytes(FileHandle file_h, uint8_t* bytes, size_t num_bytes) {
  ASSERT(num_bytes <= ULONG_MAX);

  FileData* const file = file_h.data;

  BufferRange range = get_loaded_range(file, file->abstract_file_ptr, num_bytes);

  if (num_bytes > range.size) {
    sync_buffer(file); //always need to sync here

    bool at_buffer_start = range.ptr_start == 0;
    bool at_buffer_end = range.ptr_start + range.size == num_bytes;

    bool straddles = range.size == FileData::BUFFER_SIZE && (!at_buffer_start || !at_buffer_end);

    if (straddles) {
      //straddles the buffer
      //lets just do-over
      big_buffer_read(file, file->abstract_file_ptr, bytes, num_bytes);
    }
    else if (range.ptr_start == 0) {
      memcpy_s(bytes, range.size, file->buffer + range.buffer_start, range.size);
      usize remaining = num_bytes - (usize)range.size;
      generic_buffer_read(file, file->abstract_file_ptr + range.size,
                          bytes + range.size, remaining);
    }
    else {
      ASSERT(range.buffer_start == 0);
      memcpy_s(bytes + range.ptr_start, range.size, file->buffer + range.buffer_start, range.size);
      usize remaining = range.ptr_start;
      generic_buffer_read(file, file->abstract_file_ptr, bytes, remaining);
    }
  }
  else {
    ASSERT(range.size == num_bytes);
    ASSERT(range.ptr_start == 0);
    //all is loaded
    memcpy_s(bytes, num_bytes, file->buffer + range.buffer_start, num_bytes);
  }

  file->abstract_file_ptr += num_bytes;

  return ErrorCode::OK;
}

uint8_t FILES::read_byte(FileHandle file) {
  u8 byte = 0;
  read_to_bytes(file, &byte, 1);
  return byte;
}

size_t FILES::size_of_file(FileHandle file) {
  return file.data->abstract_file_size;
}

OwnedArr<u8> FILES::read_full_file(const ViewArr<const char>& file_name) {
#ifdef AXLE_TRACING
  TRACING_FUNCTION();
#endif

  Windows::NativePath path = file_name;
  return Windows::FILES::read_full_file(path);
}

FILES::ErrorCode FILES::write(FileHandle file_h, const uint8_t* bytes, size_t num_bytes) {
  FileData* const file = file_h.data;

  usize end = file->abstract_file_ptr + num_bytes;
  usize max_buffer_end = file->real_buffer_ptr + FileData::BUFFER_SIZE;

  if (file->real_buffer_ptr <= file->abstract_file_ptr && end <= max_buffer_end) {
    usize start = (file->abstract_file_ptr - file->real_buffer_ptr);

    //write over part of the buffer
    memcpy_s(file->buffer + start, num_bytes, bytes, num_bytes);

    if (file->buffer_size < start + num_bytes) file->buffer_size = (u32)(start + num_bytes);

    file->in_sync = false;
  }
  else {
    //lazy
    sync_buffer(file);
    seek_from_start_internal(file, file->abstract_file_ptr);
    write_new_buffer(file, bytes, num_bytes);
  }

  file->abstract_file_ptr += num_bytes;
  if (file->abstract_file_size < file->abstract_file_ptr) {
    file->abstract_file_size = file->abstract_file_ptr;
  }

  return ErrorCode::OK;
}

FILES::ErrorCode FILES::write_padding_bytes(FileHandle file_h, uint8_t byte, size_t num) {
  //TODO: actual buffered io
  FileData* const file = file_h.data;

  sync_buffer(file);
  seek_from_start_internal(file, file->abstract_file_ptr);

  usize remaining = num;

  while (remaining > FileData::BUFFER_SIZE) {
    memset(file->buffer, byte, FileData::BUFFER_SIZE);
    file->buffer_size = FileData::BUFFER_SIZE;
    file->real_buffer_ptr = file->abstract_file_ptr;
    force_sync_buffer(file);

    remaining -= FileData::BUFFER_SIZE;
  }

  if (num > 0) {
    memset(file->buffer, byte, remaining);
    file->buffer_size = (u32)remaining;
    file->real_buffer_ptr = file->abstract_file_ptr;
    force_sync_buffer(file);
  }

  file->abstract_file_ptr += num;

  return ErrorCode::OK;
}

FILES::ErrorCode FILES::write_aligned_array(FileHandle file, const uint8_t* arr, const size_t size, const size_t align) {
  //Write the data
  write(file, arr, size);

  //Padding
  const size_t size_mod_align = size % align;

  if (size_mod_align != 0) {
    return write_padding_bytes(file, '\0', align - size_mod_align);
  }
  else {
    //TODO: Error Codes from fwrite
    return ErrorCode::OK;
  }
}

FILES::ErrorCode FILES::write_aligned_array(FileHandle file, const Array<uint8_t>& arr, const size_t align) {
  return write_aligned_array(file, arr.data, arr.size, align);
}

void FILES::seek_from_start(FileHandle file, size_t location) {
  return seek_from_start_internal(file.data, location);
}

size_t FILES::get_current_pos(FileHandle file) {
  LARGE_INTEGER li = {};
  SetFilePointerEx(file.data->handle, {}, &li, FILE_CURRENT);

  return (size_t)li.QuadPart;
}

constexpr static bool is_character(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'z');
}

constexpr static bool is_absolute_path(const ViewArr<const char>& r) {
  return (r.size >= 3)
    && is_character(r.data[0])
    && r.data[1] == ':'
    && (r.data[2] == '\\' || r.data[2] == '/');
}

constexpr static bool logical_xor(bool a, bool b) {
  return (a || b) && !(a && b);
}

static void append_single_to_path(Array<ViewArr<const char>>& path, const ViewArr<const char>& dir) {
  if (dir.size == 0) {
    return;
  }

  if (dir.size == 2 && memeq_ts(dir.data, "..", 2) && path.size > 0) {
    //Is "go up a directory"
    //Pop last dir

    ViewArr<const char>* back_r = path.back();
    if (back_r->size == 2 && memeq_ts(back_r->data, "..", 2)) {
      //Dont pop
      path.insert(dir);
    }
    else if (back_r->size == 1 && back_r->data[0] == '.') {
      path.pop();
      path.insert(dir);
    }
    else {
      path.pop();
    }
  }
  else if (dir.size == 1 && dir.data[0] == '.') {
    //Is "stay in same directory"
    //do nothing
  }
  else {
    //Directory name
    path.insert(dir);
  }
}

static ViewArr<const char> append_path_to_path(Array<ViewArr<const char>>& path, const ViewArr<const char>& dir) {
  usize start = 0;
  usize i = 0;

  while (i < dir.size) {
    if (dir[i] == '/' || dir[i] == '\\') {
      append_single_to_path(path, view_arr(dir, start, i - start));

      i += 1;
      start = i;
    }
    else {
      i += 1;
    }
  }

  return view_arr(dir, start, i - start);
}

static const char* find_dot_in_file_name(const char* start, const char* end) {
  while (start < end) {
    if (*start == '.') return start;

    start += 1;
  }

  return nullptr;
}

OwnedArr<const char> normalize_path(const ViewArr<const char>& path_str) {
  ASSERT(path_str.data != nullptr);
  ASSERT(path_str.size > 0);

  const bool absolute_path = is_absolute_path(path_str);

  Array<ViewArr<const char>> path = {};

  {
    ViewArr<const char> remaining = append_path_to_path(path, path_str);

    if (remaining.size > 0) {
      append_single_to_path(path, remaining);
    }
  }

  Array<char> str = {};

  if (!absolute_path) {
    str.insert('.');
    str.insert('\\');
  }

  {
    auto i = path.begin();
    const auto end = path.end();

    if (i < end) {
      str.concat(*i);
      i += 1;

      while (i < end) {
        str.insert('\\');
        str.concat(*i);
        i += 1;
      }
    }
  }

  return bake_const_arr(std::move(str));
}

OwnedArr<const char> normalize_path(const ViewArr<const char>& path_str,
                                    const ViewArr<const char>& file_str) {
  ASSERT(path_str.data != nullptr);
  ASSERT(path_str.size > 0);

  ASSERT(file_str.data != nullptr);
  ASSERT(file_str.size > 0);

  const bool absolute_path = is_absolute_path(path_str);
  ASSERT(!is_absolute_path(file_str));

  Array<ViewArr<const char>> path = {};

  {
    ViewArr<const char> remaining = append_path_to_path(path, path_str);

    if (remaining.size > 0) {
      append_single_to_path(path, remaining);
    }
  }

  {
    ViewArr<const char> remaining = append_path_to_path(path, file_str);
    if (remaining.size > 0) {
      append_single_to_path(path, remaining);
    }
  }

  Array<char> str = {};

  if (!absolute_path) {
    str.insert('.');
    str.insert('\\');
  }

  {
    auto i = path.begin();
    const auto end = path.end();

    if (i < end) {
      str.concat(*i);
      i += 1;
      while (i < end) {
        str.insert('\\');
        str.concat(*i);
        i += 1;
      }
    }
  }

  return bake_const_arr(std::move(str));
}

AllocFilePath format_file_path(const ViewArr<const char>& path_str,
                               const ViewArr<const char>& file_str,
                               const ViewArr<const char>& extension) {
  ASSERT(path_str.data != nullptr);
  ASSERT(path_str.size > 0);

  ASSERT(file_str.data != nullptr);
  ASSERT(file_str.size > 0);

  const bool absolute_path = is_absolute_path(path_str);
  ASSERT(!is_absolute_path(file_str));

  Array<ViewArr<const char>> path = {};

  {
    ViewArr<const char> remaining = append_path_to_path(path, path_str);
    if (remaining.size > 0) {
      append_single_to_path(path, remaining);
    }
  }


  ViewArr<const char> file_p_info = append_path_to_path(path, file_str);

  const char* dot = find_dot_in_file_name(file_p_info.begin(), file_p_info.end());

  const bool inline_extension = dot != nullptr && (dot + 1) < file_p_info.end();
  const bool has_extension = inline_extension || extension.size > 0;

  //save is the start of the file

  Array<char> str = {};

  if (!absolute_path) {
    str.insert('.');
    str.insert('\\');
  }

  //Directory
  {
    auto i = path.begin();
    const auto end = path.end();

    for (; i < end; i++) {
      str.concat(*i);
      str.insert('\\');
    }
  }

  const usize directory_size = str.size;
  const usize file_name_index = str.size;

  const char* name_end;
  if (inline_extension) {
    name_end = dot;
  }
  else {
    name_end = file_p_info.end();
  }

  //Load the file name
  {
    const size_t len = (name_end - file_p_info.begin());
    str.insert_uninit(len);

    memcpy_ts(str.data + str.size - len, len, file_p_info.data, len);
  }

  const usize file_name_size = str.size - file_name_index;
  usize extension_index = 0;
  usize extension_size = 0;

  if (has_extension) {
    ASSERT(!(inline_extension && extension.size > 0));
    str.insert('.');
    extension_index = str.size;

    //Load the extension if there isn't one already
    {
      const char* e_start;
      const char* e_end;
      if (inline_extension) {
        ASSERT(extension.size == 0);
        e_start = dot + 1;
        e_end = file_p_info.end();
      }
      else {
        e_start = extension.begin();
        e_end = extension.end();
      }

      ASSERT(e_end > e_start);
      const size_t len = (e_end - e_start);
      str.insert_uninit(len);

      memcpy_ts(str.data + str.size - len, len, e_start, len);
    }

    extension_size = str.size - extension_index;
  }

  str.insert('\0');

  auto arr = bake_const_arr(std::move(str));
  arr.size -= 1;

  return {
    std::move(arr),
    directory_size,
    file_name_index,
    file_name_size,
    extension_index,
    extension_size,
  };
}

AllocFilePath format_file_path(const ViewArr<const char>& path_str,
                               const ViewArr<const char>& file_str) {
  return format_file_path(path_str, file_str, {});
}

FileLocation parse_file_location(const AllocFilePath& path,
                                 StringInterner* const strings) {
  FileLocation loc = {};

  loc.full_name = strings->intern(path.raw);
  loc.directory = strings->intern(const_view_arr(path.raw, 0, path.directory_size));
  if (path.extension_size > 0) {
    loc.extension = strings->intern(const_view_arr(path.raw, path.extension_start, path.extension_size));
  }
  else {
    loc.extension = nullptr;
  }

  return loc;
}

FileLocation parse_file_location(const ViewArr<const char>& path_str_in,
                                 const ViewArr<const char>& file_str_in,
                                 StringInterner* const strings) {
  return parse_file_location(format_file_path(path_str_in, file_str_in, {}), strings);
}
}
