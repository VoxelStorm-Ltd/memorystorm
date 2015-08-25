#include "memorystorm.h"
#include <sstream>
#include <iostream>
#include "platform_defines.h"
#if defined(PLATFORM_WINDOWS)
  #include <windows.h>
  #include <psapi.h>
#elif defined(PLATFORM_LINUX) {
  #include <sys/resource.h>
  #include <unistd.h>
#elif defined(PLATFORM_MACOS) {
  #include <sys/types.h>
  #include <sys/sysctl.h>
  #include <mach/vm_statistics.h>
  #include <mach/mach.h>
  #include <mach/mach_types.h>
  #include <mach/mach_init.h>
  #include <mach/mach_host.h>
  // TODO: check if any of these are redundant
#endif // defined

namespace memorystorm {

uint64_t get_stack_available() {
  /// Measure the available stack space
  #if defined(PLATFORM_WINDOWS)
    MEMORY_BASIC_INFORMATION mbi;                                               // page range
    VirtualQuery((PVOID)&mbi, &mbi, sizeof(mbi));                               // get range
    return (UINT_PTR)&mbi - (UINT_PTR)mbi.AllocationBase;                       // subtract from top (stack grows downward on win)
  #elif defined(PLATFORM_LINUX) {
    rlimit limit;                                                               // hard limit and soft limit
    getrlimit(RLIMIT_STACK, limit);                                             // stack size availability
    return std::min(limit.rlim_cur, limit.rlim_max);                            // return the smallest of the two
  #elif defined(PLATFORM_MACOS) {
    // TODO: OS X
    return 0;
  #endif // defined
}

uint64_t get_physical_total() {
  /// Fetch the total memory of the system
  #if defined(PLATFORM_WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<uint64_t>(status.ullTotalPhys);                            // physical memory
  #elif defined(PLATFORM_LINUX) {
    //uint64_t pages = sysconf(_SC_PHYS_PAGES);
    //uint64_t page_size = sysconf(_SC_PAGE_SIZE);
    //return pages * page_size;
    // alternative:
    struct sysinfo info;
    sysinfo(&info);
    uint64_t result = info.totalram;
    result *= info.mem_unit;                                                    // don't collapse this to avoid int overflow on rhs
    return result;
  #elif defined(PLATFORM_MACOS) {
    int64_t result;
    length = sizeof(result);
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    sysctl(mib, 2, &result, &length, NULL, 0);
    return result;
  #endif // defined
}
uint64_t get_physical_available() {
  /// Fetch the available memory of the system
  #if defined(PLATFORM_WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<uint64_t>(status.ullAvailPhys);                            // physical memory
  #elif defined(PLATFORM_LINUX) {
    struct sysinfo info;
    sysinfo(&info);
    uint64_t totalVirtualMem = info.freeram;
    totalVirtualMem *= info.mem_unit;                                           // don't collapse this to avoid int overflow on rhs
    return totalVirtualMem;
  #elif defined(PLATFORM_MACOS) {
    mach_port_t mach_port;
    mach_port = mach_host_self();
    vm_size_t page_size;
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);
    if(host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
       host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stats, &count) == KERN_SUCCESS) {
      return static_cast<uint64_t>(vm_stats.free_count) * static_cast<uint64_t>(page_size);
    } else {
      return 0;
    }
  #endif // defined
}
uint64_t get_physical_usage() {
  /// Fetch the memory used by this process
  #if defined(PLATFORM_WINDOWS)
    PROCESS_MEMORY_COUNTERS counters;
    GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters));
    return counters.WorkingSetSize;                                             // physical memory used
  #elif defined(PLATFORM_LINUX) {
    FILE *file = fopen("/proc/self/status", "r");
    uint64_t result = 0;
    char line[128];
    while(fgets(line, 128, file) != NULL) {
      if(strncmp(line, "VmRSS:", 6) == 0) {
        unsigned int const length = strlen(line);
        while(*line < '0' || *line > '9') {
          line++;
        }
        line[length - 3] = '\0';
        result = atoi(line) * 1024;                                             // value is kilobytes
        break;
      }
    }
    fclose(file);
    return result;
  #elif defined(PLATFORM_MACOS) {
    struct task_basic_info info;
    mach_msg_type_number_t info_count = TASK_BASIC_INFO_COUNT;
    if(task_info(mach_task_self(), TASK_BASIC_INFO, static_cast<task_info_t>(&info), &info_count) != KERN_SUCCESS) {
      return 0;
    }
    return t_info.resident_size;
  #endif // defined
}

