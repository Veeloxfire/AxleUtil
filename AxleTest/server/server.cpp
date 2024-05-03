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

struct OwnedHandle {
  HANDLE h;
  
  void close() noexcept {
    if(h != INVALID_HANDLE_VALUE) CloseHandle(h);
    h = INVALID_HANDLE_VALUE;
  }

  bool is_valid() const noexcept  {
    return h != INVALID_HANDLE_VALUE;
  }

  operator HANDLE() && noexcept {
    return std::exchange(h, INVALID_HANDLE_VALUE);
  }

  OwnedHandle() noexcept : h(INVALID_HANDLE_VALUE) {}
  OwnedHandle(HANDLE&& h_) noexcept : h(std::exchange(h_, INVALID_HANDLE_VALUE)) {}
  OwnedHandle(OwnedHandle&& h_) noexcept : h(std::exchange(h_.h, INVALID_HANDLE_VALUE)) {}

  OwnedHandle& operator=(HANDLE&& h_) noexcept {
    h = std::exchange(h_, INVALID_HANDLE_VALUE);
    return *this;
  }

  OwnedHandle& operator=(OwnedHandle&& h_) noexcept {
    if(this == &h_) return *this;

    h = std::exchange(h_.h, INVALID_HANDLE_VALUE);
    return *this;
  }
  
  ~OwnedHandle() noexcept {
    if(h != INVALID_HANDLE_VALUE) CloseHandle(h);
  }
};

struct ChildProcess {
  OwnedHandle write_handle;
  OwnedHandle read_handle;

  OwnedHandle process_handle;
  OwnedHandle thread_handle;
};

ChildProcess start_test_executable(const Axle::ViewArr<const char>& self, 
                                   const Axle::ViewArr<const char>& exe) {
  STACKTRACE_FUNCTION();
  
  OwnedHandle parent_read;
  OwnedHandle child_write;

  {
    SECURITY_ATTRIBUTES inheritable_pipes;
    inheritable_pipes.nLength = sizeof(inheritable_pipes);
    inheritable_pipes.lpSecurityDescriptor = NULL;
    inheritable_pipes.bInheritHandle = true;    

    if(!CreatePipe(&parent_read.h, &child_write.h, &inheritable_pipes, 0)) {
      LOG::error("Failed to create back communication pipes");
      return {};
    }
  }

  ASSERT(parent_read.is_valid());
  ASSERT(child_write.is_valid());

  OwnedHandle parent_write;
  OwnedHandle child_read;
  {
    SECURITY_ATTRIBUTES inheritable_pipes;
    inheritable_pipes.nLength = sizeof(inheritable_pipes);
    inheritable_pipes.lpSecurityDescriptor = NULL;

    inheritable_pipes.bInheritHandle = true;    

    if(!CreatePipe(&child_read.h, &parent_write.h, &inheritable_pipes, 0)) {
      LOG::error("Failed to create forwards communication pipes");
      return {}; 
    }
  }

  ASSERT(parent_write.is_valid());
  ASSERT(child_read.is_valid());
 
  {
    COMMTIMEOUTS pipe_timeouts={};
    pipe_timeouts.ReadIntervalTimeout = MAXDWORD;
    pipe_timeouts.ReadTotalTimeoutMultiplier = 0;
    pipe_timeouts.ReadTotalTimeoutConstant = 1000;// 1 second allowed for operations
    pipe_timeouts.WriteTotalTimeoutMultiplier = 0;
    pipe_timeouts.WriteTotalTimeoutConstant = 1000;// 1 second allowed for operations

    if(!SetCommTimeouts(parent_read.h, &pipe_timeouts)) {
      LOG::error("Failed to set timeouts");
      return {};
    }

    if(!SetCommTimeouts(child_write.h, &pipe_timeouts)) {
      LOG::error("Failed to set timeouts");
      return {};
    }
    if(!SetCommTimeouts(parent_write.h, &pipe_timeouts)) {
      LOG::error("Failed to set timeouts");
      return {};
    }
    if(!SetCommTimeouts(child_read.h, &pipe_timeouts)) {
      LOG::error("Failed to set timeouts");
      return {};
    }
  }

  STARTUPINFO startup_info;
  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.cb = sizeof(startup_info);

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  Axle::Windows::NativePath exe_path;
  ASSERT(self.size + exe.size < MAX_PATH);
  Axle::memcpy_ts(Axle::view_arr(exe_path.path, 0, self.size), self);
  Axle::memcpy_ts(Axle::view_arr(exe_path.path, self.size, exe.size), exe);

  OwnedHandle process_handle;
  OwnedHandle thread_handle;

  {
    HANDLE save_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE save_stdin = GetStdHandle(STD_INPUT_HANDLE);

    SetStdHandle(STD_OUTPUT_HANDLE, child_write.h);
    SetStdHandle(STD_INPUT_HANDLE, child_read.h);

    DEFER(&) {
      SetStdHandle(STD_OUTPUT_HANDLE, save_stdout);
      SetStdHandle(STD_INPUT_HANDLE, save_stdin);
    };

    BOOL ret = CreateProcessA(exe_path.c_str(), NULL, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup_info, &pi);

    process_handle = std::move(pi.hProcess);
    thread_handle = std::move(pi.hThread);

    if(ret == 0) {
      u32 ec = GetLastError();
      LOG::error("Failed to open process: {}. Error code: {}", exe, ec);

      return {}; 
    }
  }

  ChildProcess child = {};
  child.read_handle = std::move(parent_read);
  child.write_handle = std::move(parent_write);
  child.process_handle = std::move(process_handle);
  child.thread_handle = std::move(thread_handle);

  ASSERT(parent_read.h == INVALID_HANDLE_VALUE);
  ASSERT(parent_write.h == INVALID_HANDLE_VALUE);
  ASSERT(process_handle.h == INVALID_HANDLE_VALUE);
  ASSERT(thread_handle.h == INVALID_HANDLE_VALUE);
  return child;
}

