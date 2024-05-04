#include <AxleUtil/safe_lib.h>
#include <AxleUtil/files.h>
#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/os/os_windows_files.h>

#include <AxleUtil/format.h>

#include <AxleTest/ipc.h>
#include <AxleTest/unit_tests.h>
#include <AxleUtil/option.h>

#ifndef STACKTRACE_ENABLE
#define STACKTRACE_ENABLE
#endif
#include <AxleUtil/stacktrace.h>

using namespace Axle::Primitives;
namespace Windows = Axle::Windows;
using Windows::FILES::RawFile;

Axle::Array<AxleTest::UnitTest> &AxleTest::unit_tests_ref() {
  static Axle::Array<AxleTest::UnitTest> t = {};
  return t;
}

static AxleTest::IPC::Serialize::Report report_fail(const Axle::ViewArr<const char>& message) {
  return AxleTest::IPC::Serialize::Report{
    AxleTest::IPC::ReportType::Failure, message
  };
}

static AxleTest::IPC::Serialize::Report report_success(const Axle::ViewArr<const char>& message) {
  return AxleTest::IPC::Serialize::Report{
    AxleTest::IPC::ReportType::Success, message
  };
}

static void client_panic_callback(const void* ud, const Axle::ViewArr<const char>& message) {
  const RawFile* rf = reinterpret_cast<const RawFile*>(ud);

  Axle::serialize_le(*rf, report_fail(message));
}

bool AxleTest::IPC::client_main() {
  STACKTRACE_FUNCTION();

  Windows::OwnedHandle pipe_handle = CreateFileA(AxleTest::IPC::PIPE_NAME,
      GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

  ASSERT(pipe_handle.is_valid());

  Windows::OwnedHandle wait_event = CreateEventA(NULL, false, false, NULL);
  ASSERT(wait_event.is_valid());


  const Axle::Windows::FILES::TimeoutFile out_handle = {wait_event.h, pipe_handle.h, INFINITE};
  const Axle::Windows::FILES::TimeoutFile& in_handle = out_handle;
  
  const auto formatted_error = [out_handle](Axle::Format::FormatString fs, const auto& ... args) {
    const Axle::OwnedArr<const char> message = Axle::format(fs, args...);
    Axle::serialize_le(out_handle, report_fail(Axle::view_arr(message)));
  };

  Axle::Panic::set_panic_callback(&out_handle, client_panic_callback);

  const auto& unit_tests = AxleTest::unit_tests_ref();
  
  {
    STACKTRACE_SCOPE("Communication layer");

    AxleTest::IPC::MessageHeader header;
    if(!Axle::deserialize_le<AxleTest::IPC::MessageHeader>(in_handle, header)) {
      formatted_error("Invalid read");
      return false;
    }
 
    if(header.version != AxleTest::IPC::VERSION) {
      formatted_error("Incompatible IPC version. Found: {}, Expected: {}", header.version, AxleTest::IPC::VERSION);
      return false;
    }

    switch(header.type) {
      case AxleTest::IPC::Type::QueryTestInfo: {
        STACKTRACE_SCOPE("QueryTestInfo");

        const u32 size = static_cast<u32>(unit_tests.size);
          Axle::serialize_le(out_handle, AxleTest::IPC::Serialize::DataT<u32>{ size });
        {
          u32 strings_size = 0;
          FOR(unit_tests, it) {
            strings_size += static_cast<u32>(it->test_name.size);
            strings_size += static_cast<u32>(it->context_name.size);
          }

          Axle::serialize_le(out_handle, AxleTest::IPC::Serialize::DataT<u32>{ strings_size });
        }

        FOR(unit_tests, it) {
          Axle::serialize_le(out_handle, AxleTest::IPC::Serialize::Data{ Axle::cast_arr<const u8>(it->test_name) });
          Axle::serialize_le(out_handle, AxleTest::IPC::Serialize::Data{ Axle::cast_arr<const u8>(it->context_name) });
        }
        return true;
      }

      case AxleTest::IPC::Type::Execute: {
        STACKTRACE_SCOPE("Execute");

        AxleTest::TestErrors errors;

        u32 test_id;
        if(!Axle::deserialize_le<u32>(in_handle, test_id)) {
          formatted_error("Unexpected read error");
          return false;
        }

        if(test_id >= static_cast<u32>(unit_tests.size)) {
          formatted_error("Tried to run test {} / {}", test_id, unit_tests.size);
          return false;
        }

        const auto& test = unit_tests[test_id];

        Axle::OwnedArr<u8> maybe_context;

        if(test.context_name.data != nullptr) {
          AxleTest::IPC::MessageHeader data_header;
          if(!Axle::deserialize_le<AxleTest::IPC::MessageHeader>(in_handle, data_header)) {
            formatted_error("Unexpected read error");
            return false;
          }

          if(data_header.version != AxleTest::IPC::VERSION || data_header.type != AxleTest::IPC::Type::Data) {
            formatted_error("Unexpected header. Version: {}, Type: {}", data_header.version, data_header.type);
            return false;
          }

          u32 size;
          if(!Axle::deserialize_le<u32>(in_handle, size)) {
            formatted_error("Unexpected read error");
            return false;
          }

          maybe_context = Axle::new_arr<u8>(size);
          Axle::ViewArr<u8> out_data = Axle::view_arr(maybe_context);
          if(!Axle::deserialize_le<Axle::ViewArr<u8>>(in_handle, out_data)) {
            formatted_error("Unexpected read error");
            return false;
          }
        }

        IPC::OpaqueContext context = {};
        context.name = test.context_name;
        context.data = Axle::view_arr(maybe_context);

        errors.test_name = test.test_name;
        {
          // Set the stacktrace so that tests
          // get a stracktrace of just their name
          const Axle::Stacktrace::TraceNode* old_base = Axle::Stacktrace::EXECUTION_TRACE;
          DEFER(old_base) {
            // Reset to the old stacktrace
            Axle::Stacktrace::EXECUTION_TRACE = old_base;
          };

          const Axle::Stacktrace::TraceNode curr_stacktrace = {
            nullptr, test.test_name,
          };
          Axle::Stacktrace::EXECUTION_TRACE = &curr_stacktrace;

          // Run the test finally
          test.test_func(&errors, context);
        }
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
