#include <AxleUtil/memory.h>
#include <AxleUtil/utility.h>

namespace Axle {
u8* MemoryPool::push_alloc_bytes(usize size, usize align) {
  usize new_top = top;
  if(top % align != 0) {
    new_top += align - (top % align);
  }

  u8* new_ptr = mem + new_top;
  new_top += size;

  ASSERT(new_top < total);
  top = new_top;

  return new_ptr;
}

#ifdef ARENA_ALLOCATOR_DEBUG

uint8_t* ArenaAllocator::alloc_no_construct(size_t bytes) {
  void* d = malloc(bytes);
  allocated.insert(d);

  return (uint8_t*)d;
}

void ArenaAllocator::free_no_destruct(void* val) {
  usize check = allocated.size;
  ASSERT(check != 0);

  allocated.remove_if([val](void* data) { return val == data; });

  ASSERT(check == allocated.size + 1);
  
  free(val);
}

ArenaAllocator::~ArenaAllocator() {
  ASSERT(allocated.size == 0);
}

#else

void ArenaAllocator::add_to_free_list(ArenaAllocator::FreeList* new_fl) {
  //Should otherwise all be combined
  //Just need to find if this is after a free space or before a free space

  ASSERT(!_debug_freelist_loops());

  FreeList* prev = nullptr;
  FreeList* fl = free_list;

  if (fl == nullptr) {
    free_list = new_fl;
    return;
  }

  ASSERT(fl->next != fl);

  bool found_before = false;
  bool found_after = false;

  while (fl != nullptr) {
    if (!found_after && (uint64_t*)fl == ((uint64_t*)new_fl + new_fl->qwords_available + 1)) {
      //fl comes directly after new_fl

      //Remove from list
      if (prev != nullptr) {
        prev->next = fl->next;
      }

      new_fl->qwords_available += fl->qwords_available + 1;

      fl = fl->next;

      //Can we exit?
      if (found_before) {
        break;
      }
      found_after = true;
    }
    else if (!found_before && (uint64_t*)new_fl == ((uint64_t*)fl + fl->qwords_available + 1)) {
      //new_fl comes directly after fl

      //Remove from list
      if (prev != nullptr) {
        prev->next = fl->next;
      }

      auto* save_next = fl->next;

      fl->qwords_available += new_fl->qwords_available + 1;
      fl->next = nullptr;

      new_fl = fl;
      
      fl = save_next;

      //Can we exit
      if (found_after) {
        break;
      }
      found_before = true;
    }
    else {
      prev = fl;
      fl = fl->next;
    }
  }

  if (prev != nullptr) {
    //free_list is not new_fl
    new_fl->next = (FreeList*)free_list;
  }
  free_list = new_fl;

  ASSERT(free_list->next != free_list);
}

bool ArenaAllocator::_debug_freelist_loops() const {
  Array<FreeList*> list_elements ={};

  FreeList* list = free_list;

  while (list != nullptr) {
    if(list_elements.contains(list)) return true;

    list_elements.insert(list);
    list = list->next;
  }

  return false;
}

bool ArenaAllocator::_debug_valid_pointer(void* ptr) const {
  Block* block = base;

  //Checking the pointer is actually from one of these blocks

  while (block != nullptr) {
    if(ptr >= (block->data + 1)/*+ 1 for the saved free size*/
       && ptr < (block->data + Block::BLOCK_SIZE)) return true;

    block = block->next;
  }

  return false;
}

bool ArenaAllocator::_debug_is_allocated_data(uint64_t* ptr, usize len) const {
  auto list = free_list;

  auto ptr_end = ptr + len;
  //Try to find if its in the freelist

  while (list != nullptr) {
    u64* start = (u64*)list;
    u64* end = start + list->qwords_available + 1;

    //Check if the ranges overlap - error if they do
    if((ptr <= start && start < ptr_end)
       || (ptr < end && end <= ptr_end)
       || (ptr <= start && end <= ptr_end))
      return false;

    list = list->next;
  }

  return true;
}

uint8_t* ArenaAllocator::alloc_no_construct(size_t bytes) {
  ASSERT(bytes != 0);
  ASSERT(!_debug_freelist_loops());

  size_t req_size = ceil_div(bytes, 8);
  ASSERT(req_size != 0);

  FreeList* prev = nullptr;
  FreeList* fl = (FreeList*)free_list;

  if (bytes > Block::BLOCK_SIZE - 1) {
    INVALID_CODE_PATH("Arena allocator block does not have enough space");
  }

  //Find free space
  while (fl != nullptr && fl->qwords_available < req_size) {
    prev = fl;
    fl = fl->next;
  }

  if (fl == nullptr) {
    //Allocate more data
    new_block();
    fl = free_list;
  }

  const uint64_t available_space = fl->qwords_available;
 
  uint64_t* const used_space = (uint64_t*)fl;
  uint64_t* current_alloc = used_space + 1;


  //can we fit a new node?
  if (available_space - req_size >= 2) {
    //Yay there is more space

    FreeList* new_fl = (FreeList*)(current_alloc + req_size);

    if (prev != nullptr) {
      prev->next = new_fl;
    }
    else {
      free_list = new_fl;
    }

    new_fl->qwords_available = available_space - (req_size + 1);
    new_fl->next = fl->next;
  }
  else {
    //Not enough space for another value
    req_size = available_space;

    if (prev != nullptr) {
      //not the top of the free list
      prev->next = fl->next;
    }
    else {
      //is top of free list
      free_list = fl->next;
    }
  }

  //How much data to free
  *used_space = req_size;

  ASSERT(!_debug_freelist_loops());
  return (uint8_t*)current_alloc;
}


void ArenaAllocator::free_no_destruct(void* val) {
  ASSERT(val != nullptr);
  ASSERT(!_debug_freelist_loops());
  ASSERT(_debug_valid_pointer(val));

  uint64_t* ptr = (uint64_t*)val;

  const uint64_t free_size = ptr[-1];

  FreeList* new_fl = (FreeList*)(ptr - 1);
  new_fl->qwords_available = free_size;
  new_fl->next = nullptr;

  add_to_free_list(new_fl);
}


void ArenaAllocator::new_block() {
  Block* block = allocate_default<Block>(1);

  FreeList* fl = (FreeList*)block->data;

  fl->qwords_available = Block::BLOCK_SIZE - 1;

  fl->next = free_list;
  free_list = fl;

  block->next = base;
  base = block;
}

ArenaAllocator::~ArenaAllocator() {
  free_destruct_single<Block>(base);
}

ArenaAllocator::Block::~Block() { free_destruct_single<Block>(next); }
#endif


}
