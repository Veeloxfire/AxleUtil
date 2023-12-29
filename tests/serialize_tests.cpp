#include <AxleTest/unit_tests.h>
#include <AxleUtil/serialize.h>

using namespace Axle;

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
