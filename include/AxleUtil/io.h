#ifndef AXLEUTIL_IO_H_
#define AXLEUTIL_IO_H_

#include <AxleUtil/safe_lib.h>
#include <AxleUtil/formattable.h>

namespace IO_Single {
  void print(const char* string, usize N);
  void print(const ViewArr<char>& string);
  void print(const ViewArr<const char>& string);
  void print(const char c);

  template<usize N>
  void print(const char(&string)[N]) {
    print(string, N);
  }

  void err_print(const char* string, usize N);
  void err_print(const ViewArr<char>& string);
  void err_print(const ViewArr<const char>& string);
  void err_print(const char c);

  template<usize N>
  void err_print(const char(&string)[N]) {
    err_print(string, N);
  }

  void lock();
  void unlock();

  struct ScopeLock {
    ScopeLock() {
      lock();
    }

    ScopeLock(const ScopeLock&) = delete;
    ScopeLock(ScopeLock&&) = delete;
    ScopeLock& operator=(const ScopeLock&) = delete;
    ScopeLock& operator=(ScopeLock&&) = delete;

    ~ScopeLock() {
      unlock();
    }
  };
}

namespace Format {
  struct STPrintFormatter {
    template<usize N>
    void load_string_lit(const char(&str)[N]) {
      IO_Single::print(str);
    }

    template<usize N>
    void load_string_exact(const char(&str)[N]) {
      IO_Single::print(str);
    }

    inline void load_string(const char* str, usize N) {
      ASSERT(N > 0);
      IO_Single::print(str, N);
    }

    inline void load_char(char c) {
      IO_Single::print(c);
    }
  };

  struct STErrPrintFormatter {
    template<usize N>
    void load_string_lit(const char(&str)[N]) {
      IO_Single::err_print(str);
    }

    template<usize N>
    void load_string_exact(const char(&str)[N]) {
      IO_Single::err_print(str);
    }

    inline void load_string(const char* str, usize N) {
      ASSERT(N > 0);
      IO_Single::err_print(str, N);
    }

    inline void load_char(char c) {
      IO_Single::err_print(c);
    }
  };
}

namespace IO_Single {
  template<typename ... T>
  void format(const Format::FormatString& format, const T& ... ts) {
    Format::STPrintFormatter result = {};

    Format::format_to_formatter(result, format, ts...);
  }
}

namespace IO {
  inline void print(const char* string, usize N) {
    IO_Single::ScopeLock lock;
    IO_Single::print(string, N);
  }
  inline void print(const ViewArr<char>& string) {
    IO_Single::ScopeLock lock;
    IO_Single::print(string);
  }
  inline void print(const ViewArr<const char>& string) {
    IO_Single::ScopeLock lock;
    IO_Single::print(string);
  }
  inline void print(const char c) {
    IO_Single::ScopeLock lock;
    IO_Single::print(c);
  }

  template<usize N>
  inline void print(const char(&string)[N]) {
    IO_Single::ScopeLock lock;
    IO_Single::print(string, N);
  }

  inline void err_print(const char* string, usize N) {
    IO_Single::ScopeLock lock;
    IO_Single::err_print(string, N);
  }
  inline void err_print(const ViewArr<char>& string) {
    IO_Single::ScopeLock lock;
    IO_Single::err_print(string);
  }
  inline void err_print(const ViewArr<const char>& string) {
    IO_Single::ScopeLock lock;
    IO_Single::err_print(string);
  }
  inline void err_print(const char c) {
    IO_Single::ScopeLock lock;
    IO_Single::err_print(c);
  }

  template<usize N>
  inline void err_print(const char(&string)[N]) {
    IO_Single::ScopeLock lock;
    IO_Single::err_print(string, N);
  }

  template<typename ... T>
  void format(const Format::FormatString& format, const T& ... ts) {
    IO_Single::ScopeLock lock;
    Format::STPrintFormatter result = {};

    Format::format_to_formatter(result, format, ts...);
  }

  template<typename ... T>
  void err_format(const Format::FormatString& format, const T& ... ts) {
    IO_Single::ScopeLock lock;
    Format::STErrPrintFormatter result = {};

    Format::format_to_formatter(result, format, ts...);
  }
}
#endif