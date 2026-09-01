cmake_minimum_required(VERSION 3.24)
project(hft_engine CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Werror)

include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(catch2)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
FetchContent_MakeAvailable(benchmark)

add_library(engine_core src/version.cpp src/order_book.cpp src/itch_framing.cpp src/itch_message.cpp)
target_include_directories(engine_core PUBLIC include)

add_executable(engine_tests tests/version_test.cpp tests/order_test.cpp tests/order_book_test.cpp tests/itch_framing_test.cpp tests/itch_message_test.cpp tests/byte_reader_test.cpp)
target_link_libraries(engine_tests PRIVATE engine_core Catch2::Catch2WithMain)

enable_testing()
add_test(NAME engine_tests COMMAND engine_tests)

add_executable(engine_benchmarks benchmarks/version_benchmark.cpp)
target_link_libraries(engine_benchmarks PRIVATE engine_core benchmark::benchmark)
