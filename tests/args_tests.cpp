#include <AxleUtil/args.h>

#include <AxleTest/unit_tests.h>

TEST_FUNCTION(Args, basic) {
  const char* args[] {
    "executable.exe",
    "-arg1=hello",
    "-arg2=h",
  };
  int argc = static_cast<int>(Axle::array_size(args));

  Axle::ViewArr<const char> arg1;
  char arg2;
  {
    const auto arg_list = clArg::ArgsList{static_cast<usize>(argc), args};
    bool res1 = clArg::parse_arg(*test_errors, arg_list, Axle::lit_view_arr("arg1"), arg1);
    if(test_errors->is_panic()) return;
    TEST_EQ(true, res1);

    bool res2 = clArg::parse_arg(*test_errors, arg_list, Axle::lit_view_arr("arg2"), arg2);
    if(test_errors->is_panic()) return;
    TEST_EQ(true, res2);
  }

  Axle::ViewArr<const char> expected_arg1 = Axle::lit_view_arr("hello");
  TEST_STR_EQ(expected_arg1, arg1);
  TEST_EQ(static_cast<char>('h'), arg2);
};

TEST_FUNCTION(Args, opt) {
  const char* args[] {
    "executable.exe",
    "-arg1=hello",
    "-arg2=h",
  };
  int argc = static_cast<int>(Axle::array_size(args));

  Axle::ViewArr<const char> arg1;
  char arg2;

  const char internal_array[] = "foo";
  Axle::ViewArr<const char> arg3 = Axle::lit_view_arr(internal_array);
  {
    const auto arg_list = clArg::ArgsList{static_cast<usize>(argc), args};
    bool res1 = clArg::parse_arg(*test_errors, arg_list, Axle::lit_view_arr("arg1"), arg1);
    if(test_errors->is_panic()) return;
    TEST_EQ(true, res1);

    bool res2 = clArg::parse_arg(*test_errors, arg_list, Axle::lit_view_arr("arg2"), arg2);
    if(test_errors->is_panic()) return;
    TEST_EQ(true, res2);

    // this argument is optiona;
    bool res3 = clArg::parse_opt_arg(*test_errors, arg_list, Axle::lit_view_arr("arg3"), arg3);
    if(test_errors->is_panic()) return;
    TEST_EQ(false, res3);
  }

  Axle::ViewArr<const char> expected_arg1 = Axle::lit_view_arr("hello");
  TEST_STR_EQ(expected_arg1, arg1);
  TEST_EQ(static_cast<char>('h'), arg2);

  // should not have changed
  TEST_EQ(Axle::lit_view_arr(internal_array).data, arg3.data);
  TEST_EQ(Axle::lit_view_arr(internal_array).size, arg3.size);
};

struct ShouldFail {
  bool failed;

  template<typename ... T>
  void report_error(const char*, const T&...) {
    failed = true;
  }
};

TEST_FUNCTION(Args, fail) {
  ShouldFail errors = {}; 

  const auto arg_list = clArg::ArgsList{0, nullptr};
  char c = 'b';
  bool res1 = clArg::parse_arg(errors, arg_list, Axle::lit_view_arr("arg1"), c);
  TEST_EQ(true, errors.failed);
  TEST_EQ(false, res1);
  TEST_EQ(static_cast<char>('b'), c);
}
