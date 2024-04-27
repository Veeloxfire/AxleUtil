#include <AxleUtil/io.h>
#include <AxleUtil/safe_lib.h>
#include <AxleUtil/utility.h>
#include <AxleUtil/format.h>

#include <AxleUtil/os/os_windows.h>
#include <AxleUtil/os/os_windows_files.h>
#include <namedpipeapi.h>
#include <processthreadsapi.h>

#include <AxleTest/ipc.h>

namespace LOG = Axle::LOG;
namespace IPC = AxleTest::IPC;
namespace IO = Axle::IO;

using namespace Axle::Primitives;

PROCESS_INFORMATION start_test_executable(const Axle::ViewArr<const char>& self, 
                                          const Axle::ViewArr<const char>& exe,
                                          HANDLE hstdout, HANDLE hstdin) {
  HANDLE save_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE save_stdin = GetStdHandle(STD_INPUT_HANDLE);

  SetStdHandle(STD_OUTPUT_HANDLE, hstdout);
  SetStdHandle(STD_INPUT_HANDLE, hstdin);

  DEFER(&) {
    SetStdHandle(STD_OUTPUT_HANDLE, save_stdout);
    SetStdHandle(STD_INPUT_HANDLE, save_stdin);
  };

  STARTUPINFO startup_info;
  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.cb = sizeof(startup_info);

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  Axle::Windows::NativePath exe_path;
  ASSERT(self.size + exe.size < MAX_PATH);
  Axle::memcpy_ts(Axle::view_arr(exe_path.path, 0, self.size), self);
  Axle::memcpy_ts(Axle::view_arr(exe_path.path, self.size, exe.size), exe);

  BOOL ret = CreateProcessA(exe_path.c_str(), NULL, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &pi);

  if(ret == 0) {
    u32 ec = GetLastError();
    LOG::error("Failed to open process: {}. Error code: {}", exe, ec);
    return { INVALID_HANDLE_VALUE };
  }

  return pi;
}

struct ReportMessage {
  IPC::ReportType type;
  Axle::OwnedArr<const char> message;
};

static bool expect_report(Axle::Windows::FILES::RawFile in_handle, ReportMessage& out) {
  Axle::Serializer<Axle::Windows::FILES::RawFile> ser = in_handle;
  
  const IPC::MessageHeader header = Axle::Serializable<IPC::MessageHeader>::deserialize_le(ser);
  if(header.version != IPC::VERSION) {
    LOG::error("Incompatable IPC version. Expected: {}, Found: {}", IPC::VERSION, header.version);
    return false;
  }

  if(header.type != IPC::Type::Report) {
    LOG::error("Expected Report Type. Found: {}", static_cast<u32>(header.type));
    return false;
  }

  out.type = static_cast<IPC::ReportType>(Axle::Serializable<u32>::deserialize_le(ser));

  const u32 message_len = Axle::Serializable<u32>::deserialize_le(ser);
 
  if(message_len > 0) {
    Axle::OwnedArr<char> m = Axle::new_arr<char>(message_len);
    ser.read_bytes(Axle::cast_arr<u8>(Axle::view_arr(m)));

    out.message = std::move(m);
  }

  return true;
}

