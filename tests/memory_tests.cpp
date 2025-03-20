#include <AxleUtil/memory.h>

#include <AxleTest/unit_tests.h>

using namespace Axle::Primitives;

struct DeleteCounter {
  usize* i = nullptr;

  DeleteCounter() = default;
  DeleteCounter(usize* p) : i(p) {}

  DeleteCounter(const DeleteCounter&) = delete;
  DeleteCounter(DeleteCounter&&) = delete;
  DeleteCounter& operator=(const DeleteCounter&) = delete;
  DeleteCounter& operator=(DeleteCounter&&) = delete;

  ~DeleteCounter() {
    if(i != nullptr) {
      *i += 1;
    }
  }
};


struct DeleteCounter2 {
  usize* i = nullptr;

  DeleteCounter2() = default;

  DeleteCounter2(const DeleteCounter2&) = delete;
  DeleteCounter2(DeleteCounter2&&) = delete;
  DeleteCounter2& operator=(const DeleteCounter2&) = delete;
  DeleteCounter2& operator=(DeleteCounter2&&) = delete;

  ~DeleteCounter2() {
    if(i != nullptr) {
      *i += 1;
    }
  }
};

TEST_FUNCTION(GlobalAllocators, basic) {
  {
    usize counter = 0;
    DeleteCounter* d = Axle::allocate_default<DeleteCounter>(1000);
    for(usize i = 0; i < 1000; ++i) {
      d[i].i = &counter;
    }
    Axle::free_destruct_n<DeleteCounter>(d, 1000);

    TEST_EQ(static_cast<usize>(1000), counter);
  }

  {
    usize counter = 0;
    DeleteCounter* d = Axle::allocate_default<DeleteCounter>(1000);
    for(usize i = 0; i < 1000; ++i) {
      d->i = &counter;
    }
    Axle::free_no_destruct<DeleteCounter>(d);

    TEST_EQ(static_cast<usize>(0), counter);
  }

  {
    usize counter = 0;
    Axle::free_destruct_single<DeleteCounter>(Axle::allocate_single_constructed<DeleteCounter>(&counter));
    TEST_EQ(static_cast<usize>(1), counter);
  }
}

TEST_FUNCTION(GlobalAllocators, alloc_zero) {
  int* i = Axle::allocate_default<int>(0);
  TEST_EQ(static_cast<int*>(nullptr), i);
}

TEST_FUNCTION(GrowingMemoryPool, alloc_zero) {
  Axle::GrowingMemoryPool<128> pool;

  int* empty = pool.allocate_n<int>(0);

  TEST_EQ(static_cast<int*>(nullptr), empty);
}

TEST_FUNCTION(GrowingMemoryPool, destruct) {
  usize counter = 0;

  {
    Axle::GrowingMemoryPool<128> pool;

    for(usize i = 0; i < 256; ++i) {
      auto* dc = pool.allocate<DeleteCounter>();
      auto* dc2 = pool.allocate<DeleteCounter2>();

      dc->i = &counter;
      dc2->i = &counter;
    }

    struct Big {
      DeleteCounter arr[200];

      Big() = default;
      ~Big() = default;
      
      Big(const Big&) = delete;
      Big(Big&&) = delete;
      Big& operator=(const Big&) = delete;
      Big& operator=(Big&&) = delete;
    };

    Big* big = pool.allocate<Big>();
    for(usize i = 0; i < 200; ++i) {
      big->arr[i].i = &counter;
    }

    TEST_EQ(static_cast<usize>(0), counter);
  }

  TEST_EQ(static_cast<usize>(256 * 2 + 200), counter);
}

TEST_FUNCTION(GrowingMemoryPool, destruct_n) {
  usize counter = 0;

  {
    Axle::GrowingMemoryPool<128> pool;

    for(usize i = 0; i < 25; ++i) {
      auto* dc = pool.allocate_n<DeleteCounter>(10);
      auto* dc2 = pool.allocate_n<DeleteCounter2>(5);

      for(usize j = 0; j < 10; ++j) {
        dc[j].i = &counter;
      }

      for(usize j = 0; j < 5; ++j) {
        dc2[j].i = &counter;
      }
    }

    auto* big = pool.allocate_n<DeleteCounter>(200);
    for(usize i = 0; i < 200; ++i) {
      big[i].i = &counter;
    }

    TEST_EQ(static_cast<usize>(0), counter);
  }

  TEST_EQ(static_cast<usize>(25 * 10 + 25 * 5 + 200), counter);
}

TEST_FUNCTION(DenseBlockAllocator, allocate) {
  usize counter = 0;

  {
    Axle::DenseBlockAllocator<DeleteCounter> pool;

    for(usize i = 0; i < 256; ++i) {
      auto* dc = pool.allocate();

      dc->i = &counter;
    }

    TEST_EQ(static_cast<usize>(0), counter);
  }

  TEST_EQ(static_cast<usize>(256), counter);
}


TEST_FUNCTION(DenseBlockAllocator, allocate_n) {
  usize counter = 0;

  {
    Axle::DenseBlockAllocator<DeleteCounter> pool;

    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(255);
      TEST_EQ(static_cast<usize>(255), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(129);
      TEST_EQ(static_cast<usize>(129), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(70);
      TEST_EQ(static_cast<usize>(70), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(20);
      TEST_EQ(static_cast<usize>(20), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(16);
      TEST_EQ(static_cast<usize>(16), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(16);
      TEST_EQ(static_cast<usize>(16), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(16);
      TEST_EQ(static_cast<usize>(16), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(16);
      TEST_EQ(static_cast<usize>(16), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(8);
      TEST_EQ(static_cast<usize>(8), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(4);
      TEST_EQ(static_cast<usize>(4), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(2);
      TEST_EQ(static_cast<usize>(2), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }
    {
      Axle::ViewArr<DeleteCounter> cn = pool.allocate_n(1);
      TEST_EQ(static_cast<usize>(1), cn.size);
      FOR_MUT(cn, it) { it->i = &counter; };
    }

    TEST_EQ(static_cast<usize>(0), counter);
  }

  TEST_EQ(static_cast<usize>(255 + 129 + 70 + 20 + 16 + 16 + 16 + 16 + 8 + 4 + 2 + 1), counter);
}

TEST_FUNCTION(DenseBlockAllocator, alloc_zero) {
  Axle::DenseBlockAllocator<int> pool;

  const Axle::ViewArr<int> empty = pool.allocate_n(0);

  TEST_EQ(static_cast<int*>(nullptr), empty.data);
  TEST_EQ(static_cast<usize>(0), empty.size);
}
