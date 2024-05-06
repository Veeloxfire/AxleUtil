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
    u64 value;
    bool valid = deserialize_be<u64>(be_res, value);
    TEST_EQ(true, valid);
    const u64 expected = 0x01'23'45'67'89'ab'cd'efllu;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    u64 value;
    bool valid = deserialize_le<u64>(le_res, value);
    TEST_EQ(true, valid);
    const u64 expected = 0x01'23'45'67'89'ab'cd'efllu;
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    u32 value;
    bool valid = deserialize_be<u32>(be_res, value);
    TEST_EQ(true, valid);
    const u32 expected = 0x01'23'45'67u;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    u32 value;
    bool valid = deserialize_le<u32>(le_res, value);
    TEST_EQ(true, valid);
    const u32 expected = 0x01'23'45'67u;
    TEST_EQ(expected, value);
  }
  
  {
     const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    u16 value;
    bool valid = deserialize_be<u16>(be_res, value);
    TEST_EQ(true, valid);
    const u16 expected = static_cast<u16>(0x01'23);
    TEST_EQ(expected, value);
  }

  {
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    u16 value;
    bool valid = deserialize_le<u16>(le_res, value);
    TEST_EQ(true, valid);
    const u16 expected = static_cast<u16>(0x01'23u);
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    u8 value;
    bool valid = deserialize_be<u8>(be_res, value);
    TEST_EQ(true, valid);
    const u8 expected = static_cast<u8>(0x01u);
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    u8 value;
    bool valid = deserialize_le<u8>(le_res, value);
    TEST_EQ(true, valid);
    const u8 expected = static_cast<u8>(0x01u);
    TEST_EQ(expected, value);
  }
  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    i64 value;
    bool valid = deserialize_be<i64>(be_res, value);
    TEST_EQ(true, valid);
    const i64 expected = 0x01'23'45'67'89'ab'cd'efll;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    i64 value;
    bool valid = deserialize_le<i64>(le_res, value);
    TEST_EQ(true, valid);
    const i64 expected = 0x01'23'45'67'89'ab'cd'efll;
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    i32 value;
    bool valid = deserialize_be<i32>(be_res, value);
    TEST_EQ(true, valid);
    const i32 expected = 0x01'23'45'67;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    i32 value;
    bool valid = deserialize_le<i32>(le_res, value);
    TEST_EQ(true, valid);
    const i32 expected = 0x01'23'45'67;
    TEST_EQ(expected, value);
  }
  
  {
    const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    i16 value;
    bool valid = deserialize_be<i16>(be_res, value);
    TEST_EQ(true, valid);
    const i16 expected = static_cast<i16>(0x01'23);
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    i16 value;
    bool valid = deserialize_le<i16>(le_res, value);
    TEST_EQ(true, valid);
    const i16 expected = static_cast<i16>(0x01'23);
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    i8 value;
    bool valid = deserialize_be<i8>(be_res, value);
    TEST_EQ(true, valid);
    const i8 expected = static_cast<i8>(0x01);
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    i8 value;
    bool valid = deserialize_le<i8>(le_res, value);
    TEST_EQ(true, valid);
    const i8 expected = static_cast<i8>(0x01);
    TEST_EQ(expected, value);
  }
}

TEST_FUNCTION(Deserialize, rvalues) {
  //Should be able to deserialize from an r-value
  
  const u8 bytes[4] {1, 2, 3, 4};
  {
    u32 value;
    bool valid = deserialize_le<u32>(ViewArr<const u8>{bytes, 4}, value);
    TEST_EQ(true, valid);
    const u32 expected_le = 0x04030201u;
    TEST_EQ(expected_le, value);
  }
  {
    u32 value;
    bool valid = deserialize_be<u32>(ViewArr<const u8>{bytes, 4}, value);
    TEST_EQ(true, valid);
    const u32 expected_be = 0x01020304u;
    TEST_EQ(expected_be, value);
  }
}

TEST_FUNCTION(DeserializeForce, ints) {
  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    const u64 value = deserialize_be_force<u64>(be_res);
    const u64 expected = 0x01'23'45'67'89'ab'cd'efllu;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    const u64 value = deserialize_le_force<u64>(le_res);
    const u64 expected = 0x01'23'45'67'89'ab'cd'efllu;
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    const u32 value = deserialize_be_force<u32>(be_res);
    const u32 expected = 0x01'23'45'67u;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    const u32 value = deserialize_le_force<u32>(le_res);
    const u32 expected = 0x01'23'45'67u;
    TEST_EQ(expected, value);
  }
  
  {
     const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    const u16 value = deserialize_be_force<u16>(be_res);
    const u16 expected = static_cast<u16>(0x01'23);
    TEST_EQ(expected, value);
  }

  {
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    const u16 value = deserialize_le_force<u16>(le_res);
    const u16 expected = static_cast<u16>(0x01'23u);
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    const u8 value = deserialize_be_force<u8>(be_res);
    const u8 expected = static_cast<u8>(0x01u);
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    const u8 value = deserialize_le_force<u8>(le_res);
    const u8 expected = static_cast<u8>(0x01u);
    TEST_EQ(expected, value);
  }
  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab,0xcd, 0xef
    };
    static_assert(sizeof(be_res) == 8);
    const i64 value = deserialize_be_force<i64>(be_res);
    const i64 expected = 0x01'23'45'67'89'ab'cd'efll;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 8);
    const i64 value = deserialize_le_force<i64>(le_res);
    const i64 expected = 0x01'23'45'67'89'ab'cd'efll;
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01, 0x23, 0x45, 0x67,
    };
    static_assert(sizeof(be_res) == 4);
    const i32 value = deserialize_be_force<i32>(be_res);
    const i32 expected = 0x01'23'45'67;
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x67, 0x45, 0x23, 0x01
    };
    static_assert(sizeof(le_res) == 4);
    const i32 value = deserialize_le_force<i32>(le_res);
    const i32 expected = 0x01'23'45'67;
    TEST_EQ(expected, value);
  }
  
  {
    const u8 be_res[] {
      0x01, 0x23,
    };
    static_assert(sizeof(be_res) == 2);
    const i16 value = deserialize_be_force<i16>(be_res);
    const i16 expected = static_cast<i16>(0x01'23);
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x23, 0x01
    };
    static_assert(sizeof(le_res) == 2);
    const i16 value = deserialize_le_force<i16>(le_res);
    const i16 expected = static_cast<i16>(0x01'23);
    TEST_EQ(expected, value);
  }

  {
    const u8 be_res[] {
      0x01,
    };
    static_assert(sizeof(be_res) == 1);
    const i8 value = deserialize_be_force<i8>(be_res);
    const i8 expected = static_cast<i8>(0x01);
    TEST_EQ(expected, value);
  }
  {
    const u8 le_res[] {
      0x01
    };
    static_assert(sizeof(le_res) == 1);
    const i8 value = deserialize_le_force<i8>(le_res);
    const i8 expected = static_cast<i8>(0x01);
    TEST_EQ(expected, value);
  }
}

