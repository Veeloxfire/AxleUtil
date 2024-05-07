#include <AxleTest/unit_tests.h>
#include <AxleUtil/files.h>
#include <AxleUtil/strings.h>

using namespace Axle;

TEST_FUNCTION(Files, normalise_paths) {
  {
    constexpr ViewArr<const char> ARR = lit_view_arr("C:\\hello\\thing2\\thing3");
    OwnedArr str = normalize_path(lit_view_arr("C:\\hello/world/thing/../../thing2/thing3"));
    TEST_STR_EQ(ARR, str);
  }

  {
    constexpr ViewArr<const char> ARR = lit_view_arr(".\\hello\\thing2");
    OwnedArr str = normalize_path(lit_view_arr("hello/world/thing/"), lit_view_arr("../../thing2/thing3/../"));
    TEST_STR_EQ(ARR, str);
  }

  {
    constexpr ViewArr<const char> ARR = lit_view_arr(".\\hello\\thing2");
    OwnedArr str = normalize_path(lit_view_arr("hello/world/thing"), lit_view_arr("../../thing2/thing3/../"));
    TEST_STR_EQ(ARR, str);
  }

  {
    constexpr ViewArr<const char> ARR = lit_view_arr(".\\hello\\thing2");
    OwnedArr str = normalize_path(lit_view_arr("hello/world/thing/"), lit_view_arr("../../thing2/thing3/.."));
    TEST_STR_EQ(ARR, str);
  }

  {
    constexpr ViewArr<const char> ARR = lit_view_arr(".\\hello\\thing2");
    OwnedArr str = normalize_path(lit_view_arr("hello/world/thing"), lit_view_arr("../../thing2/thing3/.."));
    TEST_STR_EQ(ARR, str);
  }

  {
    constexpr ViewArr<const char> ARR = lit_view_arr("C:\\hello\\thing2\\thing3");
    OwnedArr str = normalize_path(lit_view_arr("C:\\hello/world/thing"), lit_view_arr("../../thing2/thing3"));
    TEST_STR_EQ(ARR, str);
  }

  {
#define DIRECTORY ".\\hello\\thing2\\thing3\\"
#define NAME "two"
#define EXTENSION "exe"
    constexpr auto DIRECTORY_ARR = lit_view_arr(DIRECTORY);
    constexpr auto NAME_ARR = lit_view_arr(NAME);
    constexpr auto EXTENSION_ARR = lit_view_arr(EXTENSION);
    constexpr auto ARR = lit_view_arr(DIRECTORY NAME "." EXTENSION);
#undef DIRECTORY
#undef NAME
#undef EXTENSION

    {
      AllocFilePath str = format_file_path(lit_view_arr("hello/world/thing"),
                                           lit_view_arr("../../thing2/thing3/two"),
                                           lit_view_arr("exe"));

      TEST_STR_EQ(ARR, str.raw);
      TEST_STR_EQ(DIRECTORY_ARR, view_arr(str.raw, 0, str.directory_size));
      TEST_STR_EQ(NAME_ARR, view_arr(str.raw, str.file_name_start, str.file_name_size));
      TEST_STR_EQ(EXTENSION_ARR, view_arr(str.raw, str.extension_start, str.extension_size));
    }

    {
      AllocFilePath str = format_file_path(lit_view_arr("hello/world/thing"),
                                           lit_view_arr("../../thing2/thing3/two.exe"));

      TEST_STR_EQ(ARR, str.raw);
      TEST_STR_EQ(DIRECTORY_ARR, view_arr(str.raw, 0, str.directory_size));
      TEST_STR_EQ(NAME_ARR, view_arr(str.raw, str.file_name_start, str.file_name_size));
      TEST_STR_EQ(EXTENSION_ARR, view_arr(str.raw, str.extension_start, str.extension_size));
    }
  }
}


