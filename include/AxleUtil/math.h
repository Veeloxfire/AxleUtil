#ifndef AXLEUTIL_MATH_H_
#define AXLEUTIL_MATH_H_

#include <AxleUtil/safe_lib.h>

namespace Axle {
 
constexpr inline u64 MAX_DECIMAL_U64_DIGITS = sizeof("18446744073709551615") - 1;

constexpr u64 greatest_common_divisor(u64 v1, u64 v2) {
  //Swap to be the correct way around
  if (v1 > v2) {
    const u64 temp = v1;
    v1 = v2;
    v2 = temp;
  }

  while (v2 != 0) {
    const u64 temp = v1 % v2;
    v1 = v2;
    v2 = temp;
  }

  return v1;
}

constexpr u64 lowest_common_multiple(u64 v1, u64 v2) {
  const u64 gcd = greatest_common_divisor(v1, v2);

  return (v1 * v2) / gcd;
}

template<typename T, typename B>
constexpr T bit_fill_lower(B bits) {
  constexpr B U64_MAX_SHIFT = 64;
  ASSERT(bits <= U64_MAX_SHIFT);

  if (bits == 0) return 0;

  constexpr B B_MAX_BITS{sizeof(T) * 8};
  if (bits > B_MAX_BITS) bits = B_MAX_BITS;

  return static_cast<uint64_t>(-1) >> (U64_MAX_SHIFT - bits);
}

template<typename T, typename B>
constexpr T bit_fill_upper(B bits) {
  constexpr B B_MAX_BITS{sizeof(T) * 8};
  return ~bit_fill_lower<T, B>(B_MAX_BITS - bits);
}

constexpr size_t ceil_div(size_t x, size_t y) noexcept {
  return x / y + (x % y != 0);
}

constexpr uint64_t ceil_to_pow_2(uint64_t v) {
  //See: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  ASSERT(v != 0);
  ASSERT(v <= (1llu << 63llu));

  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;

  return v;
}

//Log 2 for uniform random 64 bit number
constexpr inline uint64_t log_2(uint64_t v) {
  //TODO: fancy bitwise stuff

  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  uint64_t max = 64;
  uint64_t min = 0;

  while (max > min + 1) {
    uint64_t mid = min + ((max - min) / 2);
    uint64_t guess = 1llu << mid;

    if (guess == v) {
      return mid;
    }
    else if (guess < v) {
      min = mid;
    }
    else {
      max = mid;
    }
  }

  return min;
}

constexpr inline uint64_t pow_16(uint64_t v) {
  return (1ull << (4ull * v));
}

constexpr inline uint64_t pow_10(uint64_t v) {
  if (v > 19) {
    INVALID_CODE_PATH("Power too high!");
  }

  constexpr uint64_t pow10[] = {
    1ull,
    10ull,
    100ull,
    1000ull,
    10000ull,
    100000ull,
    1000000ull,
    10000000ull,
    100000000ull,
    1000000000ull,
    10000000000ull,
    100000000000ull,
    1000000000000ull,
    10000000000000ull,
    100000000000000ull,
    1000000000000000ull,
    10000000000000000ull,
    100000000000000000ull,
    1000000000000000000ull,
    10000000000000000000ull
  };

  return pow10[v];
}

constexpr inline uint64_t log_10_floor(uint64_t v) {
  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  //Max uint64_t = 18446744073709551615
  //Max pow 10   = 10000000000000000000ull

  uint64_t max = 20;
  uint64_t min = 0;

  while (max > min + 1) {
    uint64_t mid = min + ((max - min) / 2);
    uint64_t guess = pow_10(mid);

    if (guess == v) {
      return mid;
    }
    else if (guess < v) {
      min = mid;
    }
    else {
      max = mid;
    }
  }

  return min;
}

//Log 2 optimised for small numbers
//Floors the output
constexpr inline uint64_t small_log_2_floor(uint64_t v) {
  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  uint64_t counter = 0;
  while (v > 1) {
    v >>= 1;
    counter += 1;
  }

  return counter;
}

constexpr inline uint64_t small_log_2_ceil(uint64_t v) {
  if (v == 0) {
    INVALID_CODE_PATH("MATH ERROR! Cannot log of 0");
  }

  int found1 = 0;//max value of 1

  uint64_t counter = 0;
  while (v > 1) {
    found1 |= (v & 0b1);
    v >>= 1;
    counter += 1;
  }

  return counter + found1;
}

template<typename T>
constexpr T ceil_to_n(T val, T n) {
  const T raised = val + (n - 1);
  return raised - (raised % n);
}

template<usize N, typename T>
constexpr T ceil_to_N(T val) {
  const T raised = val + (N - 1);
  return raised - (raised % N);
}

template<typename T>
constexpr T ceil_to_8(T val) {
  return ceil_to_N<8>(val);
}

}
#endif
