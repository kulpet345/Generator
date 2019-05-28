cmake_minimum_required(VERSION 3.5)

enable_language(ASM)

add_executable(
  generator
  context.hpp
  context.cpp
  context.S
  generator.hpp
  test.cpp
  )
target_link_libraries(generator twist)
