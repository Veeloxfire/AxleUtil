#ifndef AXLEUTIL_SERIALIZE_H_
#define AXLEUTIL_SERIALIZE_H_
#include <AxleUtil/safe_lib.h>

namespace Axle {

template<typename T>
struct Serializable {
  template<typename U>
  struct TemplateFalse {
    constexpr static bool VAL = false;
  };

  static_assert(TemplateFalse<T>::VAL, "Attempted to use unspecialized serializable arg");
};

template<typename T>
struct Serializer {
  template<typename U>
  struct TemplateFalse {
    constexpr static bool VAL = false;
  };

  static_assert(TemplateFalse<T>::VAL, "Attempted to use unspecialized serializer");
};

template<typename S, typename T>
constexpr void serialize_le(S& base, const T& val) {
  Serializer<S> ser = {base};
  Serializable<T>::serialize_le(ser, val);
}

template<typename S, typename T>
constexpr void serialize_le(Serializer<S>& ser, const T& val) {
  Serializable<T>::serialize_le(ser, val);
}

template<typename S, typename T>
constexpr void serialize_be(S& base, const T& val) {
  Serializer<S> ser = {base};
  Serializable<T>::serialize_be(ser, val);
}

template<typename S, typename T>
constexpr void serialize_be(Serializer<S>& ser, const T& val) {
  Serializable<T>::serialize_be(ser, val);
}

template<typename S, typename T>
constexpr T deserialize_le(S& base) {
  return Serializable<T>::deserialize_le(base);
}

template<typename S, typename T>
constexpr T deserialize_be(S& base) {
  return Serializable<T>::deserialize_be(base);
}

template<typename S, typename T>
constexpr T deserialize_le(Serializer<S>& base) {
  return Serializable<T>::deserialize_le(base);
}

template<typename S, typename T>
constexpr T deserialize_be(Serializer<S>& base) {
  return Serializable<T>::deserialize_be(base);
}

template<>
struct Serializable<ViewArr<u8>> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const ViewArr<const u8>& in) {
    const ViewArr<u8> bytes = ser.take_bytes(in.size);
    memcpy_ts(bytes, in); 
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const ViewArr<const u8>& in) {
    const ViewArr<u8> bytes = ser.take_bytes(in.size);
    memcpy_ts(bytes, in); 
  }
};

template<>
struct Serializable<ViewArr<const u8>> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const ViewArr<const u8>& in) {
    const ViewArr<u8> bytes = ser.take_bytes(in.size);
    memcpy_ts(bytes, in); 
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const ViewArr<const u8>& in) {
    const ViewArr<u8> bytes = ser.take_bytes(in.size);
    memcpy_ts(bytes, in); 
  }
};

template<>
struct Serializable<u64> {
  constexpr static usize SERIALIZE_SIZE = 8;

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const u64 u) {
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[7] = static_cast<uint8_t>(u >> 56);
    bytes[6] = static_cast<uint8_t>(u >> 48);
    bytes[5] = static_cast<uint8_t>(u >> 40);
    bytes[4] = static_cast<uint8_t>(u >> 32);
    bytes[3] = static_cast<uint8_t>(u >> 24);
    bytes[2] = static_cast<uint8_t>(u >> 16);
    bytes[1] = static_cast<uint8_t>(u >> 8);
    bytes[0] = static_cast<uint8_t>(u);
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u64 u) {
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[0] = static_cast<uint8_t>(u >> 56);
    bytes[1] = static_cast<uint8_t>(u >> 48);
    bytes[2] = static_cast<uint8_t>(u >> 40);
    bytes[3] = static_cast<uint8_t>(u >> 32);
    bytes[4] = static_cast<uint8_t>(u >> 24);
    bytes[5] = static_cast<uint8_t>(u >> 16);
    bytes[6] = static_cast<uint8_t>(u >> 8);
    bytes[7] = static_cast<uint8_t>(u);
  }

  template<typename S>
  static constexpr u64 deserialize_le(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
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
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
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
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[3] = static_cast<uint8_t>(u >> 24);
    bytes[2] = static_cast<uint8_t>(u >> 16);
    bytes[1] = static_cast<uint8_t>(u >> 8);
    bytes[0] = static_cast<uint8_t>(u);
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u32 u) {
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[0] = static_cast<uint8_t>(u >> 24);
    bytes[1] = static_cast<uint8_t>(u >> 16);
    bytes[2] = static_cast<uint8_t>(u >> 8);
    bytes[3] = static_cast<uint8_t>(u);
  }

  template<typename S>
  static constexpr u32 deserialize_le(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    return (static_cast<uint32_t>(bytes[3]) << 24)
      | (static_cast<uint32_t>(bytes[2]) << 16)
      | (static_cast<uint32_t>(bytes[1]) << 8)
      | static_cast<uint32_t>(bytes[0]);
  }

    template<typename S>
  static constexpr u32 deserialize_be(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
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
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[1] = static_cast<uint8_t>(u >> 8);
    bytes[0] = static_cast<uint8_t>(u);
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u16 u) {
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[0] = static_cast<uint8_t>(u >> 8);
    bytes[1] = static_cast<uint8_t>(u);
  }

  template<typename S>
  static constexpr u16 deserialize_le(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    return (static_cast<uint16_t>(bytes[1]) << 8)
      | static_cast<uint16_t>(bytes[0]);
  }
 
  template<typename S>
  static constexpr u16 deserialize_be(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    return (static_cast<uint16_t>(bytes[0]) << 8)
      | static_cast<uint16_t>(bytes[1]);
  }
};

template<>
struct Serializable<u8> {
  constexpr static usize SERIALIZE_SIZE = 1;

  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const u8 u) {
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[0] = u;
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const u8 u) {
    const ViewArr<u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    bytes[0] = u;
  }

  template<typename S>
  static constexpr u8 deserialize_le(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    return bytes[0];
  }
 
  template<typename S>
  static constexpr u8 deserialize_be(Serializer<S>& ser) {
    const ViewArr<const u8> bytes = ser.take_bytes(SERIALIZE_SIZE);
    return bytes[0];
  }
};


template<>
struct Serializer<ViewArr<u8>> {
  ViewArr<u8> view;
  
  constexpr Serializer(const ViewArr<u8>& arr)
    : view(arr)
  {}

  constexpr ViewArr<u8> take_bytes(usize size) {
    ASSERT(view.size >= size);
    const auto res = view_arr(view, 0, size);
    view  = view_arr(view, size, view.size - size);
    return res;
  }
};

template<usize N>
struct Serializer<u8[N]> {
  ViewArr<u8> view;
  
  constexpr Serializer(u8(&arr)[N])
    : view(view_arr(arr))
  {}

  constexpr ViewArr<u8> take_bytes(usize size) {
    ASSERT(view.size >= size);
    const auto res = view_arr(view, 0, size);
    view  = view_arr(view, size, view.size - size);
    return res;
  }
};

struct SerializeZeros {
  usize num;
};

template<>
struct Serializable<SerializeZeros> {
  template<typename S>
  static constexpr void serialize_le(Serializer<S>& ser, const SerializeZeros u) {
    const ViewArr<u8> bytes = ser.take_bytes(u.num);
    for(usize i = 0; i < u.num; ++i) {
      bytes[i] = 0;
    }
  }

  template<typename S>
  static constexpr void serialize_be(Serializer<S>& ser, const SerializeZeros u) {
    const ViewArr<u8> bytes = ser.take_bytes(u.num);
    for(usize i = 0; i < u.num; ++i) {
      bytes[i] = 0;
    }
  }
};
}
#endif
