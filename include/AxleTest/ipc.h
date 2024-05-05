#ifndef AXLETEST_IPC_H_
#define AXLETEST_IPC_H_

#include <AxleUtil/serialize.h>
#include <AxleUtil/formattable.h>

namespace AxleTest::IPC { 
  template<typename T>
  struct ContextName {
    static_assert(Axle::DependentFalse<T>::value, "Unspecialized Context Name");
  };

  struct OpaqueContext {
    Axle::ViewArr<const char> name;
    Axle::ViewArr<const u8> data;
  };

  template<typename T>
  OpaqueContext as_context(const T& t) {
    return { ContextName<T>::NAME, Axle::cast_arr<const u8, const T>({&t, 1}) };
  }

  constexpr inline const char PIPE_NAME[] = "\\\\.\\pipe\\AxleTestServer";

  bool server_main(const Axle::ViewArr<const char>& client_exe,
                   const Axle::ViewArr<const OpaqueContext>& contexts,
                   u32 timeout_time_ms);
  bool client_main(const Axle::ViewArr<const char>& runtime_dir);
  
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
   *  - Type: QueryTestInfo -
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

#define TYPE_MOD \
MOD(Data, 0)\
MOD(QueryTestInfo, 1)\
MOD(Execute, 2)\
MOD(Report, 3)\
MOD(QueryContext, 4)

  enum struct Type: u32 {
#define MOD(name, val) name = val,
    TYPE_MOD
#undef MOD
  };

  constexpr Axle::ViewArr<const char> type_as_string(Type t) {
    switch(t) {
#define MOD(name, ...) case Type:: name: return Axle::lit_view_arr(#name);
      TYPE_MOD
#undef MOD
      default: return {};
    }
  }

#undef TYPE_MOD


#define REPORT_TYPE_MOD \
MOD(Success, 0)\
MOD(Failure, 1)\

  enum struct ReportType: u32 {
#define MOD(name, val) name = val,
    REPORT_TYPE_MOD
#undef MOD
  };

  constexpr Axle::ViewArr<const char> report_type_as_string(ReportType t) {
    switch(t) {
#define MOD(name, ...) case ReportType:: name: return Axle::lit_view_arr(#name);
      REPORT_TYPE_MOD
#undef MOD
      default: return {};
    }
  }

  struct MessageHeader {
    u32 version;
    Type type;
  };

  namespace Serialize {
    template<typename T>
    constexpr MessageHeader header() {
      return { VERSION, T::MESSAGE_TYPE };
    }

    struct Data {
      static constexpr Type MESSAGE_TYPE = Type::Data;

      Axle::ViewArr<const u8> data;
    };

    template<typename T>
    struct DataT {
      static constexpr Type MESSAGE_TYPE = Type::Data;

      const T& data;
    };

    struct QueryTestInfo {
      static constexpr Type MESSAGE_TYPE = Type::QueryTestInfo;
    };

    struct Execute {
      static constexpr Type MESSAGE_TYPE = Type::Execute;
      u32 test_id;
    };

    struct Report {
      static constexpr Type MESSAGE_TYPE = Type::Report;
      ReportType report_type;
      Axle::ViewArr<const char> message;
    };

    struct QueryContext {
      static constexpr Type MESSAGE_TYPE = Type::QueryContext;
      const char* context_name;
      u32 countext_name_len;
    };
  }

  namespace Deserialize { 
    struct Data {
      static constexpr Type MESSAGE_TYPE = Type::Data;

      Axle::ViewArr<const u8> data;
    };

    template<typename T>
    struct DataT {
      static constexpr Type MESSAGE_TYPE = Type::Data;

      T& data;
    };
  }
}

namespace Axle::Format {
  template<>
  struct FormatArg<AxleTest::IPC::Type> {
    template<Formatter F>
    constexpr static void load_string(F& res, AxleTest::IPC::Type ty) {
      ViewArr<const char> err_str = AxleTest::IPC::type_as_string(ty);
      res.load_string(err_str.data, err_str.size);
    }
  };

  template<>
  struct FormatArg<AxleTest::IPC::ReportType> {
    template<Formatter F>
    constexpr static void load_string(F& res, AxleTest::IPC::ReportType ty) {
      ViewArr<const char> err_str = AxleTest::IPC::report_type_as_string(ty);
      res.load_string(err_str.data, err_str.size);
    }
  };
}

namespace Axle {
  template<>
  struct Serializable<AxleTest::IPC::Type> : Serializable_Convert<AxleTest::IPC::Type, u32>{};

