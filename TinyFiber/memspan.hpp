#pragma once

#include <cstddef>
#include <cstdint>

namespace support {

struct MemSpan {
 public:
  MemSpan(char* start, size_t size) : start_(start), size_(size) {
  }

  MemSpan() : MemSpan(nullptr, 0) {
  }

  size_t Size() const noexcept {
    return size_;
  }

  char* Begin() const noexcept {
    return start_;
  }

  char* End() const noexcept {
    return start_ + size_;
  }

  char* Data() const noexcept {
    return Begin();
  }

  char* Back() const noexcept {
    return End() - 1;
  }

 private:
  char* start_;
  size_t size_;
};

}  // namespace tiny::support
