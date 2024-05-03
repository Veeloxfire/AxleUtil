#include <AxleUtil/stdext/vector.h>

#include <AxleTest/unit_tests.h>

TEST_FUNCTION(stdext_vector, view) {
  int arr[] = {1, 2, 3, 4};

  {
    std::vector<int> vec = {1, 2, 3, 4};
    
    Axle::ViewArr<int> v_view = Axle::view_arr(vec);
    TEST_EQ(true, Axle::memeq_ts(Axle::view_arr(arr), v_view));
    
    Axle::ViewArr<const int> v_view_c = Axle::const_view_arr(vec);
    TEST_EQ(true, Axle::memeq_ts(Axle::view_arr(arr), v_view_c));
  }

  {
    const std::vector<int> vec = {1, 2, 3, 4};
    
    Axle::ViewArr<const int> v_view = Axle::view_arr(vec);
    TEST_EQ(true, Axle::memeq_ts(Axle::view_arr(arr), v_view));
    
    Axle::ViewArr<const int> v_view_c = Axle::const_view_arr(vec);
    TEST_EQ(true, Axle::memeq_ts(Axle::view_arr(arr), v_view_c));
  }
}
