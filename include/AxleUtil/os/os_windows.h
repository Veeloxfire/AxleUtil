#ifndef AXLEUTIL_OS_WINDOWS_H_
#define AXLEUTIL_OS_WINDOWS_H_

#define NOMINMAX
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef NOMINMAX
#undef NOGDICAPMASKS
#undef NOVIRTUALKEYCODES
#undef NOWINMESSAGES
#undef NOWINSTYLES
#undef NOSYSMETRICS
#undef NOMENUS
#undef NOICONS
#undef NOKEYSTATES
#undef NOSYSCOMMANDS
#undef NORASTEROPS
#undef NOSHOWWINDOW
#undef OEMRESOURCE
#undef NOATOM
#undef NOCLIPBOARD
#undef NOCOLOR
#undef NOCTLMGR
#undef NODRAWTEXT
#undef NOGDI
#undef NOKERNEL
#undef NOUSER
#undef NONLS
#undef NOMB
#undef NOMEMMGR
#undef NOMETAFILE
#undef NOMSG
#undef NOOPENFILE
#undef NOSCROLL
#undef NOSERVICE
#undef NOSOUND
#undef NOTEXTMETRIC
#undef NOWH
#undef NOWINOFFSETS
#undef NOCOMM
#undef NOKANJI
#undef NOHELP
#undef NOPROFILER
#undef NODEFERWINDOWPOS
#undef NOMCX
#undef WIN32_LEAN_AND_MEAN

// there are lots of these - will try to add more
#undef VOID
#undef CONST
#undef FALSE
#undef TRUE
#undef IN
#undef OUT
#undef OPTIONAL
#undef far
#undef near
#undef pascal
#undef CDECL
#undef CALLBACK
#undef APIENTRY
#undef APIPRIVATE
#undef PASCAL
#undef FAR
#undef NEAR
#undef STRICT

#include <AxleUtil/safe_lib.h>
namespace Axle::Windows {
  template<typename T> 
  struct VirtualPtr {
    T* ptr = nullptr;
    usize size = 0;
    usize entry = 0;

    template<typename U, typename ... J>
    U call(J&& ... t) {
      using PTR = U(*)(J...);
      return ((PTR)ptr)(std::forward<J>(t)...);
    }

    template<typename U>
    U call() {
      using PTR = U(*)();
      return ((PTR)(ptr + entry))();
    }
  };

  template<typename T>
  VirtualPtr<T> get_exectuable_memory(size_t num) {
    VirtualPtr<T> ptr ={};
    ptr.ptr = (T*) VirtualAlloc(nullptr, sizeof(T) * num, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    ptr.size = sizeof(T) * num;

    return ptr;
  }

  template<typename T>
  void free_executable_memory(VirtualPtr<T>& ptr) {
    VirtualFree(ptr.ptr, 0, MEM_RELEASE);
  }

  struct NativePath {
    char path[MAX_PATH + 1] = {};

    constexpr NativePath() = default;
    constexpr NativePath(const ViewArr<const char>& vr) {
      usize len = vr.size > MAX_PATH ? MAX_PATH : vr.size;
      memcpy_ts(view_arr(path, 0, len), view_arr(vr, 0, len)); 
    }

    constexpr const char* c_str() const {
      return path;
    }

    constexpr ViewArr<const char> view() const {
      return { path, strlen_ts(path) };
    }
  };


  NativePath get_current_directory();
  void set_current_directory(const ViewArr<const char>& str);

  struct OwnedHandle {
    HANDLE h;

    void close() noexcept;
    bool is_valid() const noexcept;

    operator HANDLE() && noexcept;

    OwnedHandle(const OwnedHandle& h_) = delete;
    OwnedHandle& operator=(const OwnedHandle& h_) = delete;
    
    OwnedHandle() noexcept;
    OwnedHandle(HANDLE&& h_) noexcept;
    OwnedHandle(OwnedHandle&& h_) noexcept;

    OwnedHandle& operator=(HANDLE&& h_) noexcept;
    OwnedHandle& operator=(OwnedHandle&& h_) noexcept;
  
    ~OwnedHandle() noexcept;
  };
}

#endif
