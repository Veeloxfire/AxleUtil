#include <AxleUtil/utility.h>
#include <AxleUtil/strings.h>
#include <AxleUtil/io.h>

//Always enabled here
#ifndef STACKTRACE_ENABLE
#define STACKTRACE_ENABLE
#endif
#include <AxleUtil/stacktrace.h>

#include <AxleUtil/os/os_windows.h>
#include <debugapi.h>

#include <bit>

namespace Axle {
static bool panicking = false;
static const void* panic_callback_userdata = nullptr;
static Panic::CallbackFn panic_callback = Panic::default_panic_callback;

void Panic::set_panic_callback(const void* data, CallbackFn callback) noexcept {
  panic_callback_userdata = data;
  panic_callback = callback;
}

void Panic::default_panic_callback(const void*, const ViewArr<const char>& message) noexcept {
  IO_Single::ScopeLock lock;
  Format::STErrPrintFormatter formatter = {};
  formatter.load_string(message.data, message.size);

  using TraceNode = Axle::Stacktrace::TraceNode;
  const TraceNode* tn = Axle::Stacktrace::EXECUTION_TRACE;
  if(tn != nullptr) {
    formatter.load_string_lit("\nStacktrace:\n");
    Format::format_to(formatter, "- {}", tn->name);
    tn = tn->prev;
    while(tn != nullptr) {
      Format::format_to(formatter, "\n- {}", tn->name);
      tn = tn->prev;
    }
  }
}

[[noreturn]] void Panic::panic(const ViewArr<const char>& message) noexcept {
  panic(message.data, message.size);
}

[[noreturn]] void Panic::panic(const char* message, usize size) noexcept {
  if(IsDebuggerPresent()) DebugBreak();
  
  if(panicking) {
    fputs("ERROR: panic called, while panicking\nMessage: \"", stderr);
    fwrite(message, 1, size, stderr);
    fputs("\"\n", stderr);
  }
  else {
    panicking = true;
    panic_callback(panic_callback_userdata, {message, size});
  }

  std::terminate();
}
}
