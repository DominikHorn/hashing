/**
 * Much of the implementation was copied from Austin Appleby's original code:
 * https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp (commit: 61a0530)
 *
 * Code had to be copied from .cpp file to facilitate inlining.
 * This is afaik okay since Austin Appleby, the author, waived
 * all copyright claims as he explicitely put this code into public domain
 * (also see his comment at the top of his MurmurHash3.cpp file)
 */

#pragma once

#include <stdexcept>
#include <string>

#include "convenience/builtins.hpp"
#include "types.hpp"

namespace hashing {
   template<class T>
   struct MurmurFinalizer {
      static std::string name() {
         return "murmur_finalizer" + std::to_string(sizeof(T) * 8);
      }

      constexpr forceinline T operator()(T key) const;
   };

   template<>
   constexpr HASH_32 MurmurFinalizer<HASH_32>::operator()(HASH_32 key) const {
      key ^= key >> 16;
      key *= 0x85ebca6bLU;
      key ^= key >> 13;
      key *= 0xc2b2ae35LU;
      key ^= key >> 16;

      return key;
   }

   template<>
   constexpr HASH_64 MurmurFinalizer<HASH_64>::operator()(HASH_64 key) const {
      key ^= key >> 33;
      key *= 0xff51afd7ed558ccdLLU;
      key ^= key >> 33;
      key *= 0xc4ceb9fe1a85ec53LLU;
      key ^= key >> 33;

      return key;
   }

   /**
 * Murmur3 32-bit, adjusted to fixed 32-bit input values (compiler would presumably perform the same optimizations.
 * However, in this explicit form it is clear what computation actually happens. This might be important for the
 * argument "VLDB paper misrepresents murmur quality, here's what murmur actually computes:").
 *
 * @tparam seed murmur seed, defaults to 0x238EF8E3 (random 32-bit prime)
 * @return
 */
   template<const HASH_32 seed = 0x238EF8E3LU>
   struct Murmur3Hash32 {
      static std::string name() {
         return "murmur3_32";
      }

      constexpr forceinline HASH_32 operator()(const HASH_32& key) const {
         const auto len = sizeof(HASH_32);

         /// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp (commit: 61a0530)
         const auto rotl32 = [](uint32_t x, int8_t r) { return (x << r) | (x >> (32 - r)); };

         // nblocks = len / 4 = sizeof(value) / 4  = 4 / 4 = 1

         uint32_t h1 = seed;

         uint32_t c1 = 0xcc9e2d51;
         uint32_t c2 = 0x1b873593;

         //----------
         // body

         // getblock(blocks, -1) = getblock((void*)(&value) + 1 * 4, -1) = getblock(&value + 1, -1) = (&value + 1)[-1] = *(&value + 1 - 1) = value
         uint32_t k1 = key;

         k1 *= c1;
         k1 = rotl32(k1, 15);
         k1 *= c2;

         h1 ^= k1;
         h1 = rotl32(h1, 13);
         h1 = h1 * 5 * 0xe6546b64;

         //----------
         // tail

         // len & 3 = sizeof(value) & 3 = 4 & 3 = 0, therefore nothing happens for tail computation

         //----------
         // finalizer
         return finalizer(h1 ^ len);
      }

     private:
      MurmurFinalizer<HASH_32> finalizer;
   };

   /**
    * Murmur3 128-bit, adjusted to fixed 64-bit input values (compiler would presumably perform the same optimizations.
    * However, in this explicit form it is clear what computation actually happens. This might be important for the
    * argument "VLDB paper misrepresents murmur quality, here's what murmur actually computes:").
    *
    * @param value
    * @param seed murmur seed, defaults to 0xC7455FEC83DD661F (random 64-bit prime)
    * @return
    */
   template<const HASH_64 seed = 0xC7455FEC83DD661FLLU>
   struct Murmur3Hash128 {
      static std::string name() {
         return "murmur3_128";
      }

