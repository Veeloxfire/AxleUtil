#ifndef AXLEUTIL_SERIALIZE_H_
#define AXLEUTIL_SERIALIZE_H_
#include <AxleUtil/safe_lib.h>
#include <AxleUtil/option.h>

namespace Axle {

template<typename T>
struct Serializable {
  static_assert(DependentFalse<T>::VAL, "Attempted to use unspecialized serializable arg");
};

enum struct ByteOrder {
  LittleEndian, BigEndian
};

template<typename T, ByteOrder Ord>
struct Serializer {
  static_assert(DependentFalse<T>::VAL, "Attempted to use unspecialized serializer");
};

//default implementation copies non-const version
template<typename T, ByteOrder Ord>
struct Serializer<const T, Ord> : Serializer<T, Ord> {};

namespace TypeTests {
  template<typename T>
  struct IsSerializer {
    static constexpr bool VAL = false;
  };

  template<typename T, ByteOrder Ord>
  struct IsSerializer<Serializer<T, Ord>> {
    static constexpr bool VAL = true;
  };

  template<typename T>
  concept NotSerializer = !TypeTests::IsSerializer<T>::VAL;
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_le(const S& base, const T& val) {
  Serializer<S, ByteOrder::LittleEndian> ser(base);
  Serializable<T>::serialize(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_be(const S& base, const T& val) {
  Serializer<S, ByteOrder::BigEndian> ser(base);
  Serializable<T>::serialize(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_le(S& base, const T& val) {
  Serializer<S, ByteOrder::LittleEndian> ser(base);
  Serializable<T>::serialize(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_be(S& base, const T& val) {
  Serializer<S, ByteOrder::BigEndian> ser(base);
  Serializable<T>::serialize(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_le(Serializer<S, ByteOrder::LittleEndian>& ser, const T& val) {
  Serializable<T>::serialize(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr void serialize_be(Serializer<S, ByteOrder::BigEndian>& ser, const T& val) {
  Serializable<T>::serialize(ser, val);
}

template<typename T, TypeTests::NotSerializer S>
constexpr bool deserialize_le(const S& base, T& out) {
  Serializer<const S, ByteOrder::LittleEndian> ser(base);
  return Serializable<T>::deserialize(ser, out);
}

template<typename T, TypeTests::NotSerializer S>
constexpr bool deserialize_be(const S& base, T& out) {
  Serializer<const S, ByteOrder::BigEndian> ser(base);
  return Serializable<T>::deserialize(ser, out);
}

template<typename T, TypeTests::NotSerializer S>
constexpr bool deserialize_le(S& base, T& out) {
  Serializer<S, ByteOrder::LittleEndian> ser(base);
  return Serializable<T>::deserialize(ser, out);
}

template<typename T, TypeTests::NotSerializer S>
constexpr bool deserialize_be(S& base, T& out) {
  Serializer<S, ByteOrder::BigEndian> ser(base);
  return Serializable<T>::deserialize(ser, out);
}

template<typename T, TypeTests::NotSerializer S>
constexpr bool deserialize_le(Serializer<S, ByteOrder::LittleEndian>& base, T& out) {
  return Serializable<T>::deserialize(base, out);
}

template<typename T, TypeTests::NotSerializer S>
constexpr bool deserialize_be(Serializer<S, ByteOrder::BigEndian>& base, T& out) {
  return Serializable<T>::deserialize(base, out);
}

template<>
struct Serializable<ViewArr<u8>> {
  template<typename S, ByteOrder Ord>
  static constexpr void serialize(Serializer<S, Ord>& ser, const ViewArr<u8>& in) {
    ser.write_bytes(in);
  }

  template<typename S, ByteOrder Ord>
  static constexpr bool deserialize(Serializer<S, Ord>& ser, ViewArr<u8>& in) {
    return ser.read_bytes(in);
  }
};

template<>
struct Serializable<ViewArr<const u8>> {
  template<typename S, ByteOrder Ord>
  static constexpr void serialize(Serializer<S, Ord>& ser, const ViewArr<const u8>& in) {
    ser.write_bytes(in);
  }
};

template<>
struct Serializable<u64> {
  constexpr static usize SERIALIZE_SIZE = 8;

  template<typename S>
  static constexpr void serialize(Serializer<S, ByteOrder::LittleEndian>& ser, const u64 u) {
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
  static constexpr void serialize(Serializer<S, ByteOrder::BigEndian>& ser, const u64 u) {
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
  static constexpr bool deserialize(Serializer<S, ByteOrder::LittleEndian>& ser, u64& out) {
    u8 bytes[SERIALIZE_SIZE];
    if(!ser.read_bytes(view_arr(bytes))) return false;
    out = (static_cast<uint64_t>(bytes[7]) << 56)
      | (static_cast<uint64_t>(bytes[6]) << 48)
      | (static_cast<uint64_t>(bytes[5]) << 40)
      | (static_cast<uint64_t>(bytes[4]) << 32)
      | (static_cast<uint64_t>(bytes[3]) << 24)
      | (static_cast<uint64_t>(bytes[2]) << 16)
      | (static_cast<uint64_t>(bytes[1]) << 8)
      | static_cast<uint64_t>(bytes[0]);
    return true;
  }

  template<typename S>
  static constexpr bool deserialize(Serializer<S, ByteOrder::BigEndian>& ser, u64& out) {
    u8 bytes[SERIALIZE_SIZE];
    if(!ser.read_bytes(view_arr(bytes))) return false;
    out = (static_cast<uint64_t>(bytes[0]) << 56)
      | (static_cast<uint64_t>(bytes[1]) << 48)
      | (static_cast<uint64_t>(bytes[2]) << 40)
      | (static_cast<uint64_t>(bytes[3]) << 32)
      | (static_cast<uint64_t>(bytes[4]) << 24)
      | (static_cast<uint64_t>(bytes[5]) << 16)
      | (static_cast<uint64_t>(bytes[6]) << 8)
      | static_cast<uint64_t>(bytes[7]);
    return true;
  }
};

template<>
struct Serializable<u32> {
  constexpr static usize SERIALIZE_SIZE = 4;

  template<typename S>
  static constexpr void serialize(Serializer<S, ByteOrder::LittleEndian>& ser, const u32 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u),
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u >> 16),
      static_cast<uint8_t>(u >> 24),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr void serialize(Serializer<S, ByteOrder::BigEndian>& ser, const u32 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u >> 24),
      static_cast<uint8_t>(u >> 16),
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr bool deserialize(Serializer<S, ByteOrder::LittleEndian>& ser, u32& out) {
    u8 bytes[SERIALIZE_SIZE];
    if(!ser.read_bytes(view_arr(bytes))) return false;
    out = (static_cast<uint32_t>(bytes[3]) << 24)
      | (static_cast<uint32_t>(bytes[2]) << 16)
      | (static_cast<uint32_t>(bytes[1]) << 8)
      | static_cast<uint32_t>(bytes[0]);
    return true;
  }

    template<typename S>
  static constexpr bool deserialize(Serializer<S, ByteOrder::BigEndian>& ser, u32& out) {
    u8 bytes[SERIALIZE_SIZE];
    if(!ser.read_bytes(view_arr(bytes))) return false;
    out = (static_cast<uint32_t>(bytes[0]) << 24)
      | (static_cast<uint32_t>(bytes[1]) << 16)
      | (static_cast<uint32_t>(bytes[2]) << 8)
      | static_cast<uint32_t>(bytes[3]);
    return true;
  }
};

template<>
struct Serializable<u16> {
  constexpr static usize SERIALIZE_SIZE = 2;

  template<typename S>
  static constexpr void serialize(Serializer<S, ByteOrder::LittleEndian>& ser, const u16 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u),
      static_cast<uint8_t>(u >> 8),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr void serialize(Serializer<S, ByteOrder::BigEndian>& ser, const u16 u) {
    const u8 arr[SERIALIZE_SIZE] = {
      static_cast<uint8_t>(u >> 8),
      static_cast<uint8_t>(u),
    };

    ser.write_bytes(view_arr(arr));
  }

  template<typename S>
  static constexpr bool deserialize(Serializer<S, ByteOrder::LittleEndian>& ser, u16& out) {
    u8 bytes[SERIALIZE_SIZE];
    if(!ser.read_bytes(view_arr(bytes))) return false;
    out = static_cast<u16>((bytes[1] << 8) | bytes[0]);
    return true;
  }
 
  template<typename S>
  static constexpr bool deserialize(Serializer<S, ByteOrder::BigEndian>& ser, u16& out) {
    u8 bytes[SERIALIZE_SIZE];
    if(!ser.read_bytes(view_arr(bytes))) return false;
    out = static_cast<u16>((bytes[0] << 8) | bytes[1]);
    return true;
  }
};

template<>
struct Serializable<u8> {
  constexpr static usize SERIALIZE_SIZE = 1;

  template<typename S, ByteOrder Ord>
  static constexpr void serialize(Serializer<S, Ord>& ser, const u8 u) {
    ser.write_bytes({&u, SERIALIZE_SIZE});
  }

  template<typename S, ByteOrder Ord>
  static constexpr bool deserialize(Serializer<S, Ord>& ser, u8& out) {
    return ser.read_bytes({&out, SERIALIZE_SIZE});
  }
};

template<typename F, typename T>
struct Serializable_Convert {
  template<typename S, ByteOrder Ord>
  static constexpr void serialize(Serializer<S, Ord>& ser, const F& f) {
    const T t = static_cast<T>(f);
    Serializable<T>::serialize(ser, t);
  }
  
  template<typename S, ByteOrder Ord>
  static constexpr bool deserialize(Serializer<S, Ord>& ser, F& out) {
    T t_out;
    if(!Serializable<T>::deserialize(ser, t_out)) return false;
    out = static_cast<F>(t_out);
    return true;
  } 
};

template<>
struct Serializable<i64> : Serializable_Convert<i64, u64> {};
template<>
struct Serializable<i32> : Serializable_Convert<i32, u32> {};
template<>
struct Serializable<i16> : Serializable_Convert<i16, u16> {};
template<>
struct Serializable<i8> : Serializable_Convert<i8, u8> {};


template<ByteOrder Ord>
struct Serializer<ViewArr<u8>, Ord> {
  ViewArr<u8> view;
  
  constexpr Serializer(const ViewArr<u8>& arr)
    : view(arr)
  {}

  constexpr bool read_bytes(const ViewArr<u8>& bytes) {
    if(view.size < bytes.size) return false;
    
    const auto res = view_arr(view, 0, bytes.size);
    view  = view_arr(view, bytes.size, view.size - bytes.size);
    memcpy_ts(bytes, res);
    return true;
  }

  constexpr void write_bytes(const ViewArr<const u8>& bytes) {
    ASSERT(view.size >= bytes.size);
    
    const auto res = view_arr(view, 0, bytes.size);
    view  = view_arr(view, bytes.size, view.size - bytes.size);
    memcpy_ts(res, bytes);
  }
};

template<ByteOrder Ord>
struct Serializer<ViewArr<const u8>, Ord> {
  ViewArr<const u8> view;
  
  constexpr Serializer(const ViewArr<const u8>& arr)
    : view(arr)
  {}

  constexpr bool read_bytes(const ViewArr<u8>& bytes) {
    if(view.size < bytes.size) return false;
    
    const auto res = view_arr(view, 0, bytes.size);
    view  = view_arr(view, bytes.size, view.size - bytes.size);
    memcpy_ts(bytes, res);
    return true;
  }
};

template<usize N, ByteOrder Ord>
struct Serializer<u8[N], Ord> {
  Serializer<ViewArr<u8>, Ord> ser;

  constexpr Serializer(u8(&arr)[N])
    : ser(view_arr(arr))
  {}

  constexpr auto read_bytes(const ViewArr<u8>& bytes) {
    return ser.read_bytes(bytes);
  }

  constexpr auto write_bytes(const ViewArr<const u8>& bytes) {
    return ser.write_bytes(bytes); 
  }
};

template<usize N, ByteOrder Ord>
struct Serializer<const u8[N], Ord> {
  Serializer<ViewArr<const u8>, Ord> ser;

  constexpr Serializer(const u8(&arr)[N])
    : ser(view_arr(arr))
  {}

  constexpr auto read_bytes(const ViewArr<u8>& bytes) {
    return ser.read_bytes(bytes);
  }
};

struct SerializeZeros {
  usize num;
};

template<>
struct Serializable<SerializeZeros> {
  static constexpr usize BUFFER_SIZE = 1024;
  static constexpr u8 zeros[BUFFER_SIZE] = {};//zero buffer to write from

  template<typename S, ByteOrder Ord>
  static constexpr void serialize(Serializer<S, Ord>& ser, const SerializeZeros u) {
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
