#pragma once

#include <cstdint>

#include "convenience/builtins.hpp"

/**
 * ----------------------------
 *      General & Typedefs
 * ----------------------------
 */
using HASH_32 = std::uint32_t;
using HASH_64 = std::uint64_t;

#if __GNUC__
using HASH_128 = unsigned __int128;
constexpr forceinline HASH_128 to_hash128(HASH_64 higher, HASH_64 lower) {
   return (static_cast<HASH_128>(higher) << 64) | lower;
}
#endif

struct HASH_256 {
   HASH_64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
} alignit(16) packit;

