#ifndef AXLETEST_IPC_H_
#define AXLETEST_IPC_H_

#include <AxleUtil/serialize.h>

namespace AxleTest::IPC {
  bool server_main(const Axle::ViewArr<const char>& client_exe);
  bool client_main();
  
  /*  AxleTest IPC binary format
   *
   *  - Header -
   *  [Version:32]
   *  [Type:32]
   *
   *  - Type: Data -
   *  [Header:64]
   *  [DataLen:32]
   *  [Data:DataLen]
   *
   *  - Type: Query -
   *  [Header:64]
   * 
   *  - Type: Execute -
   *  [Header:64]
   *  [TestID:32]
   *
   *  - Type: Report -
   *  [Header:64]
   *  [ReportType:32]
   *  [MessageLen:32]
   *  [Message:MessageLen]
   *
   */


  constexpr inline u32 VERSION = 0;

  enum struct Type: u32 {
    Data = 0,
    Query = 1,
    Execute = 2,
    Report = 3,
  };

  enum struct ReportType: u32 {
    Start, Success, Failure
  };

  struct MessageHeader {
    u32 version;
    Type type;
  };


  namespace Serialize {
    struct Data {
      const u8* data;
      u32 size;
    };

    struct Query {};

    struct Execute {
      u32 test_id;
    };

    struct Report {
      ReportType report_type;
      const char* message;
      u32 message_len;
    };
  }
}

namespace Axle {
  template<>
  struct Serializable<AxleTest::IPC::MessageHeader> {
    using Self = AxleTest::IPC::MessageHeader;

    template<typename S>
    static constexpr void serialize_le(Serializer<S>& ser, const Self& u) {
      Serializable<u32>::serialize_le(ser, u.version);
      Serializable<u32>::serialize_le(ser, static_cast<u32>(u.type));
    }

    template<typename S>
    static constexpr void serialize_be(Serializer<S>& ser, const Self& u) {
      Serializable<u32>::serialize_be(ser, u.version);
      Serializable<u32>::serialize_be(ser, static_cast<u32>(u.type));
    }
    
    template<typename S>
    static constexpr Self deserialize_le(Serializer<S>& ser) {
      u32 version = Serializable<u32>::deserialize_le(ser);
      u32 type = Serializable<u32>::deserialize_le(ser);
      return { version, static_cast<AxleTest::IPC::Type>(type) };
    }
   
    template<typename S>
    static constexpr Self deserialize_be(Serializer<S>& ser) {
      u32 version = Serializable<u32>::deserialize_be(ser);
      u32 type = Serializable<u32>::deserialize_be(ser);
      return { version, static_cast<AxleTest::IPC::Type>(type) };
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Data> {
    using Self = AxleTest::IPC::Serialize::Data;

    template<typename S>
    static constexpr void serialize_le(Serializer<S>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_le(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Data});
      Serializable<u32>::serialize_le(ser, u.size);

      ser.write_bytes({u.data, u.size});
    }

    template<typename S>
    static constexpr void serialize_be(Serializer<S>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_be(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Data});
      Serializable<u32>::serialize_be(ser, u.size);
      
      ser.write_bytes({u.data, u.size});
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Query> {
    using Self = AxleTest::IPC::Serialize::Query;

    template<typename S>
    static constexpr void serialize_le(Serializer<S>& ser, const Self&) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_le(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Query});
    }

    template<typename S>
    static constexpr void serialize_be(Serializer<S>& ser, const Self&) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_be(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Query});
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Execute> {
    using Self = AxleTest::IPC::Serialize::Execute;

    template<typename S>
    static constexpr void serialize_le(Serializer<S>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_le(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Execute});

      Serializable<u32>::serialize_le(ser, u.test_id);
    }

    template<typename S>
    static constexpr void serialize_be(Serializer<S>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_be(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Execute});

      Serializable<u32>::serialize_be(ser, u.test_id);
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Report> {
    using Self = AxleTest::IPC::Serialize::Report;

    template<typename S>
    static constexpr void serialize_le(Serializer<S>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_le(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Report});
      Serializable<u32>::serialize_le(ser, static_cast<u32>(u.report_type));
      Serializable<u32>::serialize_le(ser, u.message_len);

      ser.write_bytes({reinterpret_cast<const u8*>(u.message), u.message_len});
    }

    template<typename S>
    static constexpr void serialize_be(Serializer<S>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize_be(ser, {AxleTest::IPC::VERSION, AxleTest::IPC::Type::Report});
      Serializable<u32>::serialize_be(ser, u.report_type);
      Serializable<u32>::serialize_be(ser, u.message_len);

      ser.write_byte({reinterpret_cast<const u8*>(u.message), u.message_len});
    }
  };
}

#endif