  template<>
  struct Serializable<AxleTest::IPC::ReportType> : Serializable_Convert<AxleTest::IPC::ReportType, u32>{};

  template<>
  struct Serializable<AxleTest::IPC::MessageHeader> {
    using Self = AxleTest::IPC::MessageHeader;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self& u) {
      Serializable<u32>::serialize(ser, u.version);
      Serializable<AxleTest::IPC::Type>::serialize(ser, u.type);
    }
    
    template<typename S, ByteOrder Ord>
    static constexpr bool deserialize(Serializer<S, Ord>& ser, Self& out) {
      if(!Serializable<u32>::deserialize(ser, out.version)) return false;
      if(!Serializable<AxleTest::IPC::Type>::deserialize(ser, out.type)) return false;
      return true;
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Data> {
    using Self = AxleTest::IPC::Serialize::Data;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize(ser, AxleTest::IPC::Serialize::header<Self>());
      Serializable<u32>::serialize(ser, static_cast<u32>(u.data.size));

      ser.write_bytes(u.data);
    }
  };

  template<typename T>
  struct Serializable<AxleTest::IPC::Serialize::DataT<T>> {
    using Self = AxleTest::IPC::Serialize::DataT<T>;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize(ser, AxleTest::IPC::Serialize::header<Self>());
      Serializable<u32>::serialize(ser, sizeof(u.data));
      Serializable<T>::serialize(ser, u.data);
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Deserialize::Data> {
    using Self = AxleTest::IPC::Deserialize::Data;

    template<typename S, ByteOrder Ord>
    static constexpr bool deserialize(Serializer<S, Ord>& ser, Self& u) {
      AxleTest::IPC::MessageHeader header;
      if(!Serializable<AxleTest::IPC::MessageHeader>::deserialize(ser, header)) return false;
      if(header.version != AxleTest::IPC::VERSION) return false;
      if(header.type != Self::MESSAGE_TYPE) return false;

      u32 size;
      if(!Serializable<u32>::deserialize(ser, size)) return false;
      if(u.data.size != size) return false;

      if(!Serializable<ViewArr<u8>>::deserialize(ser, u.data)) return false;

      return true;
    }
  };

  template<typename T>
  struct Serializable<AxleTest::IPC::Deserialize::DataT<T>> {
    using Self = AxleTest::IPC::Deserialize::DataT<T>;

    template<typename S, ByteOrder Ord>
    static constexpr bool deserialize(Serializer<S, Ord>& ser, Self& u) {
      AxleTest::IPC::MessageHeader header;
      if(!Serializable<AxleTest::IPC::MessageHeader>::deserialize(ser, header)) return false;
      if(header.version != AxleTest::IPC::VERSION) return false;
      if(header.type != Self::MESSAGE_TYPE) return false;

      u32 size;
      if(!Serializable<u32>::deserialize(ser, size)) return false;
      if(sizeof(T) != size) return false;

      if(!Serializable<T>::deserialize(ser, u.data)) return false;

      return true;
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::QueryTestInfo> {
    using Self = AxleTest::IPC::Serialize::QueryTestInfo;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self&) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize(ser, AxleTest::IPC::Serialize::header<Self>());
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Execute> {
    using Self = AxleTest::IPC::Serialize::Execute;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize(ser, AxleTest::IPC::Serialize::header<Self>());

      Serializable<u32>::serialize(ser, u.test_id);
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::Report> {
    using Self = AxleTest::IPC::Serialize::Report;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize(ser, AxleTest::IPC::Serialize::header<Self>());
      Serializable<u32>::serialize(ser, static_cast<u32>(u.report_type));
      Serializable<u32>::serialize(ser, static_cast<u32>(u.message.size));

      ser.write_bytes(cast_arr<const u8>(u.message));
    }
  };

  template<>
  struct Serializable<AxleTest::IPC::Serialize::QueryContext> {
    using Self = AxleTest::IPC::Serialize::QueryContext;

    template<typename S, ByteOrder Ord>
    static constexpr void serialize(Serializer<S, Ord>& ser, const Self& u) {
      Serializable<AxleTest::IPC::MessageHeader>::serialize(ser, AxleTest::IPC::Serialize::header<Self>());
      Serializable<u32>::serialize(ser, u.countext_name_len);

      ser.write_bytes({reinterpret_cast<const u8*>(u.context_name), u.countext_name_len});
    }
  };
}

#endif
