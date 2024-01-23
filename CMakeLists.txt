# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.14)

# Name of the project
project(MemoryManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Compilation options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# # Turn on logging if running in debug mode
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     add_compile_definitions(LOGGING)
# endif()

# Create a few variables for the folder names, so they are easier to rename in
# the future
set(SRC_DIR src)

# Add all the source files needed to build the executable
add_executable(
  MemoryManager
  ${SRC_DIR}/main.cpp
  ${SRC_DIR}/memory_manager.cpp
)


# Add tclap
# TODO: There is probably a way to import tclap from its source bz2 url similar to gtest, but
#       extracting the source include directory & having our project include that works, so
#       sticking to that solution for now...
target_include_directories(MemoryManager PUBLIC tclap-1.4.0-rc1/include/)

enable_testing()

add_executable(
  MemoryManagerTests
  ${SRC_DIR}/main_tests.cpp
  ${SRC_DIR}/memory_manager_tests.cpp
  ${SRC_DIR}/memory_manager.cpp
)

target_link_libraries(
  MemoryManagerTests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(MemoryManagerTests)
