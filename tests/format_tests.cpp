#include <AxleUtil/format.h>

#include <AxleTest/unit_tests.h>
using namespace Axle;

TEST_FUNCTION(Formatters, syntax) {
  const ViewArr<const char> expected = lit_view_arr("hello {} world {}");
  OwnedArr<const char> arr = format("hello {{}} {} {{}}", lit_view_arr("world"));

  TEST_STR_EQ(expected, arr);

}

TEST_FUNCTION(FormatArg, strings) {
  const ViewArr<const char> expected = lit_view_arr("hello world");
  OwnedArr<const char> arr = format("hello {}", lit_view_arr("world"));

  TEST_STR_EQ(expected, arr);
}

TEST_FUNCTION(FormatArg, c_string) {
  const ViewArr<const char> expected = lit_view_arr("hello world");
  OwnedArr<const char> arr = format("hello {}", CString{"world"});

  TEST_STR_EQ(expected, arr);
}

void test_all_valid_signed_ints(AxleTest::TestErrors* test_errors, const ViewArr<const char>& expected, i64 i) {
  if(SCHAR_MAX >= i && i >= SCHAR_MIN) {
    OwnedArr<const char> sc123 = format("{}", (signed char)i);
    TEST_STR_EQ(expected, sc123);
  }

  if (SHRT_MAX >= i && i >= SHRT_MIN) {
    OwnedArr<const char> ss123 = format("{}", (signed short)i);
    TEST_STR_EQ(expected, ss123);
  }

  if (INT_MAX >= i && i >= INT_MIN) {
    OwnedArr<const char> si123 = format("{}", (signed int)i);
    TEST_STR_EQ(expected, si123);
  }

  if (LONG_MAX >= i && i >= LONG_MIN) {
    OwnedArr<const char> sl123 = format("{}", (signed long)i);
    TEST_STR_EQ(expected, sl123);
  }

  {
    OwnedArr<const char> sll123 = format("{}", (signed long long)i);
    TEST_STR_EQ(expected, sll123);
  }
}

void test_all_valid_unsigned_ints(AxleTest::TestErrors* test_errors, const ViewArr<const char>& expected, u64 i) {
  if (UCHAR_MAX >= i) {
    OwnedArr<const char> sc123 = format("{}", (unsigned char)i);
    TEST_STR_EQ(expected, sc123);
  }

  if (USHRT_MAX >= i) {
    OwnedArr<const char> ss123 = format("{}", (unsigned short)i);
    TEST_STR_EQ(expected, ss123);
  }

  if (UINT_MAX >= i) {
    OwnedArr<const char> si123 = format("{}", (unsigned int)i);
    TEST_STR_EQ(expected, si123);
  }

  if (ULONG_MAX >= i) {
    OwnedArr<const char> sl123 = format("{}", (unsigned long)i);
    TEST_STR_EQ(expected, sl123);
  }

  {
    OwnedArr<const char> sll123 = format("{}", (unsigned long long)i);
    TEST_STR_EQ(expected, sll123);
  }
}

TEST_FUNCTION(FormatArg, ints) {
  {
    const ViewArr<const char> expected = lit_view_arr("0");
    test_all_valid_signed_ints(test_errors, expected, 0);
    if (test_errors->is_panic()) return;

    test_all_valid_unsigned_ints(test_errors, expected, 0);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("1");
    test_all_valid_signed_ints(test_errors, expected, 1);
    if (test_errors->is_panic()) return;

    test_all_valid_unsigned_ints(test_errors, expected, 1);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("-1");
    test_all_valid_signed_ints(test_errors, expected, -1);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("123");
    test_all_valid_signed_ints(test_errors, expected, 123);
    if (test_errors->is_panic()) return;

    test_all_valid_unsigned_ints(test_errors, expected, 123);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("-123");
    test_all_valid_signed_ints(test_errors, expected, -123);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("-128");
    test_all_valid_signed_ints(test_errors, expected, -128);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("-32768");
    test_all_valid_signed_ints(test_errors, expected, -32768);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("-32768");
    test_all_valid_signed_ints(test_errors, expected, -32768);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("-2147483648");
    test_all_valid_signed_ints(test_errors, expected, -2147483647 - 1);
    if (test_errors->is_panic()) return;
  }
  
  {
    const ViewArr<const char> expected = lit_view_arr("-9223372036854775808");
    test_all_valid_signed_ints(test_errors, expected, -9223372036854775807i64 - 1);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("127");
    test_all_valid_signed_ints(test_errors, expected, 127);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 127);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("255");
    test_all_valid_signed_ints(test_errors, expected, 255);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 255);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("327687");
    test_all_valid_signed_ints(test_errors, expected, 327687);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 327687);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("65535");
    test_all_valid_signed_ints(test_errors, expected, 65535);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 65535);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("2147483647");
    test_all_valid_signed_ints(test_errors, expected, 2147483647);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 2147483647);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("4294967295");
    test_all_valid_signed_ints(test_errors, expected, 4294967295);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 4294967295);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("9223372036854775807");
    test_all_valid_signed_ints(test_errors, expected, 9223372036854775807i64);
    if (test_errors->is_panic()) return;
    test_all_valid_unsigned_ints(test_errors, expected, 9223372036854775807i64);
    if (test_errors->is_panic()) return;
  }

  {
    const ViewArr<const char> expected = lit_view_arr("18446744073709551615");
    test_all_valid_unsigned_ints(test_errors, expected, 18446744073709551615ui64);
    if (test_errors->is_panic()) return;
  }
}

