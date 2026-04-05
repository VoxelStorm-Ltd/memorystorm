cmake_minimum_required(VERSION 3.15)
project(memorystorm_tests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  cast_if_required
  GIT_REPOSITORY https://github.com/VoxelStorm-Ltd/cast_if_required.git
  GIT_TAG        master
)
FetchContent_GetProperties(cast_if_required)
if(NOT cast_if_required_POPULATED)
  if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
  endif()
  FetchContent_Populate(cast_if_required)
endif()

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)

# Build the memorystorm library from source for testing.
# The library is intended for direct inclusion; this target exists only for tests.
add_library(memorystorm STATIC ../memorystorm/memorystorm.cpp)
# Include the memorystorm headers and fetched cast_if_required dependency
target_include_directories(memorystorm PUBLIC ../memorystorm ${cast_if_required_SOURCE_DIR})
if(WIN32)
  target_link_libraries(memorystorm PRIVATE Psapi)
endif()

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(WARNING "Coverage is only supported with GCC or Clang")
  else()
    target_compile_options(memorystorm PRIVATE --coverage -O0 -g)
    target_link_options(memorystorm PRIVATE --coverage)
  endif()
endif()

add_executable(memorystorm_tests test_memorystorm.cpp)
target_link_libraries(memorystorm_tests PRIVATE memorystorm Catch2::Catch2WithMain)

if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(memorystorm_tests PRIVATE --coverage -O0 -g)
    target_link_options(memorystorm_tests PRIVATE --coverage)
  endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
enable_testing()
catch_discover_tests(memorystorm_tests)
