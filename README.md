# MemoryStorm

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
