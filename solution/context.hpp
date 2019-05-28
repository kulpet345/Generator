#pragma once

#include <twist/memory/memspan.hpp>

#include <cstdlib>
#include <cstdint>

namespace context {

struct ExecutionContext;

// Switch between ExecutionContext-s
extern "C" void SwitchContext(ExecutionContext* from, ExecutionContext* to);

typedef void (*Trampoline)();

struct ExecutionContext {
  // Execution context saved on top of suspended fiber/thread stack
  void* rsp_;

  // Prepare execution context for running trampoline function
  void Setup(twist::MemSpan stack, Trampoline trampoline);

  // Save the current execution context to 'this' and jump to the 'target'
  // context.
  // 'target' context created directly by Setup or by another
  // target.SwitchTo(smth) call.
  void SwitchTo(ExecutionContext& target);
};

}  // namespace context
