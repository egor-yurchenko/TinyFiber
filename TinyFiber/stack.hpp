#pragma once

#include "memspan.hpp"
#include "mmap_allocation.h"

#include <cstdint>

namespace tinyfiber {

//////////////////////////////////////////////////////////////////////

class Stack {
 public:
  Stack() = default;

  static Stack Allocate();

  Stack(Stack&& that) = default;
  Stack& operator=(Stack&& that) = default;

  char* Bottom() const;

  size_t Size() const {
    return allocation_.Size();
  }

  support::MemSpan AsMemSpan() const;

 private:
  Stack(support::MmapAllocation allocation);

 private:
  support::MmapAllocation allocation_;
};

//////////////////////////////////////////////////////////////////////

class StackBuilder {
  using Self = StackBuilder;

  using Word = std::uintptr_t;
  static const size_t kWordSize = sizeof(Word);

 public:
  StackBuilder(char* bottom) : top_(bottom) {
  }

  void AlignNextPush(size_t alignment) {
    size_t shift = (size_t)(top_ - kWordSize) % alignment;
    top_ -= shift;
  }

  void* Top() const {
    return top_;
  }

  Self& Push(Word value) {
    top_ -= kWordSize;
    *(Word*)top_ = value;
    return *this;
  }

  Self& Push(void* value) {
      return Push((Word)value);
  }


  Self& Allocate(size_t bytes) {
    top_ -= bytes;
    return *this;
  }

 private:
  char* top_;
};

}  // namespace tinyfiber
