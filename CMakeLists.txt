cmake_minimum_required(VERSION 3.14)
project(actor_model CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

find_package(Threads REQUIRED)

enable_testing()

add_executable(actor_tests
  tests/unit_tests.cpp
  src/mailbox.cpp
  src/actor.cpp
  src/thread_pool.cpp
  src/schedular.cpp
  src/actorsystem.cpp
)

target_include_directories(actor_tests PRIVATE src)
target_link_libraries(actor_tests PRIVATE gtest gtest_main Threads::Threads)
target_compile_options(actor_tests PRIVATE -Wall -Wextra)

include(GoogleTest)
gtest_discover_tests(actor_tests)