TEST_FUNCTION(Files, parse_file_locations) {
#define DIRECTORY ".\\hello\\thing2\\thing3\\"
#define NAME "two"
#define EXTENSION "exe"
  constexpr auto DIRECTORY_ARR = lit_view_arr(DIRECTORY);
  constexpr auto NAME_ARR = lit_view_arr(NAME);
  constexpr auto EXTENSION_ARR = lit_view_arr(EXTENSION);
  constexpr auto ARR = lit_view_arr(DIRECTORY NAME "." EXTENSION);
#undef DIRECTORY
#undef NAME
#undef EXTENSION

  {
    StringInterner strings = {};

    AllocFilePath str = format_file_path(lit_view_arr("hello/world/thing"),
                                         lit_view_arr("../../thing2/thing3/two"),
                                         lit_view_arr("exe"));

    TEST_STR_EQ(ARR, str.raw);
    TEST_STR_EQ(DIRECTORY_ARR, view_arr(str.raw, 0, str.directory_size));
    TEST_STR_EQ(NAME_ARR, view_arr(str.raw, str.file_name_start, str.file_name_size));
    TEST_STR_EQ(EXTENSION_ARR, view_arr(str.raw, str.extension_start, str.extension_size));

    FileLocation files = parse_file_location(str, &strings);

    const InternString* full = strings.intern(ARR);
    const InternString* dir = strings.intern(DIRECTORY_ARR);
    const InternString* ext = strings.intern(EXTENSION_ARR);

    TEST_EQ(full, files.full_name);
    TEST_EQ(dir, files.directory);
    TEST_EQ(ext, files.extension);
  }

  {
    StringInterner strings = {};

    FileLocation files = parse_file_location(lit_view_arr("hello/world/thing"),
                                             lit_view_arr("../../thing2/thing3/two.exe"), &strings);

    const InternString* full = strings.intern(ARR);
    const InternString* dir = strings.intern(DIRECTORY_ARR);
    const InternString* ext = strings.intern(EXTENSION_ARR);

    TEST_EQ(full, files.full_name);
    TEST_EQ(dir, files.directory);
    TEST_EQ(ext, files.extension);
  }
}

constexpr auto data_file_path = lit_view_arr("./tests/data/file.txt");
constexpr auto full_test_file = lit_view_arr("Hello\r\nWorld\r\n1234\r\n99 99 99\r\n");

TEST_FUNCTION(Files, exists) {
  TEST_EQ(true, FILES::exists(data_file_path));
  TEST_EQ(false, FILES::exists(lit_view_arr("./tests/data2.txt")));
}

TEST_FUNCTION(Files, read_full_file) {
  OwnedArr<const u8> data = FILES::read_full_file(data_file_path);

  TEST_STR_EQ(full_test_file, cast_arr<const char>(view_arr(data)));
}

TEST_FUNCTION(Files, open_read) {
  FILES::OpenedFile f = FILES::open(data_file_path, FILES::OPEN_MODE::READ);
  TEST_EQ(FILES::ErrorCode::OK, f.error_code);

  {
    usize size = FILES::size_of_file(f.file);
    TEST_EQ(full_test_file.size, size);
  }

  {
    u8 byte = FILES::read_byte(f.file);
    TEST_EQ(static_cast<u8>('H'), byte);
  }

  {
    const u32 expected = deserialize_le_force<u32>(cast_arr<const u8>(lit_view_arr("ello")));
    u32 i = 0;
    FILES::ErrorCode error = FILES::read<u32>(f.file, &i, 1);
    TEST_EQ(FILES::ErrorCode::OK, error);
    TEST_EQ(expected, i);
  }

  {
    constexpr auto remaining = lit_view_arr("\r\nWorld\r\n1234\r\n99 99 99\r\n");
    u8 bytes[remaining.size];
    FILES::ErrorCode error = FILES::read_to_bytes(f.file, bytes, array_size(bytes));
    TEST_EQ(FILES::ErrorCode::OK, error);

    TEST_STR_EQ(remaining, cast_arr<const char>(view_arr(bytes)));
  }
}

TEST_FUNCTION(Files, DirItr) {
  const FILES::DirectoryIteratorEnd end = {};
  FILES::DirectoryIterator itr = FILES::directory_iterator(lit_view_arr("./tests/data/"));

  TEST_EQ(true, itr < end);
  
  bool found_dir = false;
  bool found_file = false;

  for(; itr < end; ++itr) {
    FILES::DirectoryElement el = *itr;
    switch(el.type) {
      case FILES::DirectoryElementType::File: {
        TEST_EQ(false, found_file);
        found_file = true;
        TEST_STR_EQ(lit_view_arr("file.txt"), el.name);
        break;
      }
      case FILES::DirectoryElementType::Directory: {
        TEST_EQ(false, found_dir);
        found_dir = true;
        TEST_STR_EQ(lit_view_arr("dir"), el.name);
        break;
      }

      default: {
        INVALID_CODE_PATH("Unsupported/invalid Directory Element type");
      }
    }
  }

  TEST_EQ(false, itr < end);
  TEST_EQ(true, found_dir);
  TEST_EQ(true, found_file);
}
