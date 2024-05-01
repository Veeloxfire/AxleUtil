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
using Axle::Windows::FILES::RawFile;

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
        
        IPC::OpaqueContext context = {};
        context.name = test.context_name;
        context.data = {};

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
