#include <AxleUtil/utility.h>
#include <AxleUtil/strings.h>
#include <AxleUtil/io.h>

//Always enabled here
#define STACKTRACE_ENABLE
#include <AxleUtil/stacktrace.h>

#include <AxleUtil/os/os_windows.h>
#include <debugapi.h>

#include <bit>

namespace Axle {
static OwnedArr<const char> create_exception_message(const char* message) noexcept {
  Format::ArrayFormatter formatter = {};
  formatter.load_string(message, strlen_ts(message));

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
  formatter.null_terminate();
  return formatter.take();
}

struct InternalException final : std::exception {
  OwnedArr<const char> message_string;

  InternalException(OwnedArr<const char>&& data) : message_string(std::move(data)) {}
  virtual ~InternalException() = default;

  virtual const char* what() const noexcept override {
    return message_string.data;
  }
};

void throw_testing_assertion(const char* message) {
  if(IsDebuggerPresent()) DebugBreak();
  
  if (std::uncaught_exceptions() == 0) {
    OwnedArr<const char> final_message = create_exception_message(message);
    throw InternalException(std::move(final_message));
  }
}

void abort_assertion(const char* message) noexcept {
  if(IsDebuggerPresent()) DebugBreak();

  const ViewArr<const char> msg_view = {message, strlen_ts(message) };

  IO::err_print(msg_view);
  
  using TraceNode = Axle::Stacktrace::TraceNode;
  const TraceNode* tn = Axle::Stacktrace::EXECUTION_TRACE;
  if(tn != nullptr) {
    IO::err_print("\nStacktrace:\n");
    IO::err_format("- {}", tn->name);
    tn = tn->prev;
    while(tn != nullptr) {
      IO::err_format("\n- {}", tn->name);
      tn = tn->prev;
    }
  }

  std::abort();
}

SquareBitMatrix::~SquareBitMatrix() {
  free();
}

void SquareBitMatrix::free() {
  free_no_destruct<uint8_t>(data);

  data = nullptr;
  side_length = 0;
  capacity = 0;
}

bool SquareBitMatrix::test_a_intersects_b(size_t a, size_t b) const {
  ASSERT(a < side_length);
  ASSERT(b < side_length);

  const size_t bytes_per_val = bytes_per_val_per_side(side_length);
  const uint8_t* a_data = data + bytes_per_val * a;

  const size_t b_mod8 = b % 8;
  const size_t b_div8 = b / 8;

  ASSERT(a_data + b_div8 < data + capacity);

  return (a_data[b_div8] & (1 << b_mod8)) > 0;
}

void SquareBitMatrix::set_a_intersects_b(size_t a, size_t b) {
  ASSERT(a < side_length);
  ASSERT(b < side_length);

  const size_t bytes_per_val = bytes_per_val_per_side(side_length);
  uint8_t* a_data = data + bytes_per_val * a;

  const size_t b_mod8 = b % 8;
  const size_t b_div8 = b / 8;

  ASSERT(a_data + b_div8 < data + capacity);

  a_data[b_div8] |= (uint8_t)(1 << b_mod8);
}

void SquareBitMatrix::remove_a_intersects_b(size_t a, size_t b) {
  ASSERT(a < side_length);
  ASSERT(b < side_length);

  const size_t bytes_per_val = bytes_per_val_per_side(side_length);
  uint8_t* a_data = data + bytes_per_val * a;

  const size_t b_mod8 = b % 8;
  const size_t b_div8 = b / 8;

  ASSERT(a_data + b_div8 < data + capacity);

  a_data[b_div8] &= ~(uint8_t)(1 << b_mod8);
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

BitArray::BitArray(size_t length_) : data(new u8[ceil_div(length_, 8)]{ 0 }), length(length_), highest_set(0) {}
BitArray::~BitArray() { delete[] data; }

BitArray::BitArray(BitArray&& b) noexcept
  : data(std::exchange(b.data, nullptr)),
    length(std::exchange(b.length, 0)), highest_set(std::exchange(b.highest_set, 0)) {}

BitArray& BitArray::operator=(BitArray&& b) noexcept {
  if(this == &b) return *this;

  data = std::exchange(b.data, nullptr);
  length = std::exchange(b.length, 0);
  highest_set = std::exchange(b.highest_set, 0);


  return *this;
}

void BitArray::set(size_t a) {
  ASSERT(a < length);

  size_t index = a / 8;
  size_t offset = a % 8;

  data[index] |= 1 << offset;

  if (a > highest_set) highest_set = a;
}

constexpr bool test_bit(const u8* data, usize big, usize small) {
  return (data[big] & (1 << small)) > 0;
}

bool BitArray::test(size_t a) const {
  ASSERT(a < length);

  size_t index = a / 8;
  size_t offset = a % 8;

  return test_bit(data, index, offset);
}

bool BitArray::intersects(const BitArray& other) const {
  ASSERT(length == other.length);

  size_t blocks = ceil_div(length, 8);
  for (size_t i = 0; i < blocks; ++i) {
    if ((data[i] & other.data[i]) != 0) return true;
  }

  return false;
}

bool BitArray::test_all() const {
  if (length == 0) return true;
  if (highest_set != length - 1) return false;

  size_t full_blocks = ceil_div(length, 8) - 1;
  for (size_t i = 0; i < full_blocks; ++i) {
    if (data[i] != 0xff) return false;//wasn't filled
  }

  size_t final_size = length % 8;

  const u8 final_block = bit_fill_lower<u8>(final_size);

  return data[full_blocks] == final_block;
}

void BitArray::clear() {
  size_t highest_block = ceil_div(highest_set, 8);

  for (size_t i = 0; i < highest_block; ++i) {
    data[i] = 0;
  }

  highest_set = 0;
}

usize BitArray::count_set() const {
  usize count = 0;

  usize top = ceil_div(length, 8);
  for(usize i = 0; i < top; ++i) {
    count += std::popcount(data[i]);
  }

  return count;
}

usize BitArray::count_unset() const {
  return length - count_set();
}

struct BitItr {
  const u8* data;
  usize index;
  usize length;
  usize next();
};

usize BitArray::UnsetBitItr::next() {
  usize i_big = index / 8;
  usize i_sml = index % 8;

  const usize l_big = length / 8;
  const usize l_sml = length % 8;

  if(i_big < l_big) {
    while(i_sml < 8) {
      if(!test_bit(data, i_big, i_sml)) {
        const usize n = i_big * 8 + i_sml;
        index = n + 1;
        return n;
      }

      i_sml += 1;
    }

    i_sml = 0;
    i_big += 1;
    while(i_big < l_big) {
      if(std::popcount(data[i_big]) != 8) {
        do {
          if(!test_bit(data, i_big, i_sml)) {
            const usize n = i_big * 8 + i_sml;
            index = n + 1;
            return n;
          }

          i_sml += 1;
        } while(i_sml < 8);

        INVALID_CODE_PATH("Popcount was not 8, but didn't find the bit");
      }

      i_big += 1;
    }

    ASSERT(i_sml == 0);
  }

  ASSERT(i_big == l_big);

  while(i_sml < l_sml) {
    if(!test_bit(data, i_big, i_sml)) {
      const usize n = i_big * 8 + i_sml;
      index = n + 1;
      return n;
    }

    i_sml += 1;
  }

  index = length;
  return index;
}

BitArray::UnsetBitItr BitArray::unset_itr() const {
  return {data, 0, length};
}
}
