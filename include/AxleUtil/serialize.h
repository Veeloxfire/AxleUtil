#ifndef AXLEUTIL_SERIALIZE_H_
#define AXLEUTIL_SERIALIZE_H_
#include <AxleUtil/safe_lib.h>

namespace Axle {

template<typename T>
struct Serializable {
  static_assert(DependentFalse<T>::VAL, "Attempted to use unspecialized serializable arg");
};

};

template<typename T>
struct Serializer {
  static_assert(DependentFalse<T>::VAL, "Attempted to use unspecialized serializer");
};

//default implementation copies non-const version
template<typename T>
struct Serializer<const T> : Serializer<T> {};

namespace TypeTests {
  template<typename T>
  struct IsSerializer {
    static constexpr bool VAL = false;
  };

  template<typename T>
  struct IsSerializer<Serializer<T>> {
    static constexpr bool VAL = true;
  };

  template<typename T>
  concept NotSerializer = !TypeTests::IsSerializer<T>::VAL;
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_le(const S& base, const T& val) {
  Serializer<S> ser(base);
  Serializable<T>::serialize_le(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_be(const S& base, const T& val) {
  Serializer<S> ser(base);
  Serializable<T>::serialize_be(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_le(S& base, const T& val) {
  Serializer<S> ser(base);
  Serializable<T>::serialize_le(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_le(Serializer<S>& ser, const T& val) {
  Serializable<T>::serialize_le(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_be(S& base, const T& val) {
  Serializer<S> ser(base);
  Serializable<T>::serialize_be(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_be(Serializer<S>& ser, const T& val) {
  Serializable<T>::serialize_be(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr T deserialize_le(const S& base) {
  Serializer<const S> ser(base);
  return Serializable<T>::deserialize_le(ser);
}

template<typename T, TypeTests::NotSerializer S>
constexpr T deserialize_be(const S& base) {
  Serializer<const S> ser(base);
  return Serializable<T>::deserialize_be(ser);
}

template<typename T, TypeTests::NotSerializer S>
constexpr T deserialize_le(S& base) {
  Serializer<S> ser(base);
  return Serializable<T>::deserialize_le(ser);
}

template<typename T, TypeTests::NotSerializer S>
constexpr T deserialize_be(S& base) {
  Serializer<S> ser(base);
  return Serializable<T>::deserialize_be(ser);
}

template<typename T, TypeTests::NotSerializer S>
constexpr T deserialize_le(Serializer<S>& base) {
  return Serializable<T>::deserialize_le(base);
}

template<typename T, TypeTests::NotSerializer S>
constexpr T deserialize_be(Serializer<S>& base) {
  return Serializable<T>::deserialize_be(base);
}

template<>
struct Serializable<ViewArr<u8>> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const ViewArr<const u8>& in) {
    ser.write_bytes(in);
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const ViewArr<const u8>& in) {
    ser.write_bytes(in);
  }
};

template<>
struct Serializable<ViewArr<const u8>> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const ViewArr<const u8>& in) {
    ser.write_bytes(in);
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const ViewArr<const u8>& in) {
    ser.write_bytes(in);
  }
};

template<>
struct Serializable<u64> {
  constexpr static usize SERIALIZE_SIZE = 8;

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const u64 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u),
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u >> 16),
      static_cast<uint8_t>(u >> 24),
      static_cast<uint8_t>(u >> 32),
      static_cast<uint8_t>(u >> 40),
      static_cast<uint8_t>(u >> 48),
      static_cast<uint8_t>(u >> 56),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u64 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u >> 56),
      static_cast<uint8_t>(u >> 48),
      static_cast<uint8_t>(u >> 40),
      static_cast<uint8_t>(u >> 32),
      static_cast<uint8_t>(u >> 24),
      static_cast<uint8_t>(u >> 16),
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr u64 deserialize_le(Serializer<S>& ser) {
    u8 bytes[SERIALIZE_SIZE];
    ser.read_bytes(view_arr(bytes));
    return (static_cast<uint64_t>(bytes[7]) << 56)
      | (static_cast<uint64_t>(bytes[6]) << 48)
      | (static_cast<uint64_t>(bytes[5]) << 40)
      | (static_cast<uint64_t>(bytes[4]) << 32)
      | (static_cast<uint64_t>(bytes[3]) << 24)
      | (static_cast<uint64_t>(bytes[2]) << 16)
      | (static_cast<uint64_t>(bytes[1]) << 8)
      | static_cast<uint64_t>(bytes[0]);
  }

  template<typename S>
  static constexpr u64 deserialize_be(Serializer<S>& ser) {
    u8 bytes[SERIALIZE_SIZE];
    ser.read_bytes(view_arr(bytes));
    return (static_cast<uint64_t>(bytes[0]) << 56)
      | (static_cast<uint64_t>(bytes[1]) << 48)
      | (static_cast<uint64_t>(bytes[2]) << 40)
      | (static_cast<uint64_t>(bytes[3]) << 32)
      | (static_cast<uint64_t>(bytes[4]) << 24)
      | (static_cast<uint64_t>(bytes[5]) << 16)
      | (static_cast<uint64_t>(bytes[6]) << 8)
      | static_cast<uint64_t>(bytes[7]);
  }
};

template<>
struct Serializable<u32> {
  constexpr static usize SERIALIZE_SIZE = 4;

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const u32 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u),
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u >> 16),
      static_cast<uint8_t>(u >> 24),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u32 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u >> 24),
      static_cast<uint8_t>(u >> 16),
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr u32 deserialize_le(Serializer<S>& ser) {
    u8 bytes[SERIALIZE_SIZE];
    ser.read_bytes(view_arr(bytes));
    return (static_cast<uint32_t>(bytes[3]) << 24)
      | (static_cast<uint32_t>(bytes[2]) << 16)
      | (static_cast<uint32_t>(bytes[1]) << 8)
      | static_cast<uint32_t>(bytes[0]);
  }

    template<typename S>
  static constexpr u32 deserialize_be(Serializer<S>& ser) {
    u8 bytes[SERIALIZE_SIZE];
    ser.read_bytes(view_arr(bytes));
    return (static_cast<uint32_t>(bytes[0]) << 24)
      | (static_cast<uint32_t>(bytes[1]) << 16)
      | (static_cast<uint32_t>(bytes[2]) << 8)
      | static_cast<uint32_t>(bytes[3]);
  }
};

template<>
struct Serializable<u16> {
  constexpr static usize SERIALIZE_SIZE = 2;

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const u16 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u),
      static_cast<uint8_t>(u >> 8),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u16 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr u16 deserialize_le(Serializer<S>& ser) {
    u8 bytes[SERIALIZE_SIZE];
    ser.read_bytes(view_arr(bytes));
    return (static_cast<uint16_t>(bytes[1]) << 8)
      | static_cast<uint16_t>(bytes[0]);
  }
 
  template<typename S>
  static constexpr u16 deserialize_be(Serializer<S>& ser) {
    u8 bytes[SERIALIZE_SIZE];
    ser.read_bytes(view_arr(bytes));
    return (static_cast<uint16_t>(bytes[0]) << 8)
      | static_cast<uint16_t>(bytes[1]);
  }
};

template<>
struct Serializable<u8> {
  constexpr static usize SERIALIZE_SIZE = 1;

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const u8 u) {
    ser.write_bytes({&u, SERIALIZE_SIZE});
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u8 u) {
    ser.write_bytes({&u, SERIALIZE_SIZE});
  }

  template<typename S>
  static constexpr u8 deserialize_le(Serializer<S>& ser) {
    u8 u;
    ser.read_bytes({&u, SERIALIZE_SIZE});
    return u;
  }
 
  template<typename S>
  static constexpr u8 deserialize_be(Serializer<S>& ser) {
    u8 u;
    ser.read_bytes({&u, SERIALIZE_SIZE});
    return u;
  }
};

template<>
struct Serializable<i64> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const i64 u) {
    Serializable<u64>::serialize_le(ser, static_cast<u64>(u));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const i64 u) {
    Serializable<u64>::serialize_be(ser, static_cast<u64>(u));
  }
  
  template<typename S>
  static constexpr i64 deserialize_le(Serializer<S>& ser) {
    return static_cast<i64>(Serializable<u64>::deserialize_le(ser));
  }
 
  template<typename S>
  static constexpr i64 deserialize_be(Serializer<S>& ser) {
    return static_cast<i64>(Serializable<u64>::deserialize_be(ser));
  }
};

