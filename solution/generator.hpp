#pragma once

#include "context.hpp"

#include <exception>
#include <functional>
#include <iostream>

#include <twist/memory/mmap_allocation.hpp>

namespace coro {

using Routine = std::function<void()>;

template <typename T>
class Generator {
 public:
  template <typename U>
  friend void Trampoline();
  Generator(Routine routine);
  std::optional<T> Resume();
  static void Yield(T value);
  Routine GetRoutine() const;
  bool IsCompleted() const;

 private:
  void SwitchToCaller();

 private:
  static const size_t kStackPages = 4;

 private:
  bool is_completed_{false};
  Routine routine_;
  twist::MmapAllocation stack_;
  context::ExecutionContext routine_context_;
  context::ExecutionContext caller_context_;
  std::optional<T> result_{std::nullopt};
};

template <typename T>
static Generator<T>* current_routine = nullptr;

template <typename T>
static void Trampoline() {
  current_routine<T>->GetRoutine()();
  current_routine<T>->is_completed_ = true;
  current_routine<T>->result_ = std::nullopt;
  current_routine<T>->SwitchToCaller();
}

template <typename T>
Generator<T>::Generator(Routine routine)
    : routine_(std::move(routine)),
      stack_(twist::MmapAllocation::AllocatePages(kStackPages)) {
  routine_context_.Setup(stack_.AsMemSpan(), Trampoline<T>);
}

template <typename T>
std::optional<T> Generator<T>::Resume() {
  current_routine<T> = this;
  caller_context_.SwitchTo(routine_context_);
  return std::move(result_);
}

template <typename T>
void Generator<T>::Yield(T value) {
  current_routine<T>->result_.emplace(std::move(value));
  current_routine<T>->SwitchToCaller();
}

template <typename T>
void Generator<T>::SwitchToCaller() {
  routine_context_.SwitchTo(caller_context_);
}

template <typename T>
bool Generator<T>::IsCompleted() const {
  return is_completed_;
}

template <typename T>
Routine Generator<T>::GetRoutine() const {
  return routine_;
}
}  // namespace coro
