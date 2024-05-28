#pragma once

#include "panic.hpp"

#include <iostream>
#include <exception>

#define TINY_VERIFY(cond, error)                                   \
  do {                                                             \
    if (!(cond)) {                                                 \
      TINY_PANIC("Assertion '" << #cond << "' failed: " << error); \
    }                                                              \
  } while (false);