TEST_FUNCTION(FormatArg, HexInt) {
  {
    const ViewArr<const char> expected = lit_view_arr("0xAB");
    OwnedArr<const char> actual = format("{}", Format::Hex<u8>{0xab});
    TEST_STR_EQ(expected, actual);
  }

  {
    const ViewArr<const char> expected = lit_view_arr("0xAB03");
    OwnedArr<const char> actual = format("{}", Format::Hex<u16>{0xab03});
    TEST_STR_EQ(expected, actual);
  }

  {
    const ViewArr<const char> expected = lit_view_arr("0xF05E9A32");
    OwnedArr<const char> actual = format("{}", Format::Hex<u32>{0xf05e9a32});
    TEST_STR_EQ(expected, actual);
  }

  {
    const ViewArr<const char> expected = lit_view_arr("0xABAB03CDF05E9A32");
    OwnedArr<const char> actual = format("{}", Format::Hex<u64>{0xabab03cdf05e9a32llu});
    TEST_STR_EQ(expected, actual);
  }
}

TEST_FUNCTION(FormatArg, Floats) {
  {
    float f = 0.0f;
    const ViewArr<const char> expected = lit_view_arr("0");
    
    OwnedArr<const char> actual = format("{}", f);
    TEST_STR_EQ(expected, actual);
  }
  {
    float f = -1.23456735e-36f;
    const ViewArr<const char> expected = lit_view_arr("-1.23456735e-36");
      
    OwnedArr<const char> actual = format("{}", f);
    TEST_STR_EQ(expected, actual);
  }
}

TEST_FUNCTION(FormatArg, Double) {
  {
    double d = 0.0;
    const ViewArr<const char> expected = lit_view_arr("0");
    
    OwnedArr<const char> actual = format("{}", d);
    TEST_STR_EQ(expected, actual);
  }
  {
    double d = -1.23456735e-36;
    const ViewArr<const char> expected = lit_view_arr("-1.23456735e-36");
    
    OwnedArr<const char> actual = format("{}", d);
    TEST_STR_EQ(expected, actual);
  }
  {
    double d = -1.2345678901234567e-100;
    const ViewArr<const char> expected
      = lit_view_arr("-1.2345678901234567e-100");
    
    OwnedArr<const char> actual = format("{}", d);
    TEST_STR_EQ(expected, actual);
  }
}

TEST_FUNCTION(Formatters, ArrayFormatter) {
  constexpr ViewArr<const char> expected_final 
      = lit_view_arr("hello worldhello worldhello worldhello world\0");

  //This is required to make sure we actually stop using the local array
  static_assert(sizeof(Format::ArrayFormatter::LocalArr::arr) < expected_final.size);
  Format::ArrayFormatter arrfm = {};

  Format::format_to(arrfm, "hello world");

  {
    const ViewArr<const char> expected = lit_view_arr("hello world");
    
    const auto actual_v = arrfm.view();
    TEST_STR_EQ(expected, actual_v);

    const auto actual_o = arrfm.take();
    TEST_STR_EQ(expected, actual_o);
  }

  Format::format_to(arrfm, "hello world");
  Format::format_to(arrfm, "hello world");
  Format::format_to(arrfm, "hello world");

  arrfm.null_terminate();

  {
    const auto actual_v = arrfm.view();
    TEST_STR_EQ(expected_final, actual_v);

    const auto actual_o = arrfm.take();
    TEST_STR_EQ(expected_final, actual_o);
  }
}

TEST_FUNCTION(Formatters, ViewFormatter) {
  constexpr ViewArr<const char> expected_final
      = lit_view_arr("hello worldhello worldhello worldhello world\0");

  char array[expected_final.size];

  Format::ViewFormatter arrfm = {view_arr(array)};
  TEST_EQ(static_cast<usize>(0), arrfm.view.size);
  TEST_EQ(static_cast<char*>(array), arrfm.view.data);
  TEST_EQ(expected_final.size, arrfm.capacity);

  Format::format_to(arrfm, "hello world");

  {
    const ViewArr<const char> expected = lit_view_arr("hello world");
    
    TEST_STR_EQ(expected, arrfm.view);
  }

  Format::format_to(arrfm, "hello world");
  Format::format_to(arrfm, "hello world");
  Format::format_to(arrfm, "hello world");
  arrfm.null_terminate();

  TEST_EQ(expected_final.size, arrfm.view.size);
  TEST_STR_EQ(expected_final, arrfm.view);
}