static bool expect_valid_header(Axle::Windows::FILES::RawFile in_handle, IPC::Type type) { 
  IPC::MessageHeader header;
  if(!Axle::deserialize_le<IPC::MessageHeader>(in_handle, header)) {
    LOG::error("Header read operation failed");
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

static bool expect_report(Axle::Windows::FILES::RawFile in_handle, ReportMessage& out) {
  STACKTRACE_FUNCTION();
  
  if(!expect_valid_header(in_handle, IPC::Type::Report)) {
    return false;
  }

  if(!Axle::deserialize_le<IPC::ReportType>(in_handle, out.type)) {
    LOG::error("Report type read operation failed");
    return false;
  }

  u32 message_len;
  if(!Axle::deserialize_le<u32>(in_handle, message_len)) {
    LOG::error("Message len read operation failed");
    return false;
  }

  if(message_len > 0) {
    Axle::OwnedArr<char> m = Axle::new_arr<char>(message_len);
    Axle::ViewArr<u8> view = Axle::cast_arr<u8>(Axle::view_arr(m));
    if(!Axle::deserialize_le<Axle::ViewArr<u8>>(in_handle, view)) {
      LOG::error("Message read operation failed");
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

static bool expect_test_info(Axle::Windows::FILES::RawFile in_handle, TestInfo& out) {
  STACKTRACE_FUNCTION();
  
  Axle::Serializer<Axle::Windows::FILES::RawFile, Axle::ByteOrder::LittleEndian> ser = {in_handle};

  u32 test_count;
  {
    IPC::Deserialize::DataT<u32> dt = {test_count};

    if(!Axle::deserialize_le(ser, dt)) {
      LOG::error("Test count message invalid");
      return false;
    }
  }

  u32 strings_size;
  {
    IPC::Deserialize::DataT<u32> dt = {strings_size};

    if(!Axle::deserialize_le(ser, dt)) {
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
      if(!Axle::deserialize_le<AxleTest::IPC::MessageHeader>(ser, header)) return false;
      if(header.version != AxleTest::IPC::VERSION) return false;
      if(header.type != AxleTest::IPC::Type::Data) return false;

      u32 size;
      if(!Axle::deserialize_le<u32>(ser, size)) return false;
      ASSERT(size != 0);

      Axle::ViewArr<char> name = view_arr(strings, counter, size);
      tests[i].test_name = name;
      ASSERT(name.size != 0);
      counter += size;

      Axle::ViewArr<u8> arr = cast_arr<u8>(name);
      if(!Axle::deserialize_le<Axle::ViewArr<u8>>(ser, arr)) return false;
    }

    {
      AxleTest::IPC::MessageHeader header;
      if(!Axle::deserialize_le<AxleTest::IPC::MessageHeader>(ser, header)) return false;
      if(header.version != AxleTest::IPC::VERSION) return false;
      if(header.type != AxleTest::IPC::Type::Data) return false;

      u32 size;
      if(!Axle::deserialize_le<u32>(ser, size)) return false;

      if(size > 0) {
        Axle::ViewArr<char> name = view_arr(strings, counter, size);
        tests[i].context_name = name;
        counter += size;

        Axle::ViewArr<u8> arr = cast_arr<u8>(name);
        if(!Axle::deserialize_le<Axle::ViewArr<u8>>(ser, arr)) return false;
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
                                const Axle::ViewArr<const AxleTest::IPC::OpaqueContext>& contexts) {
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
  
  {
    ChildProcess cp = start_test_executable(self_path, client_exe);
    
    if(!cp.process_handle.is_valid()) return false;

    const Axle::Windows::FILES::RawFile out_handle = {cp.write_handle.h};
    const Axle::Windows::FILES::RawFile in_handle = {cp.read_handle.h};
    
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
      failed_arr.insert({test_name, Axle::copy_arr("Internal Error")});
      continue;
    }

    const Axle::Windows::FILES::RawFile out_handle = {cp.write_handle.h};
    const Axle::Windows::FILES::RawFile in_handle = {cp.read_handle.h};

    Axle::serialize_le(out_handle, IPC::Serialize::Execute{i});

    if(context_name.data != nullptr) {
      ASSERT(ctx_data.size > 0);
      Axle::serialize_le(out_handle, IPC::Serialize::Data{ctx_data});
    }
    
    ReportMessage outcome_message;
    if(!expect_report(in_handle, outcome_message)) {
      IO::print("Failed\n");
      failed_arr.insert({test_name, Axle::copy_arr("Internal Error")});
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
