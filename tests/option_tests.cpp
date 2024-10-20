#include <AxleUtil/option.h>

#include <AxleTest/unit_tests.h>

TEST_FUNCTION(Option, basic) {
  Axle::Option<int> opt_int;

  TEST_EQ(false, opt_int.valid);

  opt_int.set_value(1);

  TEST_EQ(true, opt_int.valid);
  TEST_EQ(1, opt_int.value);

  {
    Axle::Option<int> opt_int2 = opt_int;

    TEST_EQ(true, opt_int2.valid);
    TEST_EQ(1, opt_int2.value);
  }

  {
    Axle::Option<int> opt_int2;

    TEST_EQ(false, opt_int2.valid);

    opt_int2 = opt_int;

    TEST_EQ(true, opt_int2.valid);
    TEST_EQ(1, opt_int2.value);
  }

  {
    Axle::Option<int> opt_int2 = std::move(opt_int);

    TEST_EQ(false, opt_int.valid);
    TEST_EQ(true, opt_int2.valid);
    TEST_EQ(1, opt_int2.value);

    Axle::Option<int> opt_int3;

    TEST_EQ(false, opt_int3.valid);

    opt_int3 = std::move(opt_int2);

    TEST_EQ(false, opt_int2.valid);
    TEST_EQ(true, opt_int3.valid);
    TEST_EQ(1, opt_int3.value);

    opt_int3.destroy();
    
    TEST_EQ(false, opt_int3.valid);
  }

  {
    Axle::Option<int> opt_int2 = 2;

    TEST_EQ(true, opt_int2.valid);
    TEST_EQ(2, opt_int2.value);
  }
}
