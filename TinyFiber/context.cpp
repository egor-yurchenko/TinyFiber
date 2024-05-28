#include "context.hpp"
#include "stack.hpp"

namespace tinyfiber {

    extern "C" void SwitchContext(ExecutionContext * from, ExecutionContext * to);

// View for stack-saved context
struct StackSavedContext {
  // Layout of the StackSavedContext matches the layout of the stack
  // in context.S at the 'Switch stacks' comment

  // Callee-saved registers
  // Saved manually in SwitchContext
    
    void* esi;
    void* edi;
    void* ebx;
    void* ebp;

  //void* rbp;
  //void* rbx;

  //void* r12;
  //void* r13;
  //void* r14;
  //void* r15;

  // Saved automatically by 'call' instruction
   void* rip;
};

void ExecutionContext::ContextTrampoline(void* arg6, void* arg7)
{
    Trampoline tr = (Trampoline)arg6;
    tr(arg7);
}

void ExecutionContext::Setup(support::MemSpan stack, TrampolineWithoutArgs trampoline) {
  // https://eli.thegreenplace.net/2011/02/04/where-the-top-of-the-stack-is-on-x86/

  StackBuilder builder(stack.Back());

  // Ensure trampoline will get 16-byte aligned frame pointer (rbp)
  // 'Next' here means first 'pushq %rbp' in trampoline prologue
  builder.AlignNextPush(16);
  

  // Reserve space for stack-saved context
  builder.Allocate(sizeof(StackSavedContext));

  auto* saved_context = (StackSavedContext*)builder.Top();
  saved_context->rip = (void*)trampoline;

  // Set current stack top
  rsp_ = saved_context;
}

void ExecutionContext::Setup(support::MemSpan stack, Trampoline trampoline, void* arg)
{
    StackBuilder builder(stack.Back());

    // Ensure trampoline will get 16-byte aligned frame pointer (rbp)
    // 'Next' here means first 'pushq %rbp' in trampoline prologue
    builder.AlignNextPush(16);


    
    builder.Push(arg);
    builder.Push((void*)trampoline);
    builder.Push((void*)int(0));

    //builder.Push((std::uintptr_t)(void*)trampoline);

    // Reserve space for stack-saved context
    builder.Allocate(sizeof(StackSavedContext));

    auto* saved_context = (StackSavedContext*)builder.Top();
    saved_context->rip = (void*)ContextTrampoline;

    // Set current stack top
    rsp_ = saved_context;

}

void ExecutionContext::SwitchTo(ExecutionContext& target) {
    SwitchExceptionsContext(exceptions_ctx_, target.exceptions_ctx_);
    SwitchContext(this, &target);
}

}  // namespace tinyfiber




/*#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.global FUNCTION_NAME(SwitchContext)

# SwitchContext(from, to)

#FUNCTION_NAME(SwitchContext):
SwitchContext:
    # SwitchContext frame created on top of the current stack

    # 1. Save current execution context to 'from'

    # 1.1 Save callee-saved registers on top of the current stack

    # https://stackoverflow.com/questions/18024672/what-registers-are-preserved-through-a-linux-x86-64-function-call
    # https://uclibc.org/docs/psABI-x86_64.pdf

    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12

    pushq %rbx
    pushq %rbp

    # Switch stacks

    # 1.2 Save current stack pointer to 'from' ExecutionContext
    movq %rsp, (%rdi) # from->rsp_ := rsp

    # 2. Activate 'to' execution context

    # 2.1 Set stack pointer to target stack
    movq (%rsi), %rsp # rsp := to->rsp_

    # 2.2 Restore and pop registers saved on target stack

    popq %rbp
    popq %rbx

    popq %r12
    popq %r13
    popq %r14
    popq %r15

    # Pop current SwitchContext frame from target stack

    retq
*/