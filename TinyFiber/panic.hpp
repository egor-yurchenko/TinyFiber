#pragma once

#include "string_builder.hpp"


namespace detail {
void Panic(const std::string& error);
}  // namespace detail


// Print error message to stderr, then abort
// Usage: TINY_PANIC("Internal error: " << e.what());

#define TINY_PANIC(error)                                       \
  do {                                                          \
    detail::Panic(support::StringBuilder()  << ": " << error);    \
  } while (false)
