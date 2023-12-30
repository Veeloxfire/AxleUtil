#ifndef AXLEUTIL_STACKTRACE_H_
#define AXLEUTIL_STACKTRACE_H_

#include <AxleUtil/safe_lib.h>

namespace Axle::Stacktrace {
  struct TraceNode {
    const TraceNode* prev;
    ViewArr<const char> name; 
  };

  inline thread_local const TraceNode* EXECUTION_TRACE = nullptr;

  struct ScopedExecTrace {
    TraceNode node;
    constexpr ScopedExecTrace(const ViewArr<const char>& name) {
      if  (!std::is_constant_evaluated()) {
        node = {EXECUTION_TRACE, name};
        EXECUTION_TRACE = &node;
      }
    }

    constexpr ~ScopedExecTrace() {
      if (!std::is_constant_evaluated()) {
        ASSERT(EXECUTION_TRACE == &node);
        EXECUTION_TRACE = EXECUTION_TRACE->prev;
      }
    }
  };
}
#endif

//every enable should have a chance to enable
#if defined(STACKTRACE_MACROS_DISABLED) && defined(STACKTRACE_ENABLE)
#undef STACKTRACE_MACROS_DISABLED
#undef STACKTRACE_SCOPE
#undef STACKTRACE_FUNCTION
#endif

//every disable should have a change to disable
#if defined(STACKTRACE_MACROS_ENABLED) && !defined(STACKTRACE_ENABLE)
#undef STACKTRACE_MACROS_ENABLED
#undef STACKTRACE_SCOPE
#undef STACKTRACE_FUNCTION
#endif

#if !defined(STACKTRACE_MACROS_ENABLED) && !defined(STACKTRACE_MACROS_DISABLED)
#ifdef STACKTRACE_ENABLE
#define STACKTRACE_MACROS_ENABLED
#define STACKTRACE_SCOPE(name) Axle::Stacktrace::ScopedExecTrace JOIN(_stacktrace_, __LINE__)(Axle::lit_view_arr(name))
#define STACKTRACE_FUNCTION() STACKTRACE_SCOPE(__FUNCTION__)
#else
#define STACKTRACE_MACROS_DISABLED
#define STACKTRACE_SCOPE(name) ((void)0)
#define STACKTRACE_FUNCTION() ((void)0)
#endif
#endif
