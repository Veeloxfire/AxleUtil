#include <AxleUtil/bits.h>
#include <AxleUtil/memory.h>

#include <bit>

namespace Axle {
SquareBitMatrix::~SquareBitMatrix() {
  free();
}

void SquareBitMatrix::free() {
  free_no_destruct<uint8_t>(data);

  data = nullptr;
  side_length = 0u;
  capacity = 0u;
}

bool SquareBitMatrix::test_a_intersects_b(size_t a, size_t b) const {
  ASSERT(a < side_length);
  ASSERT(b < side_length);

  const size_t bytes_per_val = bytes_per_val_per_side(side_length);
  const uint8_t* a_data = data + bytes_per_val * a;

  const size_t b_mod8 = b % 8u;
  const size_t b_div8 = b / 8u;

  ASSERT(a_data + b_div8 < data + capacity);

  return (a_data[b_div8] & (1u << b_mod8)) > 0;
}

void SquareBitMatrix::set_a_intersects_b(size_t a, size_t b) {
  ASSERT(a < side_length);
  ASSERT(b < side_length);

  const size_t bytes_per_val = bytes_per_val_per_side(side_length);
  uint8_t* a_data = data + bytes_per_val * a;

  const size_t b_mod8 = b % 8u;
  const size_t b_div8 = b / 8u;

  ASSERT(a_data + b_div8 < data + capacity);

  a_data[b_div8] |= (uint8_t)(1u << b_mod8);
}

void SquareBitMatrix::remove_a_intersects_b(size_t a, size_t b) {
  ASSERT(a < side_length);
  ASSERT(b < side_length);

  const size_t bytes_per_val = bytes_per_val_per_side(side_length);
  uint8_t* a_data = data + bytes_per_val * a;

  const size_t b_mod8 = b % 8u;
  const size_t b_div8 = b / 8u;

  ASSERT(a_data + b_div8 < data + capacity);

  a_data[b_div8] &= ~(uint8_t)(1u << b_mod8);
}

size_t SquareBitMatrix::new_value() {
  const size_t bytes_per_val_now = bytes_per_val_per_side(side_length);
  const size_t bytes_per_val_next = bytes_per_val_per_side(side_length + 1);
  const size_t required_capacity = bytes_per_val_next * (side_length + 1);

  //check if we have enough space
  if (required_capacity > capacity) {
    const size_t old_capacity = capacity;

    if (capacity == 0) {
      capacity = 8;
    }
    else {
      capacity = (size_t)1 << small_log_2_ceil(required_capacity);
    }
    data = reallocate_default<u8>(data, old_capacity, capacity);
  }

  //Check if we need to fix data shape
  if (bytes_per_val_now < bytes_per_val_next) {
    auto block_i = side_length == 0 ? data
      : data + bytes_per_val_now * (side_length - 1);
    auto new_i = data + bytes_per_val_next * side_length;

    //Dont need to fix the first block as it will stay the same
    const auto block_end = data + bytes_per_val_now;

    const size_t diff = bytes_per_val_next - bytes_per_val_now;

    while (block_i > block_end) {
      for (size_t i = 0; i < diff; i++) {
        new_i[bytes_per_val_next - (i + 1)] = 0;
      }

      for (size_t i = 0; i < bytes_per_val_now; i++) {
        new_i[bytes_per_val_now - (i + 1)] = block_i[bytes_per_val_now - (i + 1)];
      }

      block_i -= bytes_per_val_now;
      new_i -= bytes_per_val_now;
    }
  }

  //Finally return new value
  return side_length++;
}

BitArray::BitArray(BitArray&& b) noexcept
  : data(std::exchange(b.data, nullptr)),
    size(std::exchange(b.size, 0u)),
    capacity(std::exchange(b.capacity, 0u))
{}

BitArray& BitArray::operator=(BitArray&& b) noexcept {
  if (this == &b) return *this;

  clear();
  
  data = std::exchange(b.data, nullptr);
  size = std::exchange(b.size, 0u);
  capacity = std::exchange(b.capacity, 0u);

  return *this;
}

BitArray::~BitArray() {
  clear();
}

void BitArray::clear() noexcept {
  ASSERT(capacity % 8 == 0);
  free_destruct_n<u8>(data, capacity / 8);
  data = nullptr;
  size = 0;
  capacity = 0;
}

bool BitArray::test(usize n) const noexcept {
  ASSERT(n < size);

  usize index = n / 8u;
  usize offset = n % 8u;

  return (data[index] & (1u << offset)) > 0u;
}

void BitArray::assign(usize n, bool b) noexcept {
  ASSERT(n < size);

  usize index = n / 8u;
  usize offset = n % 8u;

  u32 bit = static_cast<usize>(b) << offset;
  u32 surrounding = static_cast<u32>(data[index]) & ~(1u << offset);
  data[index] = static_cast<u8>(bit | surrounding);
}

void BitArray::set(usize n) noexcept {
  ASSERT(n < size);

  usize index = n / 8u;
  usize offset = n % 8u;

  data[index] |= (1u << offset);
}

void BitArray::unset(usize n) noexcept {
  ASSERT(n < size);

  usize index = n / 8u;
  usize offset = n % 8u;

  data[index] &= ~(1u << offset);
}

void BitArray::insert(bool v) noexcept {
  ASSERT(size <= capacity);
  if (size == capacity) { 
    reserve_at_least(size + 1);
  }
  
  usize i = size;

  ASSERT(size < capacity);
  size += 1;
  assign(i, v);
}

void BitArray::insert_set() noexcept {
  ASSERT(size <= capacity);
  if (size == capacity) { 
    reserve_at_least(size + 1);
  }
  
  usize i = size;

  ASSERT(size < capacity);
  size += 1;
  set(i);
}

void BitArray::insert_unset() noexcept {
  ASSERT(size <= capacity);
  if (size == capacity) { 
    reserve_at_least(size + 1);
  }
  
  usize i = size;

  ASSERT(size < capacity);
  size += 1;
  unset(i);
}

void BitArray::reserve_at_least(usize n) {
  if (n <= capacity) return;

  // Always create at at least 8 bytes
  usize new_capacity = 64;
  
  while (new_capacity < n) {
    new_capacity *= 2;
  }

  // otherwise there is no point growing
  ASSERT(capacity < new_capacity);
  ASSERT(n <= new_capacity);

  ASSERT(new_capacity % 8 == 0);
  ASSERT(capacity % 8 == 0);
  data = Axle::reallocate_default<u8>(data, capacity / 8, new_capacity / 8);
  capacity = new_capacity;
}

SetBitArray::SetBitArray(size_t length_) : data(Axle::allocate_default<u8>(ceil_div(length_, 8))), length(length_), highest_set(0) {
  std::memset(data, 0, ceil_div(length_, 8));
}
SetBitArray::~SetBitArray() {
  Axle::free_destruct_n<u8>(data, ceil_div(length, 8));
}

SetBitArray::SetBitArray(SetBitArray&& b) noexcept
  : data(std::exchange(b.data, nullptr)),
    length(std::exchange(b.length, 0u)),
    highest_set(std::exchange(b.highest_set, 0u))
{}

SetBitArray& SetBitArray::operator=(SetBitArray&& b) noexcept {
  if(this == &b) return *this;
  
  Axle::free_destruct_n<u8>(data, ceil_div(length, 8));

  data = std::exchange(b.data, nullptr);
  length = std::exchange(b.length, 0u);
  highest_set = std::exchange(b.highest_set, 0u);


  return *this;
}

void SetBitArray::set(size_t a) {
  ASSERT(a < length);

  size_t index = a / 8u;
  size_t offset = a % 8u;

  data[index] |= 1u << offset;

  if (a > highest_set) highest_set = a;
}

constexpr bool test_bit(const u8* data, usize big, usize small) {
  return (data[big] & (1u << small)) > 0u;
}

bool SetBitArray::test(size_t a) const {
  ASSERT(a < length);

  size_t index = a / 8u;
  size_t offset = a % 8u;

  return test_bit(data, index, offset);
}

bool SetBitArray::intersects(const SetBitArray& other) const {
  ASSERT(length == other.length);

  size_t blocks = ceil_div(length, 8u);
  for (size_t i = 0; i < blocks; ++i) {
    if ((data[i] & other.data[i]) != 0u) return true;
  }

  return false;
}

bool SetBitArray::test_all() const {
  if (length == 0u) return true;
  if (highest_set != length - 1u) return false;

  size_t full_blocks = ceil_div(length, 8u) - 1u;
  for (size_t i = 0u; i < full_blocks; ++i) {
    if (data[i] != 0xffu) return false;//wasn't filled
  }

  size_t final_size = length % 8u;

  const u8 final_block = bit_fill_lower<u8>(final_size);

  return data[full_blocks] == final_block;
}

void SetBitArray::clear() {
  size_t highest_block = ceil_div(highest_set, 8);

  for (size_t i = 0; i < highest_block; ++i) {
    data[i] = 0;
  }

  highest_set = 0;
}

usize SetBitArray::count_set() const {
  usize count = 0;

  usize top = ceil_div(length, 8u);
  for(usize i = 0; i < top; ++i) {
    count += std::popcount(data[i]);
  }

  return count;
}

usize SetBitArray::count_unset() const {
  return length - count_set();
}

struct BitItr {
  const u8* data;
  usize index;
  usize length;
  usize next();
};

usize SetBitArray::UnsetBitItr::next() {
  usize i_big = index / 8u;
  usize i_sml = index % 8u;

  const usize l_big = length / 8u;
  const usize l_sml = length % 8u;

  if(i_big < l_big) {
    while(i_sml < 8u) {
      if(!test_bit(data, i_big, i_sml)) {
        const usize n = i_big * 8u + i_sml;
        index = n + 1;
        return n;
      }

      i_sml += 1;
    }

    i_sml = 0;
    i_big += 1;
    while(i_big < l_big) {
      if(std::popcount(data[i_big]) != 8u) {
        do {
          if(!test_bit(data, i_big, i_sml)) {
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
    if(!test_bit(data, i_big, i_sml)) {
      const usize n = i_big * 8u + i_sml;
      index = n + 1;
      return n;
    }

    i_sml += 1;
  }

  index = length;
  return index;
}

SetBitArray::UnsetBitItr SetBitArray::unset_itr() const {
  return {data, 0u, length};
}
}
