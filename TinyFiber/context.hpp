#pragma once

#include "memspan.hpp"

#include <cstdlib>
#include <cstdint>

#include "exception.h"



namespace tinyfiber {

typedef void (*TrampolineWithoutArgs)();
typedef void (*Trampoline)(void*);

struct ExecutionContext {
  // Execution context saved on top of suspended fiber/thread stack
  void* rsp_;
  ExceptionsContext exceptions_ctx_;

  static void ContextTrampoline(void*, void*);

  // Prepare execution context for running trampoline function
  void Setup(support::MemSpan stack, TrampolineWithoutArgs trampoline);
  void Setup(support::MemSpan stack, Trampoline trampoline, void* arg);

  // Save the current execution context to 'this' and jump to the
  // 'target' context. 'target' context created directly by Setup or
  // by another target.SwitchTo(other) call.
  void SwitchTo(ExecutionContext& target);
};

}  // namespace tinyfiber