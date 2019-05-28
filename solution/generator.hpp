#pragma once

#include "context.hpp"

#include <exception>
#include <functional>
#include <iostream>

#include <twist/memory/mmap_allocation.hpp>

namespace gen {

using Routine = std::function<void()>;

template <typename T>
class Iterator;

template <typename T>
class Generator {
 public:
  template <typename U>
  friend void Trampoline();

  template <typename U>
  friend void Yield(U value);

  Generator(Routine routine);
  std::optional<T> Resume();
  static void Yield(T value);
  Routine& GetRoutine();
  Iterator<T> begin();
  Iterator<T> end();

 private:
  void SwitchToCaller();

 private:
  static const size_t kStackPages = 4;

 private:
  Routine routine_;
  twist::MmapAllocation stack_;
  context::ExecutionContext routine_context_;
  context::ExecutionContext caller_context_;
  std::optional<T> result_{std::nullopt};
  static Generator* current_generator_;
};

template <typename T>
Generator<T>* Generator<T>::current_generator_{nullptr};

template <typename T>
static void Trampoline() {
  Generator<T>::current_generator_->GetRoutine()();
  Generator<T>::current_generator_->result_ = std::nullopt;
  Generator<T>::current_generator_->SwitchToCaller();
}

template <typename T>
Generator<T>::Generator(Routine routine)
    : routine_(std::move(routine)),
      stack_(twist::MmapAllocation::AllocatePages(kStackPages)) {
  routine_context_.Setup(stack_.AsMemSpan(), Trampoline<T>);
}

template <typename T>
std::optional<T> Generator<T>::Resume() {
  Generator<T>::current_generator_ = this;
  caller_context_.SwitchTo(routine_context_);
  return std::move(result_);
}

template <typename T>
void Generator<T>::Yield(T value) {
  Generator<T>::current_generator_->result_.emplace(std::move(value));
  Generator<T>::current_generator_->SwitchToCaller();
}

template <typename T>
void Generator<T>::SwitchToCaller() {
  routine_context_.SwitchTo(caller_context_);
}

template <typename T>
Routine& Generator<T>::GetRoutine() {
  return routine_;
}

template <typename T>
static void Yield(T value) {
  Generator<T>::current_generator_->Yield(value);
}

template <typename T>
Iterator<T> Generator<T>::begin() {
  return Iterator<T>(this);
}

template <typename T>
Iterator<T> Generator<T>::end() {
   return Iterator<T>();
}

template <typename T>
class Iterator {
 public:
  friend Generator<T>;
  Iterator();
  Iterator& operator++();
  T& operator*();
  bool operator==(const Iterator<T>& other);
  bool operator!=(const Iterator<T>& other);

 private:
  Iterator(Generator<T>* generator);

 private:
  Generator<T>* generator_;
  T value_;
};

template <typename T>
Iterator<T>::Iterator(): generator_(nullptr) {}

template <typename T>
Iterator<T>::Iterator(Generator<T>* generator)
: generator_(generator) {
    ++(*this);    
}

template <typename T>
Iterator<T>& Iterator<T>::operator++() {
  std::optional<T> value = generator_->Resume();
  if (value.has_value()) {
    value_ = std::move(value.value());
  } else {
    generator_ = nullptr;
  }
  return *this;
}

template <typename T>
T& Iterator<T>::operator*() {
  return value_;
}

template <typename T>
bool Iterator<T>::operator==(const Iterator<T>& other) {
  return generator_ == other.generator_;
}

template <typename T>
bool Iterator<T>::operator!=(const Iterator<T>& other) {
  return generator_ != other.generator_;
}

}  // namespace gen