bool AxleTest::IPC::server_main(const Axle::ViewArr<const char>& client_exe) {
  HANDLE parent_read = INVALID_HANDLE_VALUE;
  HANDLE child_write = INVALID_HANDLE_VALUE;

  {
    SECURITY_ATTRIBUTES inheritable_pipes;
    inheritable_pipes.nLength = sizeof(inheritable_pipes);
    inheritable_pipes.lpSecurityDescriptor = NULL;
    inheritable_pipes.bInheritHandle = true;    

    if(!CreatePipe(&parent_read, &child_write, &inheritable_pipes, 0)) {
      LOG::error("Failed to create back communication pipes");
      return false;
    }
  }

  ASSERT(parent_read != INVALID_HANDLE_VALUE);
  ASSERT(child_write != INVALID_HANDLE_VALUE);

  DEFER(&) {
    CloseHandle(parent_read);
    CloseHandle(child_write);
  };

  HANDLE parent_write = INVALID_HANDLE_VALUE;
  HANDLE child_read = INVALID_HANDLE_VALUE;
  {
    SECURITY_ATTRIBUTES inheritable_pipes;
    inheritable_pipes.nLength = sizeof(inheritable_pipes);
    inheritable_pipes.lpSecurityDescriptor = NULL;
    inheritable_pipes.bInheritHandle = true;    

    if(!CreatePipe(&child_read, &parent_write, &inheritable_pipes, 0)) {
      LOG::error("Failed to create forwards communication pipes");
      return false;
    }
  }

  DEFER(&) {
    CloseHandle(parent_write);
    CloseHandle(child_read);
  };

  Axle::Windows::NativePath self_dir_holder;
  DWORD self_path_len = GetModuleFileNameA(NULL, self_dir_holder.path, MAX_PATH);
  while(self_path_len > 0) {
    char c = self_dir_holder.path[self_path_len - 1];
    if(c == '\\' || c == '/') break;
    self_dir_holder.path[self_path_len - 1] = '\0';
    self_path_len -= 1;
  }
  const Axle::ViewArr<const char> self_path = { self_dir_holder.path, self_path_len };

  PROCESS_INFORMATION pi = start_test_executable(self_path, client_exe, child_write, child_read);
  
  if(pi.hProcess == INVALID_HANDLE_VALUE) return false;

  DEFER(&) {
    if(pi.hThread != INVALID_HANDLE_VALUE) CloseHandle(pi.hThread);
    if(pi.hProcess != INVALID_HANDLE_VALUE) CloseHandle(pi.hProcess);
  };

  const Axle::Windows::FILES::RawFile out_handle = {parent_write};
  const Axle::Windows::FILES::RawFile in_handle = {parent_read};
  
  Axle::serialize_le(out_handle, IPC::Serialize::Query{});
  u32 count;
  {
    const IPC::MessageHeader header = Axle::deserialize_le<IPC::MessageHeader>(in_handle);
    if(header.version != IPC::VERSION) {
      LOG::error("Incompatable IPC version. Expected: {}, Found: {}", IPC::VERSION, header.version);
      return false;
    }

    switch(header.type) {
      case IPC::Type::Data: {
        const auto data_size = Axle::deserialize_le<u32>(in_handle);
        ASSERT(data_size == 4);
        count = Axle::deserialize_le<u32>(in_handle);
        break;
      }
      case IPC::Type::Query:
      case IPC::Type::Execute:
      case IPC::Type::Report:
      default: {
        LOG::error("Expected Data message. Found: {}", static_cast<u32>(header.type));
        return false;
      }
    }
  }

  struct FailedResult {
    Axle::OwnedArr<const char> test_name;
    Axle::OwnedArr<const char> result_message;
  };

  Axle::Array<FailedResult> failed_arr;

  for(u32 i = 0; i < count; ++i) {
    if(pi.hProcess == INVALID_HANDLE_VALUE) {
      pi = start_test_executable(self_path, client_exe, child_write, child_read);
      if(pi.hProcess == INVALID_HANDLE_VALUE) return false;
    }

    Axle::serialize_le(out_handle, IPC::Serialize::Execute{i});
    
    ReportMessage start_message;
    if(!expect_report(in_handle, start_message)) return false;
    
    if(start_message.type != IPC::ReportType::Start) {
      LOG::error("Expected Start message (0). Found: {}", static_cast<u32>(start_message.type));
      return false;
    }

    IO::format("{} ...\t", start_message.message);

    ReportMessage outcome_message;
    if(!expect_report(in_handle, outcome_message)) return false;

    if(outcome_message.type == IPC::ReportType::Failure) {
      IO::print("Failed\n");
      failed_arr.insert({std::move(start_message.message), std::move(outcome_message.message)});
    }
    else if(outcome_message.type == IPC::ReportType::Success) {
      IO::print("Success\n");
      ASSERT(outcome_message.message.size == 0);
    }
    else {
      LOG::error("Unexpected Report Message Type: {}", static_cast<u32>(outcome_message.type));
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    pi.hThread = INVALID_HANDLE_VALUE;
    pi.hProcess = INVALID_HANDLE_VALUE;
  }

  if(failed_arr.size > 0) {
    IO::format("{} tests failed\n\n", failed_arr.size);
    
    for (const auto &t : failed_arr) {
      Axle::OwnedArr ts = Axle::format_type_set(Axle::view_arr(t.result_message), 2, 80);

      IO::err_format("\n===========\n{} failed with errors:\n{}\n", 
                     t.test_name, ts);
    }
  }
  else {
    IO::print("All tests succeeded\n\n");
  }

  return true;
}
