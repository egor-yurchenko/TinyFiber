#include "mmap_allocation.h"

#include "assert.hpp"

#include <cstdlib>
//#include <sys/mman.h>

namespace support {

#define CHECK_RESULT(ret, error) TINY_VERIFY(ret != -1, error)

//////////////////////////////////////////////////////////////////////

static size_t PagesToBytes(size_t count) {
  static const size_t kPageSize = 4096;

  return count * kPageSize;
}

MmapAllocation MmapAllocation::AllocatePages(size_t count) {
  size_t size = PagesToBytes(count);

  //void* start = mmap(/*addr=*/nullptr, /*length=*/size,
  //                   /*prot=*/PROT_READ | PROT_WRITE,
  //                   /*flags=*/MAP_PRIVATE | MAP_ANONYMOUS,
  //                   /*fd=*/-1, /*offset=*/0);

  void* start = std::malloc(size);

  TINY_VERIFY(start != nullptr , "Cannot allocate " << count << " pages");

  return MmapAllocation{(char*)start, size};
}

//void MmapAllocation::ProtectPages(size_t offset, size_t count) {
//  int ret = mprotect(/*addr=*/(void*)(start_ + PagesToBytes(offset)),
//                     /*len=*/PagesToBytes(count),
//                     /*prot=*/PROT_NONE);
//  CHECK_RESULT(
//      ret, "Cannot protect pages [" << offset << ", " << offset + count << ")");
//}

MmapAllocation::MmapAllocation(MmapAllocation&& that) noexcept {
  start_ = that.start_;
  size_ = that.size_;
  that.Reset();
}

MmapAllocation& MmapAllocation::operator=(MmapAllocation&& that) noexcept {
  Release();
  start_ = that.start_;
  size_ = that.size_;
  that.Reset();
  return *this;
}

void MmapAllocation::Release() noexcept {
  if (start_ == nullptr) {
    return;
  }

  //int ret = munmap((void*)start_, size_);
  std::free(start_);
  //CHECK_RESULT(ret, "Cannot unmap allocated pages");
}

void MmapAllocation::Reset() {
  start_ = nullptr;
  size_ = 0;
}


}  // namespace support