template<>
struct Serializable<i32> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const i32 u) {
    Serializable<u32>::serialize_le(ser, static_cast<u32>(u));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const i32 u) {
    Serializable<u32>::serialize_be(ser, static_cast<u32>(u));
  }
  
  template<typename S>
  static constexpr i32 deserialize_le(Serializer<S>& ser) {
    return static_cast<i32>(Serializable<u32>::deserialize_le(ser));
  }
 
  template<typename S>
  static constexpr i32 deserialize_be(Serializer<S>& ser) {
    return static_cast<i32>(Serializable<u32>::deserialize_be(ser));
  }
};

template<>
struct Serializable<i16> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const i16 u) {
    Serializable<u16>::serialize_le(ser, static_cast<u16>(u));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const i16 u) {
    Serializable<u16>::serialize_be(ser, static_cast<u16>(u));
  }
  
  template<typename S>
  static constexpr i16 deserialize_le(Serializer<S>& ser) {
    return static_cast<i16>(Serializable<u16>::deserialize_le(ser));
  }
 
  template<typename S>
  static constexpr i16 deserialize_be(Serializer<S>& ser) {
    return static_cast<i16>(Serializable<u16>::deserialize_be(ser));
  }
};

template<>
struct Serializable<i8> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const i8 u) {
    Serializable<u8>::serialize_le(ser, static_cast<u8>(u));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const i8 u) {
    Serializable<u8>::serialize_be(ser, static_cast<u8>(u));
  }
  
  template<typename S>
  static constexpr i8 deserialize_le(Serializer<S>& ser) {
    return static_cast<i8>(Serializable<u8>::deserialize_le(ser));
  }
 
  template<typename S>
  static constexpr i8 deserialize_be(Serializer<S>& ser) {
    return static_cast<i8>(Serializable<u8>::deserialize_be(ser));
  }
};

