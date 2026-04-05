#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <iostream>
#include <sstream>

#include "memorystorm.h"

using namespace memorystorm;

// ---------------------------------------------------------------------------
// human_readable() – pure function, fully deterministic
// ---------------------------------------------------------------------------

TEST_CASE("human_readable: byte range (< 1 KB)", "[human_readable]") {
  CHECK(human_readable(0)    == "0B");
  CHECK(human_readable(1)    == "1B");
  CHECK(human_readable(512)  == "512B");
  CHECK(human_readable(1023) == "1023B");
}

TEST_CASE("human_readable: fractional KB range (1 KB – 9.99 KB)", "[human_readable]") {
  // Uses setprecision(2) with float division – 2 significant digits
  CHECK(human_readable(1024) == "1KB");   // 1.0 KB
  CHECK(human_readable(2048) == "2KB");   // 2.0 KB
  CHECK(human_readable(5120) == "5KB");   // 5.0 KB
  CHECK(human_readable(9216) == "9KB");   // 9.0 KB
  CHECK(human_readable(9728) == "9.5KB"); // 9.5 KB
}

TEST_CASE("human_readable: integer KB range (10 KB – 999 KB)", "[human_readable]") {
  CHECK(human_readable(10240)   == "10KB");   // exactly 10 KB
  CHECK(human_readable(102400)  == "100KB");  // 100 KB
  CHECK(human_readable(512000)  == "500KB");  // 500 KB
  CHECK(human_readable(1047552) == "1023KB"); // 1023 KB (just below 1 MB)
}

TEST_CASE("human_readable: fractional MB range (1 MB – 9.99 MB)", "[human_readable]") {
  CHECK(human_readable(1048576) == "1MB");  // 1.0 MB
  CHECK(human_readable(2097152) == "2MB");  // 2.0 MB
  CHECK(human_readable(5242880) == "5MB");  // 5.0 MB
}

TEST_CASE("human_readable: integer MB range (10 MB – 999 MB)", "[human_readable]") {
  CHECK(human_readable(10485760)  == "10MB");  // 10 MB
  CHECK(human_readable(104857600) == "100MB"); // 100 MB
  CHECK(human_readable(524288000) == "500MB"); // 500 MB
}

TEST_CASE("human_readable: fractional GB range (1 GB – 9.99 GB)", "[human_readable]") {
  CHECK(human_readable(UINT64_C(1073741824)) == "1GB");  // 1.0 GB
  CHECK(human_readable(UINT64_C(2147483648)) == "2GB");  // 2.0 GB
  CHECK(human_readable(UINT64_C(5368709120)) == "5GB");  // 5.0 GB
}

TEST_CASE("human_readable: integer GB range (10 GB – 999 GB)", "[human_readable]") {
  CHECK(human_readable(UINT64_C(10737418240))  == "10GB");  // 10 GB
  CHECK(human_readable(UINT64_C(107374182400)) == "100GB"); // 100 GB
}

TEST_CASE("human_readable: fractional TB range (1 TB – 9.99 TB)", "[human_readable]") {
  CHECK(human_readable(UINT64_C(1099511627776)) == "1TB");  // 1.0 TB
  CHECK(human_readable(UINT64_C(2199023255552)) == "2TB");  // 2.0 TB
}

TEST_CASE("human_readable: integer TB range (10 TB – 999 TB)", "[human_readable]") {
  CHECK(human_readable(UINT64_C(10995116277760)) == "10TB"); // 10 TB
}

TEST_CASE("human_readable: fractional PB range (1 PB – 9.99 PB)", "[human_readable]") {
  CHECK(human_readable(UINT64_C(1125899906842624)) == "1PB");  // 1.0 PB
}

TEST_CASE("human_readable: large PB range (>= 10 PB)", "[human_readable]") {
  CHECK(human_readable(UINT64_C(11258999068426240)) == "10PB"); // 10 PB
}

TEST_CASE("human_readable: boundary values", "[human_readable]") {
  // Just below each threshold should use lower unit
  CHECK_THAT(human_readable(1023),    Catch::Matchers::EndsWith("B"));
  CHECK_THAT(human_readable(10239),   Catch::Matchers::EndsWith("KB"));
  CHECK_THAT(human_readable(1048575), Catch::Matchers::EndsWith("KB"));
  // Just at threshold should use higher unit
  CHECK_THAT(human_readable(1024),    Catch::Matchers::EndsWith("KB"));
  CHECK_THAT(human_readable(10240),   Catch::Matchers::EndsWith("KB"));
  CHECK_THAT(human_readable(1048576), Catch::Matchers::EndsWith("MB"));
}

// ---------------------------------------------------------------------------
// Memory query functions – sanity / contract checks
// ---------------------------------------------------------------------------

