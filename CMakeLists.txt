cmake_minimum_required(VERSION 3.14)
project(actor_model CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

find_package(Threads REQUIRED)

set(ACTOR_SOURCES
  src/mailbox.cpp
  src/actor.cpp
  src/thread_pool.cpp
  src/schedular.cpp
  src/actorsystem.cpp
)

enable_testing()

add_executable(actor_tests
  tests/unit_tests.cpp
  ${ACTOR_SOURCES}
)

target_include_directories(actor_tests PRIVATE src)
target_include_directories(actor_tests SYSTEM PRIVATE third_party)
target_link_libraries(actor_tests PRIVATE gtest gtest_main Threads::Threads)
target_compile_options(actor_tests PRIVATE -Wall -Wextra)

include(GoogleTest)
gtest_discover_tests(actor_tests)

add_executable(actor_bench
  bench/throughput.cpp
  ${ACTOR_SOURCES}
)

target_include_directories(actor_bench PRIVATE src)
target_include_directories(actor_bench SYSTEM PRIVATE third_party)
target_link_libraries(actor_bench PRIVATE Threads::Threads)
target_compile_options(actor_bench PRIVATE -Wall -Wextra -O2)

add_executable(actor_latency
  bench/latency.cpp
  ${ACTOR_SOURCES}
)

target_include_directories(actor_latency PRIVATE src)
target_include_directories(actor_latency SYSTEM PRIVATE third_party)
target_link_libraries(actor_latency PRIVATE Threads::Threads)
target_compile_options(actor_latency PRIVATE -Wall -Wextra -O2)
