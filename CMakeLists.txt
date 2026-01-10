cmake_minimum_required(VERSION 3.10)
project(system1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(system1_lib STATIC
  src/repo.cpp
  src/storage.cpp
)

target_include_directories(system1_lib PUBLIC src)

add_executable(system1_cli src/main.cpp)
target_link_libraries(system1_cli PRIVATE system1_lib)

add_executable(system1_tests tests/tests.cpp)
target_link_libraries(system1_tests PRIVATE system1_lib)

enable_testing()
add_test(NAME system1_tests COMMAND system1_tests)
