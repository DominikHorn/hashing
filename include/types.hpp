#pragma once

#include <cstdint>

#include "convenience/builtins.hpp"

/**
 * ----------------------------
 *      General & Typedefs
 * ----------------------------
 */
typedef std::uint32_t HASH_32;
typedef std::uint64_t HASH_64;

#if __GNUC__
typedef unsigned __int128 HASH_128;
constexpr forceinline HASH_128 to_hash128(HASH_64 higher, HASH_64 lower) {
   return (static_cast<HASH_128>(higher) << 64) | lower;
}
#endif

struct HASH_256 {
   HASH_64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
} align(16) packed;

