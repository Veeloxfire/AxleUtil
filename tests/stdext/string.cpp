#include <AxleUtil/stdext/string.h>

#include <AxleTest/unit_tests.h>

TEST_FUNCTION(stdext_string, view) {
  const auto s1_expected = Axle::lit_view_arr("hello");
  const auto s2_expected = Axle::lit_view_arr("world");
  std::string s1 = "hello";
  const Axle::ViewArr<char> v1 = Axle::view_arr(s1);
  
  const std::string s2 = "world";
  const Axle::ViewArr<const char> v2 = Axle::view_arr(s2);

  TEST_STR_EQ(s1_expected, s1);
  TEST_STR_EQ(s1_expected, v1);
  TEST_STR_EQ(s2_expected, s2);
  TEST_STR_EQ(s2_expected, v2);
}

TEST_FUNCTION(stdext_string, fmt) {
  const std::string hello = "hello";
  const std::string world = "world";
  Axle::OwnedArr<const char> arr = Axle::format("{} {}", hello, world);

  TEST_STR_EQ(Axle::lit_view_arr("hello world"), arr);
}