template<>
struct Serializer<ViewArr<u8>> {
  ViewArr<u8> view;
  
  constexpr Serializer(const ViewArr<u8>& arr)
    : view(arr)
  {}

  constexpr void read_bytes(const ViewArr<u8>& bytes) {
    ASSERT(view.size >= bytes.size);
    
    const auto res = view_arr(view, 0, bytes.size);
    view  = view_arr(view, bytes.size, view.size - bytes.size);
    memcpy_ts(bytes, res);
  }

  constexpr void write_bytes(const ViewArr<const u8>& bytes) {
    ASSERT(view.size >= bytes.size);
    
    const auto res = view_arr(view, 0, bytes.size);
    view  = view_arr(view, bytes.size, view.size - bytes.size);
    memcpy_ts(res, bytes);
  }
};

template<>
struct Serializer<ViewArr<const u8>> {
  ViewArr<const u8> view;
  
  constexpr Serializer(const ViewArr<const u8>& arr)
    : view(arr)
  {}

  constexpr void read_bytes(const ViewArr<u8>& bytes) {
    ASSERT(view.size >= bytes.size);
    
    const auto res = view_arr(view, 0, bytes.size);
    view  = view_arr(view, bytes.size, view.size - bytes.size);
    memcpy_ts(bytes, res);
  }
};

template<usize N>
struct Serializer<u8[N]> {
  Serializer<ViewArr<u8>> ser;

  constexpr Serializer(u8(&arr)[N])
    : ser(view_arr(arr))
  {}

  constexpr void read_bytes(const ViewArr<u8>& bytes) {
    ser.read_bytes(bytes);
  }

  constexpr void write_bytes(const ViewArr<const u8>& bytes) {
    ser.write_bytes(bytes); 
  }
};

template<usize N>
struct Serializer<const u8[N]> {
  Serializer<ViewArr<const u8>> ser;

  constexpr Serializer(const u8(&arr)[N])
    : ser(view_arr(arr))
  {}

  constexpr void read_bytes(const ViewArr<u8>& bytes) {
    ser.read_bytes(bytes);
  }
};

struct SerializeZeros {
  usize num;
};

template<>
struct Serializable<SerializeZeros> {
  static constexpr usize BUFFER_SIZE = 1024;
  static constexpr u8 zeros[BUFFER_SIZE] = {};//zero buffer to write from

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const SerializeZeros u) {
    const usize count = u.num / BUFFER_SIZE;
    const usize extra = u.num % BUFFER_SIZE;
    for(usize i = 0; i < count; ++i) {
      ser.write_bytes(const_view_arr(zeros));
    }
    ser.write_bytes(const_view_arr(zeros, 0, extra));
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const SerializeZeros u) {
    const usize count = u.num / BUFFER_SIZE;
    const usize extra = u.num % BUFFER_SIZE;
    for(usize i = 0; i < count; ++i) {
      ser.write_bytes(const_view_arr(zeros));
    }
    ser.write_bytes(const_view_arr(zeros, 0, extra));
  }
};
}
#endif
