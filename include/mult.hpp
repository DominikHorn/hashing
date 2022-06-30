#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <string>

#include "convenience/builtins.hpp"
#include "types.hpp"

namespace hashing {
   namespace _ {
      /**
       * Multiplicative hashing, i.e., N * (x * A % 2^w) >> w
       *
       * @tparam T output type
       * @tparam A magic constant
       */
      template<class T, T A, const char* BaseName>
      struct MultiplicationHash {
         MultiplicationHash(const T& N = std::numeric_limits<T>::max()) : N(N) {}

         static std::string name() {
            return BaseName + std::to_string(sizeof(T) * 8);
         }

         /**
          * hash = (key * constant % 2^w) >> (w - p), where w = sizeof(T) * 8
          * i.e., take the highest p bits from the w-bit multiplication
          * key * constant (ignore overflow)
          *
          * @param key the key to hash
          */
         constexpr forceinline T operator()(const T& key) const {
            if constexpr (sizeof(T) == 4)
               return hash32(key);
            return hash64(key);
         };

        private:
         static const T w = sizeof(T) * 8; // bit width of output
         const T N;

         constexpr forceinline std::uint32_t hash32(const std::uint32_t& x) const {
            // temporary register for computations
            uint64_t tmp64;

            // 1. x*A mod 2^w. Downcast is optional
            // since x and A are already uint32_t
            tmp64 = (uint32_t) (x * A);

            // 2. multiply by N
            tmp64 *= N;

            // 3. shift to get result
            return tmp64 >> w;
         }

         constexpr forceinline std::uint64_t hash64(const std::uint64_t& x) const {
            // temporary register for computations
            HASH_128 tmp128;

            // 1. x*A mod 2^w. Downcast is optional
            // since x and A are already uint64_t
            tmp128 = (uint64_t) (x * A);

            // 2. multiply by N
            tmp128 *= N;

            // 3. shift to get result
            return tmp128 >> w;
         }
      };

      const char MULT_PRIME[] = "MultHash";
      const char MULT_FIBONACCI[] = "MultFibonacci";
      const char MULT_FIBONACCI_PRIME[] = "MultFibonacciPrime";
   } // namespace _

   /// Multiplicative 32-bit hashing with prime constants
   using MultPrime32 = _::MultiplicationHash<HASH_32, 0x238EF8E3LU, _::MULT_PRIME>;
   /// Multiplicative 64-bit hashing with prime constants
   using MultPrime64 = _::MultiplicationHash<HASH_64, 0xC7455FEC83DD661FLLU, _::MULT_PRIME>;

   /// Multiplicative 32-bit hashing with constants derived from the golden ratio
   using Fibonacci32 = _::MultiplicationHash<HASH_32, 0x9E3779B9LU, _::MULT_FIBONACCI>;
   /// Multiplicative 64-bit hashing with constants derived from the golden ratio
   using Fibonacci64 = _::MultiplicationHash<HASH_64, 0x9E3779B97F4A7C15LLU, _::MULT_FIBONACCI>;

   /// Multiplicative 32-bit hashing with prime constants derived from the golden ratio
   using FibonacciPrime32 = _::MultiplicationHash<HASH_32, 0x9e3779b1LU, _::MULT_FIBONACCI_PRIME>;
   /// Multiplicative 64-bit hashing with prime constants derived from the golden ratio
   using FibonacciPrime64 = _::MultiplicationHash<HASH_64, 0x9E3779B97F4A7C55LLU, _::MULT_FIBONACCI_PRIME>;
} // namespace hashing
