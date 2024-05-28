#pragma once

#include "memspan.hpp"

namespace support {

class MmapAllocation {
 public:
  MmapAllocation() {
    Reset();
  }

  // Allocate `count` pages of zeroed memory
  static MmapAllocation AllocatePages(size_t count);

  // Moving
  MmapAllocation(MmapAllocation&& that) noexcept;
  MmapAllocation& operator=(MmapAllocation&& that) noexcept;

  ~MmapAllocation() {
    Release();
  }

  char* Start() const {
    return start_;
  }

  char* End() const {
    return start_ + size_;
  }

  size_t Size() const {
    return size_;
  }

  MemSpan AsMemSpan() const {
    return MemSpan(start_, size_);
  }

  // Protect range of pages
  // Protected pages cannot be read, written or executed
  // offset - in pages, zero-based
  //void ProtectPages(size_t offset, size_t count);

  void Release() noexcept;

 private:
  MmapAllocation(char* start, size_t size) : start_(start), size_(size) {
  }

  void Reset();

 private:
  // Aligned to page boundary
  char* start_;
  size_t size_;
};

}  // namespace tiny::support
