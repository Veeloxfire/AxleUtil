#include <AxleUtil/safe_lib.h>
#include <AxleUtil/files.h>
#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/os/os_windows_files.h>

#include <AxleUtil/format.h>

#include <AxleTest/ipc.h>
#include <AxleTest/unit_tests.h>

using namespace Axle::Primitives;
using Axle::Windows::FILES::RawFile;

static AxleTest::IPC::Serialize::Report report_fail(const Axle::ViewArr<const char>& message) {
  return AxleTest::IPC::Serialize::Report{
    AxleTest::IPC::ReportType::Failure, message.data, static_cast<u32>(message.size)
  };
}

static AxleTest::IPC::Serialize::Report report_start(const Axle::ViewArr<const char>& message) {
  return AxleTest::IPC::Serialize::Report{
    AxleTest::IPC::ReportType::Start, message.data, static_cast<u32>(message.size)
  };
}

static AxleTest::IPC::Serialize::Report report_success(const Axle::ViewArr<const char>& message) {
  return AxleTest::IPC::Serialize::Report{
    AxleTest::IPC::ReportType::Success, message.data, static_cast<u32>(message.size)
  };
}

static void client_panic_callback(const void* ud, const Axle::ViewArr<const char>& message) {
  const RawFile* rf = reinterpret_cast<const RawFile*>(ud);

  Axle::serialize_le(*rf, report_fail(message));
}

bool AxleTest::IPC::client_main() {
  const Axle::Windows::FILES::RawFile out_handle = { GetStdHandle(STD_OUTPUT_HANDLE) };
  const Axle::Windows::FILES::RawFile in_handle = { GetStdHandle(STD_INPUT_HANDLE) };

  const auto formatted_error = [out_handle](Axle::Format::FormatString fs, const auto& ... args) {
    const Axle::OwnedArr<const char> message = Axle::format(fs, args...);
    Axle::serialize_le(out_handle, report_fail(Axle::view_arr(message)));
  };

  HANDLE con_out = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
  HANDLE con_in = CreateFile("CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

  if(con_out == INVALID_HANDLE_VALUE) {
    formatted_error("Could not connect CONOUT$");
    return false;
  }

  if(con_in == INVALID_HANDLE_VALUE) {
    formatted_error("Could not connect CONIN$");
    return false;
  }

  SetStdHandle(STD_OUTPUT_HANDLE, con_out);
  SetStdHandle(STD_INPUT_HANDLE, con_in);

  Axle::Panic::set_panic_callback(&out_handle, client_panic_callback);

  const auto& unit_tests = AxleTest::unit_tests_ref();
  
  while(true) {
    AxleTest::IPC::MessageHeader header = Axle::deserialize_le<AxleTest::IPC::MessageHeader>(in_handle);
  
    if(header.version != AxleTest::IPC::VERSION) {
      formatted_error("Incompatible IPC version. Found: {}, Expected: {}", header.version, AxleTest::IPC::VERSION);
      return false;
    }

    switch(header.type) {
      case AxleTest::IPC::Type::Query: {
        const u32 size = static_cast<u32>(unit_tests.size);
        u8 arr[Axle::Serializable<u32>::SERIALIZE_SIZE];
        Axle::serialize_le(arr, size);
        Axle::serialize_le(out_handle, AxleTest::IPC::Serialize::Data{ arr, Axle::Serializable<u32>::SERIALIZE_SIZE });
        break;
      }

      case AxleTest::IPC::Type::Execute: {
        AxleTest::TestErrors errors;
        const u32 test_id = Axle::deserialize_le<u32>(in_handle);
        
        if(test_id >= static_cast<u32>(unit_tests.size)) {
          formatted_error("Tried to run test {} / {}", test_id, unit_tests.size);
          return false;
        }

        const auto& test = unit_tests[test_id];
        Axle::serialize_le(out_handle, report_start(test.test_name));

        errors.test_name = test.test_name;
        test.test_func(&errors);
        if(errors.is_panic()) {
          Axle::serialize_le(out_handle, report_fail(Axle::view_arr(errors.first_error)));
        }
        else {
          Axle::serialize_le(out_handle, report_success({}));
        }
        return true;
      }

      case AxleTest::IPC::Type::Data:
      case AxleTest::IPC::Type::Report:
      default: {
        formatted_error("Invalid IPC input type: {}", static_cast<u32>(header.type));
        return false;
      }
    }
  }
}
