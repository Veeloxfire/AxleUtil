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
namespace Windows = Axle::Windows;

using namespace Axle::Primitives;

struct ChildProcess {
  Windows::OwnedHandle pipe_handle;

  Windows::OwnedHandle process_handle;
  Windows::OwnedHandle thread_handle;
};

void terminate_child(HANDLE cp, u32 timeout) {
  WaitForSingleObject(cp, timeout);

  DWORD ec = 0;
  BOOL r = GetExitCodeProcess(cp, &ec);
  ASSERT(r != 0);

  if(ec == STILL_ACTIVE) {
    TerminateProcess(cp, 1);
  }
}

ChildProcess start_test_executable(const Axle::ViewArr<const char>& self, 
                                   const Axle::ViewArr<const char>& exe) {
  STACKTRACE_FUNCTION();
  
  Windows::OwnedHandle pipe;

  {
    SECURITY_ATTRIBUTES inheritable_pipes;
    inheritable_pipes.nLength = sizeof(inheritable_pipes);
    inheritable_pipes.lpSecurityDescriptor = NULL;
    inheritable_pipes.bInheritHandle = true;

    pipe = CreateNamedPipeA(AxleTest::IPC::PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, PIPE_UNLIMITED_INSTANCES, 0, 0,
        NMPWAIT_USE_DEFAULT_WAIT, &inheritable_pipes);

    if(!pipe.is_valid()) {
      LOG::error("Failed to create communication pipe");
      return {};
    }
  }


  ASSERT(pipe.is_valid());

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

  Windows::OwnedHandle process_handle = std::move(pi.hProcess);
  Windows::OwnedHandle thread_handle = std::move(pi.hThread);

  if(ret == 0) {
    u32 ec = GetLastError();
    LOG::error("Failed to open process: {}. Error code: {}", exe, ec);

    return {}; 
  }

  if(!ConnectNamedPipe(pipe.h, NULL)) {
    DWORD err = GetLastError();
    if(err != ERROR_PIPE_CONNECTED) {
      terminate_child(process_handle.h, 0);

      LOG::error("Pipe connection error");
      return {};
    }
  }

  ChildProcess child = {};
  child.pipe_handle = std::move(pipe);
  child.process_handle = std::move(process_handle);
  child.thread_handle = std::move(thread_handle);

  ASSERT(pipe.h == INVALID_HANDLE_VALUE);
  ASSERT(process_handle.h == INVALID_HANDLE_VALUE);
  ASSERT(thread_handle.h == INVALID_HANDLE_VALUE);
  return child;
}

template<typename S>
static bool expect_valid_header(S&& serializer, IPC::Type type) { 
  IPC::MessageHeader header;
  if(!Axle::deserialize_le<IPC::MessageHeader>(std::forward<S>(serializer), header)) {
    return false;
  }

  if(header.version != IPC::VERSION) {
    LOG::error("Incompatable IPC version. Expected: {}, Found: {}", IPC::VERSION, header.version);
    return false;
  }

  if(header.type != type) {
    LOG::error("Expected Type: {}, Found: {}", type, header.type);
    return false;
  }

  return true;
}

struct ReportMessage {
  IPC::ReportType type;
  Axle::OwnedArr<const char> message;
};

template<typename S>
static bool expect_report(S&& serializer, ReportMessage& out) {
  STACKTRACE_FUNCTION();
  
  if(!expect_valid_header(std::forward<S>(serializer), IPC::Type::Report)) {
    return false;
  }

  if(!Axle::deserialize_le<IPC::ReportType>(std::forward<S>(serializer), out.type)) {
    return false;
  }

  u32 message_len;
  if(!Axle::deserialize_le<u32>(std::forward<S>(serializer), message_len)) {
    return false;
  }

  if(message_len > 0) {
    Axle::OwnedArr<char> m = Axle::new_arr<char>(message_len);
    Axle::ViewArr<u8> view = Axle::cast_arr<u8>(Axle::view_arr(m));
    if(!Axle::deserialize_le<Axle::ViewArr<u8>>(std::forward<S>(serializer), view)) {
      return false;
    }

    out.message = std::move(m);
  }

  return true;
}

