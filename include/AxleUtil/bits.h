#ifndef AXLEUTIL_BITS_H_
#define AXLEUTIL_BITS_H_

#include <AxleUtil/math.h>

namespace Axle {

struct SquareBitMatrix {
  //Per block: ceil(side_length / 8) bytes = (side_length / 8) + 1

  //Block length = (side_length / 8) + 1
  //Block pointer = data + (Block length * val)
  //each bit corresponds to its equivalent value at that bit index

  uint8_t* data = nullptr;
  size_t side_length = 0u;
  size_t capacity = 0u;

  void free();

  constexpr SquareBitMatrix() = default;
  ~SquareBitMatrix();


  //TEMP
  SquareBitMatrix(SquareBitMatrix&&) = delete;
  SquareBitMatrix(const SquareBitMatrix&) = delete;

  constexpr static size_t bytes_per_val_per_side(size_t side_length) {
    return side_length == 0llu ? 0llu
      : ((side_length - 1llu) / 8llu) + 1llu;
  }

  bool test_a_intersects_b(size_t a, size_t b) const;
  void set_a_intersects_b(size_t a, size_t b);
  void remove_a_intersects_b(size_t a, size_t b);

  size_t new_value();
};

struct BitArray {
  u8* data = nullptr;
  usize size = 0u;
  usize capacity = 0u;
  
  constexpr BitArray() = default;
  BitArray(BitArray&&) noexcept;
  BitArray& operator=(BitArray&&) noexcept;
  ~BitArray();

  BitArray(const BitArray&) = delete;
  BitArray& operator=(const BitArray&) = delete;

  void clear() noexcept;

  [[nodiscard]] bool test(usize n) const noexcept;
  void assign(usize n, bool b) noexcept;
  void set(usize n) noexcept;
  void unset(usize n) noexcept;
  
  void insert(bool v) noexcept;
  void insert_set() noexcept;
  void insert_unset() noexcept;

  void reserve_at_least(usize n);
};

struct SetBitArray {
  uint8_t* data = nullptr;
  size_t length = 0u;
  size_t highest_set = 0u;

  constexpr SetBitArray() = default;
  SetBitArray(size_t length);
  SetBitArray(SetBitArray&&) noexcept;
  SetBitArray& operator=(SetBitArray&&) noexcept;

  SetBitArray(const SetBitArray&) = delete;
  SetBitArray& operator=(const SetBitArray&) = delete;
  ~SetBitArray();


  void set(size_t a);
  bool test(size_t a) const;

  bool intersects(const SetBitArray& b) const;
  bool test_all() const;

  usize count_set() const;
  usize count_unset() const;


  struct UnsetBitItr {
    const u8* data = nullptr;
    usize index = 0;
    usize length = 0;

    usize next();
  };  

  UnsetBitItr unset_itr() const;

  void clear();
};

template<usize N>
struct ConstBitArray {
  static_assert(N > 0);
  constexpr static usize ARRAY_SIZE = ceil_div(N, 8u);
  uint8_t data[ARRAY_SIZE] = {};

  constexpr void set(size_t a) noexcept {
    ASSERT(a < N);

    size_t index = a / 8u;
    size_t offset = a % 8u;

    data[index] |= 1u << offset;
  }
  constexpr void unset(size_t a) noexcept {
    ASSERT(a < N);

    size_t index = a / 8u;
    size_t offset = a % 8u;

    data[index] &= ~(1u << offset);
  }

  static constexpr bool internal_test_bit(const u8 (&data)[ARRAY_SIZE], usize big, usize small) noexcept {
    return (data[big] & (1u << small)) > 0u;
  }

  constexpr bool test(size_t a) const noexcept {
    ASSERT(a < N);

    size_t index = a / 8u;
    size_t offset = a % 8u;

    return internal_test_bit(data, index, offset);
  }

  constexpr bool test_all() const noexcept {
    static_assert(ARRAY_SIZE >= 1);
    constexpr size_t full_blocks = ARRAY_SIZE - 1u;
    for (size_t i = 0u; i < full_blocks; ++i) {
      if (data[i] != 0xffu) return false;//wasn't filled
    }

    constexpr size_t final_size = N % 8u;
    constexpr u8 final_block = bit_fill_lower<u8>(final_size);

    return data[full_blocks] == final_block;
  }

  constexpr usize count_set() const noexcept {
    usize count = 0;

    for(usize i = 0; i < ARRAY_SIZE; ++i) {
      // std::popcount
      // but I don't want to include bit.h at the moment
      count += __popcnt(static_cast<u32>(data[i]));
    }

    return count;
  }

  constexpr usize count_unset() const noexcept {
    return N - count_set();
  }

  struct UnsetBitItr {
    const u8 (*data)[ARRAY_SIZE] = nullptr;
    usize index = 0;

    constexpr usize next() noexcept {
      usize i_big = index / 8u;
      usize i_sml = index % 8u;

      constexpr usize l_big = N / 8u;
      constexpr usize l_sml = N % 8u;

      if(i_big < l_big) {
        while(i_sml < 8u) {
          if(!internal_test_bit(*data, i_big, i_sml)) {
            const usize n = i_big * 8u + i_sml;
            index = n + 1;
            return n;
          }

          i_sml += 1;
        }

        i_sml = 0;
        i_big += 1;
        while(i_big < l_big) {
          if(__popcnt(static_cast<u32>((*data)[i_big])) != 8u) {
            do {
              if(!internal_test_bit(*data, i_big, i_sml)) {
                const usize n = i_big * 8u + i_sml;
                index = n + 1;
                return n;
              }

              i_sml += 1;
            } while(i_sml < 8u);

            INVALID_CODE_PATH("Popcount was not 8, but didn't find the bit");
          }

          i_big += 1;
        }

        ASSERT(i_sml == 0);
      }

      ASSERT(i_big == l_big);

      while(i_sml < l_sml) {
        if(!internal_test_bit(*data, i_big, i_sml)) {
          const usize n = i_big * 8u + i_sml;
          index = n + 1;
          return n;
        }

        i_sml += 1;
      }

      index = N;
      return index;
    }

  };  

  constexpr UnsetBitItr unset_itr() const noexcept {
    return {&data, 0u};
  }

  constexpr void clear() noexcept {
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
      data[i] = 0;
    }
  }
};

template<typename T>
constexpr void set_mask(T& t, const T mask) {
  t |= mask;
}

template<typename T>
constexpr void unset_mask(T& t, const T mask) {
  t &= ~mask;
}

template<typename T>
constexpr bool test_mask(const T t, const T mask) {
  return (t & mask) == mask;
}

template<typename T>
constexpr T combine_flag(const T full, const T mask, const bool set) {
  return (full & ~mask) | (mask * set);
}

#define COMBINE_FLAG(full, mask, set) (Axle::combine_flag(full, mask, set))
#define SET_FLAG(full, mask, set) (full = COMBINE_FLAG(full, mask, set))

}
#endif