TEST_FUNCTION(DeserializeForce, rvalues) {
  //Should be able to deserialize from an r-value
  
  const u8 bytes[4] {1, 2, 3, 4};
  {
    const u32 value = deserialize_le_force<u32>(ViewArr<const u8>{bytes, 4});
    const u32 expected_le = 0x04030201u;
    TEST_EQ(expected_le, value);
  }
  {
    const u32 value = deserialize_be_force<u32>(ViewArr<const u8>{bytes, 4});
    const u32 expected_be = 0x01020304u;
    TEST_EQ(expected_be, value);
  }
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
  {
    u8 bytes[10] = {0};
    const u8 expected[] = {
      0x12, 0x6a, 0,0,0,0,0, 0x9e, 0xff, 0x02
    };
    Serializer<u8[10], ByteOrder::LittleEndian> ser(bytes);

    serialize_le(ser, 0x00006a12u);
    serialize_le(ser, SerializeZeros{3});
    serialize_le(ser, static_cast<u16>(0xff9e));
    serialize_le(ser, static_cast<u8>(0x02));

    TEST_ARR_EQ(expected, array_size(expected),
        bytes, array_size(bytes));
  }
  {
    u8 bytes[10] = {0};
    const u8 expected[] = {
      0, 0, 0x6a, 0x12, 0,0,0, 0xff, 0x9e, 0x02
    };
    Serializer<u8[10], ByteOrder::BigEndian> ser(bytes);

    serialize_be(ser, 0x00006a12u);
    serialize_be(ser, SerializeZeros{3});
    serialize_be(ser, static_cast<u16>(0xff9e));
    serialize_be(ser, static_cast<u8>(0x02));

    TEST_ARR_EQ(expected, array_size(expected),
        bytes, array_size(bytes));
  }
}

TEST_FUNCTION(Serialize, to_array) {
  {
    const u8 expected[] = {
      0x12, 0x6a, 0,0,0,0,0, 0x9e, 0xff, 0x02
    };
    Array<u8> bytes = {};
    Serializer<Array<u8>, ByteOrder::LittleEndian> ser(bytes);

    serialize_le(ser, 0x00006a12u);
    serialize_le(ser, SerializeZeros{3});
    serialize_le(ser, static_cast<u16>(0xff9e));
    serialize_le(ser, static_cast<u8>(0x02));

    TEST_ARR_EQ(expected, array_size(expected),
                bytes.data, bytes.size);
  }
  {
    const u8 expected[] = {
      0, 0, 0x6a, 0x12, 0,0,0, 0xff, 0x9e, 0x02
    };
    Array<u8> bytes = {};
    Serializer<Array<u8>, ByteOrder::BigEndian> ser(bytes);

    serialize_be(ser, 0x00006a12u);
    serialize_be(ser, SerializeZeros{3});
    serialize_be(ser, static_cast<u16>(0xff9e));
    serialize_be(ser, static_cast<u8>(0x02));

    TEST_ARR_EQ(expected, array_size(expected),
                bytes.data, bytes.size);
  }
}