struct SingleTest {
  Axle::ViewArr<const char> test_name;
  Axle::ViewArr<const char> context_name;
};

struct TestInfo {
  Axle::OwnedArr<SingleTest> tests;
  Axle::OwnedArr<const char> strings;
};

template<typename S>
static bool expect_test_info(S&& serializer, TestInfo& out) {
  STACKTRACE_FUNCTION();
  
  u32 test_count;
  {
    IPC::Deserialize::DataT<u32> dt = {test_count};

    if(!Axle::deserialize_le(std::forward<S>(serializer), dt)) {
      LOG::error("Test count message invalid");
      return false;
    }
  }

  u32 strings_size;
  {
    IPC::Deserialize::DataT<u32> dt = {strings_size};

    if(!Axle::deserialize_le(std::forward<S>(serializer), dt)) {
      LOG::error("Strings size message invalid");
      return false;
    }
  }

  Axle::OwnedArr<SingleTest> tests = Axle::new_arr<SingleTest>(test_count);
  Axle::OwnedArr<char> strings = Axle::new_arr<char>(strings_size);
  u32 counter = 0;

  for(u32 i = 0; i < test_count; ++i) {
    {
      AxleTest::IPC::MessageHeader header;
      if(!Axle::deserialize_le<AxleTest::IPC::MessageHeader>(std::forward<S>(serializer), header)) return false;
      if(header.version != AxleTest::IPC::VERSION) return false;
      if(header.type != AxleTest::IPC::Type::Data) return false;

      u32 size;
      if(!Axle::deserialize_le<u32>(std::forward<S>(serializer), size)) return false;
      ASSERT(size != 0);

      Axle::ViewArr<char> name = view_arr(strings, counter, size);
      tests[i].test_name = name;
      ASSERT(name.size != 0);
      counter += size;

      Axle::ViewArr<u8> arr = cast_arr<u8>(name);
      if(!Axle::deserialize_le<Axle::ViewArr<u8>>(std::forward<S>(serializer), arr)) return false;
    }

    {
      AxleTest::IPC::MessageHeader header;
      if(!Axle::deserialize_le<AxleTest::IPC::MessageHeader>(std::forward<S>(serializer), header)) return false;
      if(header.version != AxleTest::IPC::VERSION) return false;
      if(header.type != AxleTest::IPC::Type::Data) return false;

      u32 size;
      if(!Axle::deserialize_le<u32>(std::forward<S>(serializer), size)) return false;

      if(size > 0) {
        Axle::ViewArr<char> name = view_arr(strings, counter, size);
        tests[i].context_name = name;
        counter += size;

        Axle::ViewArr<u8> arr = cast_arr<u8>(name);
        if(!Axle::deserialize_le<Axle::ViewArr<u8>>(std::forward<S>(serializer), arr)) return false;
      }
      else {
        tests[i].context_name = {};
      }
    }
  }
 
  out.tests = std::move(tests);
  out.strings = std::move(strings);
  return true;
}

