#include <AxleUtil/stdext/compare.h>

#include <AxleTest/unit_tests.h>

TEST_FUNCTION(stdext_compare, fmt) {
  TEST_STR_EQ("std::strong_ordering::equivalent", Axle::format("{}", std::strong_ordering::equivalent));
  TEST_STR_EQ("std::strong_ordering::less", Axle::format("{}", std::strong_ordering::less));
  TEST_STR_EQ("std::strong_ordering::greater", Axle::format("{}", std::strong_ordering::greater));
}
