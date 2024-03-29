#include <AxleTest/unit_tests.h>
#include <AxleUtil/serialize.h>

using namespace Axle;

TEST_FUNCTION(Serializable, options) {
  constexpr i32 i = 0x76'54'32'10;
  constexpr const u8 le_res[] { 0x10, 0x32, 0x54, 0x76 };
  constexpr const u8 be_res[] { 0x76, 0x54, 0x32, 0x10 };
  constexpr usize res_size = 4;
  static_assert(sizeof(be_res) == res_size);
  static_assert(sizeof(le_res) == res_size);

  {
    u8 arr[4] = {};

    serialize_le<i32>(arr, i);
    TEST_ARR_EQ(le_res, res_size, arr, 4);

    serialize_be<i32>(arr, i);
    TEST_ARR_EQ(be_res, res_size, arr, 4); 
  }

  {
    u8 arr[4] = {};

    serialize_le<i32>(Axle::view_arr(arr), i);
    TEST_ARR_EQ(le_res, res_size, arr, 4);

    serialize_be<i32>(Axle::view_arr(arr), i);
    TEST_ARR_EQ(be_res, res_size, arr, 4); 
  }
}

TEST_FUNCTION(Serialize, ints) {
  {
    u8 arr[8] = {0};
    serialize_be(arr, 0x01'23'45'67'89'ab'cd'efllu);
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 8); 
  }
  {
    u8 arr[8] = {0};
    serialize_le(arr, 0x01'23'45'67'89'ab'cd'efllu);
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 8); 
  }

  {
    u8 arr[4] = {0};
    serialize_be(arr, 0x01'23'45'67u);
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 4); 
  }
  {
    u8 arr[4] = {0};
    serialize_le(arr, 0x01'23'45'67u);
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 4); 
  }
  
  {
    u8 arr[2] = {0};
    serialize_be(arr, static_cast<u16>(0x01'23u));
    const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 2); 
  }
  {
    u8 arr[2] = {0};
    serialize_le(arr, static_cast<u16>(0x01'23u));
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 2); 
  }

  {
    u8 arr[1] = {0};
    serialize_be(arr, static_cast<u8>(0x01u));
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 1); 
  }
  {
    u8 arr[1] = {0};
    serialize_le(arr, static_cast<u8>(0x01u));
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 1); 
  }
  {
    u8 arr[8] = {0};
    serialize_be(arr, 0x01'23'45'67'89'ab'cd'efll);
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 8); 
  }
  {
    u8 arr[8] = {0};
    serialize_le(arr, 0x01'23'45'67'89'ab'cd'efll);
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 8); 
  }

  {
    u8 arr[4] = {0};
    serialize_be(arr, 0x01'23'45'67);
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 4); 
  }
  {
    u8 arr[4] = {0};
    serialize_le(arr, 0x01'23'45'67);
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 4); 
  }
  
  {
    u8 arr[2] = {0};
    serialize_be(arr, static_cast<i16>(0x01'23));
    const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 2); 
  }
  {
    u8 arr[2] = {0};
    serialize_le(arr, static_cast<i16>(0x01'23));
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 2); 
  }

  {
    u8 arr[1] = {0};
    serialize_be(arr, static_cast<i8>(0x01));
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    TEST_ARR_EQ(be_res, array_size(be_res), arr, 1); 
  }
  {
    u8 arr[1] = {0};
    serialize_le(arr, static_cast<i8>(0x01));
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    TEST_ARR_EQ(le_res, array_size(le_res), arr, 1); 
  }
}

TEST_FUNCTION(Deserialize, ints) {
  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    const u64 v = deserialize_be<u64>(be_res);
    const u64 expected = 0x01'23'45'67'89'ab'cd'efllu;
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    const u64 v = deserialize_le<u64>(le_res);
    const u64 expected = 0x01'23'45'67'89'ab'cd'efllu;
    TEST_EQ(expected, v);
  }

  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    const u32 v = deserialize_be<u32>(be_res);
    const u32 expected = 0x01'23'45'67u;
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    const u32 v = deserialize_le<u32>(le_res);
    const u32 expected = 0x01'23'45'67u;
    TEST_EQ(expected, v);
  }
  
  {
     const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    const u16 v = deserialize_be<u16>(be_res);
    const u16 expected = static_cast<u16>(0x01'23);
    TEST_EQ(expected, v);
  }

  {
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    const u16 v = deserialize_le<u16>(le_res);
    const u16 expected = static_cast<u16>(0x01'23u);
    TEST_EQ(expected, v);
  }

  {
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    const u8 v = deserialize_be<u8>(be_res);
    const u8 expected = static_cast<u8>(0x01u);
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    const u8 v = deserialize_le<u8>(le_res);
    const u8 expected = static_cast<u8>(0x01u);
    TEST_EQ(expected, v);
  }
  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    const i64 v = deserialize_be<i64>(be_res);
    const i64 expected = 0x01'23'45'67'89'ab'cd'efll;
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    const i64 v = deserialize_le<i64>(le_res);
    const i64 expected = 0x01'23'45'67'89'ab'cd'efll;
    TEST_EQ(expected, v);
  }

  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    const i32 v = deserialize_be<i32>(be_res);
    const i32 expected = 0x01'23'45'67;
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    const i32 v = deserialize_le<i32>(le_res);
    const i32 expected = 0x01'23'45'67;
    TEST_EQ(expected, v);
  }
  
  {
    const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    const i16 v = deserialize_be<i16>(be_res);
    const i16 expected = static_cast<i16>(0x01'23);
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    const i16 v = deserialize_le<i16>(le_res);
    const i16 expected = static_cast<i16>(0x01'23);
    TEST_EQ(expected, v);
  }

  {
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    const i8 v = deserialize_be<i8>(be_res);
    const i8 expected = static_cast<i8>(0x01);
    TEST_EQ(expected, v);
  }
  {
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    const i8 v = deserialize_le<i8>(le_res);
    const i8 expected = static_cast<i8>(0x01);
    TEST_EQ(expected, v);
  }
}

TEST_FUNCTION(Deserialize, rvalues) {
  //Should be able to deserialize from an r-value

  const u8 bytes[4] {1, 2, 3, 4};
  u32 u_le = deserialize_le<u32>(ViewArr<const u8>{bytes, 4});
  const u32 expected_le = 0x04030201u;
  TEST_EQ(expected_le, u_le);

  u32 u_be = deserialize_be<u32>(ViewArr<const u8>{bytes, 4});
  const u32 expected_be = 0x01020304u;
  TEST_EQ(expected_be, u_be);
}

TEST_FUNCTION(Serialize, zeros) {
  {
    u8 bytes[] = {1,1,1,1,1};
    const u8 expected[] = {0,0,0,0,0};

    serialize_le(bytes, SerializeZeros{array_size(bytes)});
    TEST_ARR_EQ(expected, array_size(expected),
                bytes, array_size(bytes));
  }
  {
    u8 bytes[] = {1,1,1,1,1};
    const u8 expected[] = {0,0,0,0,0};

    serialize_be(bytes, SerializeZeros{array_size(bytes)});
    TEST_ARR_EQ(expected, array_size(expected),
                bytes, array_size(bytes));
  }

}

TEST_FUNCTION(Serialize, multiple) {
  u8 bytes[10] = {0};
  const u8 expected[] = {
    0x12, 0x6a, 0,0,0,0,0, 0xff, 0x9e, 0x02
  };

  Serializer<u8[10]> ser(bytes);

  serialize_le(ser, 0x00006a12u);
  serialize_le(ser, SerializeZeros{3});
  serialize_be(ser, static_cast<u16>(0xff9e));
  serialize_be(ser, static_cast<u8>(0x02));

  TEST_ARR_EQ(expected, array_size(expected),
      bytes, array_size(bytes));
}

TEST_FUNCTION(Serialize, to_array) {
  const u8 expected[] = {
    0x12, 0x6a, 0,0,0,0,0, 0xff, 0x9e, 0x02
  };

  Array<u8> bytes = {};
  Serializer<Array<u8>> ser(bytes);

  serialize_le(ser, 0x00006a12u);
  serialize_le(ser, SerializeZeros{3});
  serialize_be(ser, static_cast<u16>(0xff9e));
  serialize_be(ser, static_cast<u8>(0x02));

  TEST_ARR_EQ(expected, array_size(expected),
              bytes.data, bytes.size);
}