bool AxleTest::IPC::server_main(const Axle::ViewArr<const char>& client_exe,
                                const Axle::ViewArr<const AxleTest::IPC::OpaqueContext>& contexts,
                                u32 timeout_time_ms) {
  STACKTRACE_FUNCTION();

  Axle::Windows::NativePath self_dir_holder;
  DWORD self_path_len = GetModuleFileNameA(NULL, self_dir_holder.path, MAX_PATH);
  while(self_path_len > 0) {
    char c = self_dir_holder.path[self_path_len - 1];
    if(c == '\\' || c == '/') break;
    self_dir_holder.path[self_path_len - 1] = '\0';
    self_path_len -= 1;
  }
  const Axle::ViewArr<const char> self_path = { self_dir_holder.path, self_path_len };


  TestInfo test_info;

  Windows::OwnedHandle wait_event = CreateEventA(NULL, false, false, NULL);
  ASSERT(wait_event.is_valid());
  
  {
    ChildProcess cp = start_test_executable(self_path, client_exe);
    
    if(!cp.process_handle.is_valid()) return false;

    DEFER(&cp, timeout_time_ms, handle = cp.pipe_handle.h) {
      terminate_child(cp.process_handle.h, timeout_time_ms);
      DisconnectNamedPipe(handle);
    };

    const Axle::Windows::FILES::TimeoutFile out_handle = {wait_event.h, cp.pipe_handle.h, timeout_time_ms};
    const Axle::Windows::FILES::TimeoutFile& in_handle = out_handle;
    
    Axle::serialize_le(out_handle, IPC::Serialize::QueryTestInfo{});
    if(!expect_test_info(in_handle, test_info)) {
      LOG::error("Failed to read test info");
      return false;
    }
  }

  LOG::debug("{} tests found", test_info.tests.size);

  struct FailedResult {
    Axle::ViewArr<const char> test_name;
    Axle::OwnedArr<const char> result_message;
  };

  Axle::Array<FailedResult> failed_arr;

  for(u32 i = 0; i < test_info.tests.size; ++i) {
    const Axle::ViewArr<const char> test_name = test_info.tests[i].test_name;
    const Axle::ViewArr<const char> context_name = test_info.tests[i].context_name;
    Axle::ViewArr<const u8> ctx_data = {};
    if(context_name.data != nullptr) {
      IO::format("{} ({}) ...\t", test_name, context_name);

      FOR(contexts, c_it) {
        if(Axle::memeq_ts(c_it->name, context_name)) {
          ctx_data = c_it->data;
          break;
        }
      }

      if(ctx_data.data == nullptr) {
        IO::print("Failed\n");
        failed_arr.insert({test_name, Axle::format("Invalid context type: {}", context_name)});
        continue;
      }
    }
    else { 
      IO::format("{} ...\t", test_name);
    }

    ChildProcess cp = start_test_executable(self_path, client_exe);

    if(!cp.process_handle.is_valid()) {
      IO::print("Failed\n");
      failed_arr.insert({test_name, Axle::copy_arr("Internal Error: Failed to create process")});
      continue;
    }
 
    DEFER(&cp, timeout_time_ms, handle = cp.pipe_handle.h) {
      terminate_child(cp.process_handle.h, timeout_time_ms);
      DisconnectNamedPipe(handle);
    };

    const Axle::Windows::FILES::TimeoutFile out_handle = {wait_event.h, cp.pipe_handle.h, timeout_time_ms};
    const Axle::Windows::FILES::TimeoutFile& in_handle = out_handle;

    Axle::serialize_le(out_handle, IPC::Serialize::Execute{i});

    if(context_name.data != nullptr) {
      ASSERT(ctx_data.size > 0);
      Axle::serialize_le(out_handle, IPC::Serialize::Data{ctx_data});
    }
    
    ReportMessage outcome_message;
    if(!expect_report(in_handle, outcome_message)) {
      IO::print("Failed\n");
      failed_arr.insert({test_name, Axle::copy_arr("Internal Error: Message never recieved (likely timeout)")});
      continue;
    }

    if(outcome_message.type == IPC::ReportType::Failure) {
      IO::print("Failed\n");
      failed_arr.insert({test_name, std::move(outcome_message.message)});
    }
    else if(outcome_message.type == IPC::ReportType::Success) {
      IO::print("Success\n");
      ASSERT(outcome_message.message.size == 0);
    }
    else {
      failed_arr.insert({test_name, Axle::format("Unexpected Report Message Type: {}", outcome_message.type)});
      continue;
    }
  }

  if(failed_arr.size > 0) {
    IO::err_format("\n{} / {} tests failed\n", failed_arr.size, test_info.tests.size);
    
    for (const auto &t : failed_arr) {
      Axle::OwnedArr ts = Axle::format_type_set(Axle::view_arr(t.result_message), 2, 80);

      IO::err_format("\n===========\n\n\"{}\" failed with errors:\n{}\n", 
                     t.test_name, ts);
    }

    IO::err_print("\n===========\n");
  }
  else {
    IO::format("All tests ({}) succeeded\n", test_info.tests.size);
  }

  return true;
}
