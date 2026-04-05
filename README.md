# MemoryStorm

[![Tests](https://github.com/VoxelStorm-Ltd/memorystorm/actions/workflows/tests.yml/badge.svg)](https://github.com/VoxelStorm-Ltd/memorystorm/actions/workflows/tests.yml)
[![codecov](https://codecov.io/gh/VoxelStorm-Ltd/memorystorm/branch/main/graph/badge.svg)](https://codecov.io/gh/VoxelStorm-Ltd/memorystorm)

Small and simple C++ memory use monitoring and reporting library by VoxelStorm.

## Core functions

The main purpose of the library is to provide system memory summaries in a cross-platform manner.

```cpp
namespace memorystorm {

uint64_t get_stack_available();

uint64_t get_physical_total();
uint64_t get_physical_available();
uint64_t get_physical_usage();

uint64_t get_virtual_total();
uint64_t get_virtual_available();
uint64_t get_virtual_usage();

}
```

## Utility functions
```cpp
namespace memorystorm {

std::string human_readable(uint64_t amount);
std::string get_stats();
void dump_stats();

}
```

The `human_readable` function can be called on any numerical value representing a number of bytes, and will return a human readable value in kilobytes, megabytes, gigabytes, etc.

## Dependencies

- [`cast_if_required`](https://github.com/VoxelStorm-Ltd/cast_if_required) — a shared VoxelStorm utility header providing safe type casts, shared across VoxelStorm projects (libvoxelstorm). The test build fetches this automatically via CMake FetchContent. When including memorystorm directly into your project, ensure `cast_if_required.h` is on your include path.
