include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)

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
catch_discover_tests(memorystorm_tests)