uint64_t get_virtual_total() {
  /// Fetch the total memory of the system
  #if defined(PLATFORM_WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<uint64_t>(status.ullTotalPageFile);                        // total virtual memory (including swapfiles)
  #elif defined(PLATFORM_LINUX) {
    struct sysinfo info;
    sysinfo(&info);
    uint64_t result = info.totalram;
    result += info.totalswap;                                                   // Add other values in next statement to avoid int overflow on right hand side...
    result *= info.mem_unit;
    return result;
  #elif defined(PLATFORM_MACOS) {
    xsw_usage xsu = {0};
    uint64_t size = sizeof(xsu);
    if(sysctlbyname("vm.swapusage", &xsu, &size, NULL, 0) != 0) {
      perror("unable to get swap usage by calling sysctlbyname(\"vm.swapusage\",...)");
    }
    return xsu.xsu_total;
  #endif // defined
}
uint64_t get_virtual_available() {
  /// Fetch the available memory of the system
  #if defined(PLATFORM_WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<uint64_t>(status.ullAvailPageFile);                        // available virtual memory (including swapfiles)
  #elif defined(PLATFORM_LINUX) {
    struct sysinfo info;
    sysinfo(&info);
    uint64_t result = info.freeram;
    result += info.freeswap;                                                    // Add other values in next statement to avoid int overflow on right hand side...
    result *= info.mem_unit;
  #elif defined(PLATFORM_MACOS) {
    xsw_usage xsu = {0};
    uint64_t size = sizeof(xsu);
    if(sysctlbyname("vm.swapusage", &xsu, &size, NULL, 0) != 0) {
      perror("unable to get swap usage by calling sysctlbyname(\"vm.swapusage\",...)");
    }
    return xsu.xsu_avail;
  #endif // defined
}
uint64_t get_virtual_usage() {
  /// Fetch the memory used by this process
  #if defined(PLATFORM_WINDOWS)
    PROCESS_MEMORY_COUNTERS_EX counters;
    //GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters));
    //GetProcessMemoryInfo(GetCurrentProcess(), static_cast<PPROCESS_MEMORY_COUNTERS>(&counters), sizeof(counters));
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&counters, sizeof(counters));
    return counters.PrivateUsage;                                               // virtual memory used
  #elif defined(PLATFORM_LINUX) {
    FILE *file = fopen("/proc/self/status", "r");
    uint64_t result = 0;
    char line[128];
    while(fgets(line, 128, file) != NULL) {
      if(strncmp(line, "VmSize:", 7) == 0) {
        unsigned int const length = strlen(line);
        while(*line < '0' || *line > '9') {
          line++;
        }
        line[length - 3] = '\0';
        result = atoi(line) * 1024;                                             // value is kilobytes
        break;
      }
    }
    fclose(file);
    return result;
  #elif defined(PLATFORM_MACOS) {
    struct task_basic_info info;
    mach_msg_type_number_t info_count = TASK_BASIC_INFO_COUNT;
    if(task_info(mach_task_self(), TASK_BASIC_INFO, static_cast<task_info_t>(&info), &info_count) != KERN_SUCCESS) {
      return 0;
    }
    return info.virtual_size;
  #endif // defined
}

std::string human_readable(uint64_t amount) {
  /// Helper function to convert a size in bytes to a human readable size
  std::stringstream ss;
  uint64_t constexpr kilobyte{1'024};
  uint64_t constexpr megabyte{1'048'576};
  uint64_t constexpr gigabyte{1'073'741'824};
  uint64_t constexpr terabyte{1'099'511'627'776};
  uint64_t constexpr petabyte{1'125'899'906'842'624};
  if(amount < kilobyte) {
    ss << amount << "B";
  } else if(amount < megabyte) {
    ss << amount / kilobyte << "KB";
  } else if(amount < gigabyte) {
    ss << amount / megabyte << "MB";
  } else if(amount < terabyte) {
    ss << amount / gigabyte << "GB";
  } else if(amount < petabyte) {
    ss << amount / terabyte << "TB";
  } else {
    ss << amount / petabyte << "PB";
  }
  return ss.str();
}

std::string get_stats() {
  /// Return summary in string
  std::stringstream ss;
  ss << "MemoryStorm: Stack available " << human_readable(get_stack_available()) << '\n' <<
        "MemoryStorm: Physical usage "  << human_readable(get_physical_usage()) << ", " <<
                                           human_readable(get_physical_available()) << " available of " <<
                                           human_readable(get_physical_total()) << '\n' <<
        "MemoryStorm: Virtual usage "   << human_readable(get_virtual_usage()) << ", " <<
                                           human_readable(get_virtual_available()) << " available of " <<
                                           human_readable(get_virtual_total());
  return ss.str();
}

void dump_stats() {
  /// Dump stats summary to console
  std::cout << get_stats() << std::endl;
}

}