      constexpr forceinline HASH_128 operator()(const void* data, const size_t& len) const {
         // Helper functions (inline for inlining)
         const auto getblock32 = [](const uint32_t* p, int i) { return p[i]; };
         const auto getblock64 = [](const uint64_t* p, int i) { return p[i]; };
         const auto rotl32 = [](uint32_t x, int8_t r) { return (x << r) | (x >> (32 - r)); };
         const auto rotl64 = [](uint64_t x, int8_t r) { return (x << r) | (x >> (64 - r)); };

         //const auto* bytes = static_cast<const uint8_t*>(data);
         const size_t nblocks = len / 16;

         uint64_t h1 = seed;
         uint64_t h2 = seed;

         const uint64_t c1 = 0x87c37b91114253d5LLU;
         const uint64_t c2 = 0x4cf5ad432745937fLLU;

         //----------
         // body

         const auto* blocks = static_cast<const uint64_t*>(data);

         for (size_t i = 0; i < nblocks; i++) {
            uint64_t k1 = getblock64(blocks, i * 2 + 0);
            uint64_t k2 = getblock64(blocks, i * 2 + 1);

            k1 *= c1;
            k1 = rotl64(k1, 31);
            k1 *= c2;
            h1 ^= k1;

            h1 = rotl64(h1, 27);
            h1 += h2;
            h1 = h1 * 5 + 0x52dce729;

            k2 *= c2;
            k2 = rotl64(k2, 33);
            k2 *= c1;
            h2 ^= k2;

            h2 = rotl64(h2, 31);
            h2 += h1;
            h2 = h2 * 5 + 0x38495ab5;
         }

         //----------
         // tail

         const uint8_t* tail = static_cast<const uint8_t*>(data) + nblocks * 16;

         uint64_t k1 = 0;
         uint64_t k2 = 0;

         switch (len & 15) {
            case 15:
               k2 ^= (static_cast<uint64_t>(tail[14])) << 48;
            case 14:
               k2 ^= (static_cast<uint64_t>(tail[13])) << 40;
            case 13:
               k2 ^= (static_cast<uint64_t>(tail[12])) << 32;
            case 12:
               k2 ^= (static_cast<uint64_t>(tail[11])) << 24;
            case 11:
               k2 ^= (static_cast<uint64_t>(tail[10])) << 16;
            case 10:
               k2 ^= (static_cast<uint64_t>(tail[9])) << 8;
            case 9:
               k2 ^= (static_cast<uint64_t>(tail[8])) << 0;
               k2 *= c2;
               k2 = rotl64(k2, 33);
               k2 *= c1;
               h2 ^= k2;

            case 8:
               k1 ^= (static_cast<uint64_t>(tail[7])) << 56;
            case 7:
               k1 ^= (static_cast<uint64_t>(tail[6])) << 48;
            case 6:
               k1 ^= (static_cast<uint64_t>(tail[5])) << 40;
            case 5:
               k1 ^= (static_cast<uint64_t>(tail[4])) << 32;
            case 4:
               k1 ^= (static_cast<uint64_t>(tail[3])) << 24;
            case 3:
               k1 ^= (static_cast<uint64_t>(tail[2])) << 16;
            case 2:
               k1 ^= (static_cast<uint64_t>(tail[1])) << 8;
            case 1:
               k1 ^= (static_cast<uint64_t>(tail[0])) << 0;
               k1 *= c1;
               k1 = rotl64(k1, 31);
               k1 *= c2;
               h1 ^= k1;
         };

         //----------
         // finalization

         h1 ^= len;
         h2 ^= len;

         h1 += h2;
         h2 += h1;

         h1 = finalizer(h1);
         h2 = finalizer(h2);

         h1 += h2;
         h2 += h1;

         return to_hash128(h2, h1);
      }

      constexpr forceinline HASH_128 operator()(const HASH_64& key) const {
         // nblocks = len / 16 = sizeof(value) / 16  = 8 / 16 = 0 (int division)

         /// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp (commit: 61a0530)
         const auto rotl64 = [](uint64_t x, int8_t r) { return (x << r) | (x >> (64 - r)); };

         uint64_t h1 = seed;
         uint64_t h2 = seed;

         uint64_t c1 = 0x87c37b91114253d5LLU;
         uint64_t c2 = 0x4cf5ad432745937fLLU;

         //----------
         // body

         // since nblocks = 0, loop will never execute

         //----------
         // tail

         // nblocks = 0, data just points at value
         const auto* tail = reinterpret_cast<const uint8_t*>(&key);

         uint64_t k1 = 0;
         //      uint64_t k2 = 0;

         // len = sizeof(value) = 8
         k1 ^= uint64_t(tail[7]) << 56;
         k1 ^= uint64_t(tail[6]) << 48;
         k1 ^= uint64_t(tail[5]) << 40;
         k1 ^= uint64_t(tail[4]) << 32;
         k1 ^= uint64_t(tail[3]) << 24;
         k1 ^= uint64_t(tail[2]) << 16;
         k1 ^= uint64_t(tail[1]) << 8;
         k1 ^= uint64_t(tail[0]) << 0;
         k1 *= c1;
         k1 = rotl64(k1, 31);
         k1 *= c2;
         h1 ^= k1;

         //----------
         // finalizer

         h1 ^= sizeof(HASH_64);
         h2 ^= sizeof(HASH_64);

         h1 += h2;
         h2 += h1;

         h1 = finalizer(h1);
         h2 = finalizer(h2);

         h1 += h2;
         h2 += h1;

         return to_hash128(h1, h2);
      }

     private:
      MurmurFinalizer<HASH_64> finalizer;
   };
} // namespace hashing