TEST_CASE("get_stack_available: returns a positive non-zero value", "[memory][stack]") {
  auto const available{get_stack_available()};
  CHECK(available > 0);
}

TEST_CASE("get_physical_total: returns a positive non-zero value", "[memory][physical]") {
  auto const total{get_physical_total()};
  CHECK(total > 0);
}

TEST_CASE("get_physical_available: returns a non-negative value no greater than total", "[memory][physical]") {
  auto const total{get_physical_total()};
  auto const available{get_physical_available()};
  CHECK(available <= total);
}

TEST_CASE("get_physical_usage: returns a positive non-zero value", "[memory][physical]") {
  auto const usage{get_physical_usage()};
  CHECK(usage > 0);
}

TEST_CASE("get_physical_usage: does not exceed total physical memory", "[memory][physical]") {
  auto const total{get_physical_total()};
  auto const usage{get_physical_usage()};
  CHECK(usage <= total);
}

TEST_CASE("get_virtual_total: returns a positive non-zero value", "[memory][virtual]") {
  auto const total{get_virtual_total()};
  CHECK(total > 0);
}

TEST_CASE("get_virtual_available: returns a non-negative value no greater than virtual total", "[memory][virtual]") {
  auto const total{get_virtual_total()};
  auto const available{get_virtual_available()};
  CHECK(available <= total);
}

TEST_CASE("get_virtual_usage: returns a positive non-zero value", "[memory][virtual]") {
  auto const usage{get_virtual_usage()};
  CHECK(usage > 0);
}

TEST_CASE("get_virtual_total: is at least as large as physical total", "[memory][physical][virtual]") {
  // Virtual memory (RAM + swap) should be >= physical RAM
  auto const physical_total{get_physical_total()};
  auto const virtual_total{get_virtual_total()};
  CHECK(virtual_total >= physical_total);
}

TEST_CASE("virtual usage is at least physical usage", "[memory][physical][virtual]") {
  // Virtual address space used is typically >= physical (resident set size)
  auto const physical_usage{get_physical_usage()};
  auto const virtual_usage{get_virtual_usage()};
  CHECK(virtual_usage >= physical_usage);
}

// ---------------------------------------------------------------------------
// Memory values are sensible magnitudes (1 MB .. 1 TB range for a CI host)
// ---------------------------------------------------------------------------

TEST_CASE("get_physical_total: plausible for a CI host (>= 64 MB)", "[memory][physical]") {
  auto const total{get_physical_total()};
  auto const min_expected{UINT64_C(64) * 1024 * 1024}; // 64 MB
  CHECK(total >= min_expected);
}

TEST_CASE("get_physical_total: plausible upper bound (< 1 TB)", "[memory][physical]") {
  auto const total{get_physical_total()};
  auto const max_expected{UINT64_C(1024) * 1024 * 1024 * 1024}; // 1 TB
  CHECK(total < max_expected);
}

// ---------------------------------------------------------------------------
// get_stats() and dump_stats()
// ---------------------------------------------------------------------------

TEST_CASE("get_stats: returns a non-empty string", "[stats]") {
  auto const stats{get_stats()};
  CHECK_FALSE(stats.empty());
}

TEST_CASE("get_stats: contains expected section headings", "[stats]") {
  auto const stats{get_stats()};
  CHECK_THAT(stats, Catch::Matchers::ContainsSubstring("MemoryStorm:"));
  CHECK_THAT(stats, Catch::Matchers::ContainsSubstring("Stack available"));
  CHECK_THAT(stats, Catch::Matchers::ContainsSubstring("Physical usage"));
  CHECK_THAT(stats, Catch::Matchers::ContainsSubstring("Virtual usage"));
}

TEST_CASE("get_stats: each memory value appears human-readable (ends with a unit suffix)", "[stats]") {
  auto const stats{get_stats()};
  // The output must contain at least one size unit token
  bool has_unit{
    stats.find("B")  != std::string::npos ||
    stats.find("KB") != std::string::npos ||
    stats.find("MB") != std::string::npos ||
    stats.find("GB") != std::string::npos
  };
  CHECK(has_unit);
}

TEST_CASE("dump_stats: does not throw", "[stats]") {
  // Redirect stdout so the test output stays clean
  std::streambuf *old_buf{std::cout.rdbuf()};
  std::ostringstream devnull;
  std::cout.rdbuf(devnull.rdbuf());

  CHECK_NOTHROW(dump_stats());

  std::cout.rdbuf(old_buf);
}

TEST_CASE("dump_stats: writes non-empty output", "[stats]") {
  std::streambuf *old_buf{std::cout.rdbuf()};
  std::ostringstream captured;
  std::cout.rdbuf(captured.rdbuf());

  dump_stats();

  std::cout.rdbuf(old_buf);
  CHECK_FALSE(captured.str().empty());
}
