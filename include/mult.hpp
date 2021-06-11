#pragma once

#include <cassert>
#include <string>

#include "convenience/builtins.hpp"
#include "types.hpp"

namespace hashing {
   namespace _ {
      /**
       * Multiplicative hashing, i.e., (x * constant % 2^w) >> (w - p)
       *
       * @tparam constant magic constant used to multiply
       * @tparam p how many bits the result should have, i.e., result value \in [0, 2^p].
       *   NOTE: 0 <= p <= sizeof(T)*8. Narrowing result to exactly the amount of required bits should
       *   improve overall bit quality (avalanche)
       */
      template<class T, const T constant, const char* base_name, const uint8_t p = sizeof(T) * 8>
      struct MultiplicationHash {
         static std::string name() {
            return base_name + (p < sizeof(T) * 8 ? "_shift" + std::to_string(sizeof(T) * 8 - p) + "_" : "") +
               std::to_string(sizeof(T) * 8);
         }

         /**
          * hash = (key * constant % 2^w) >> (w - p), where w = sizeof(T) * 8
          * i.e., take the highest p bits from the w-bit multiplication
          * key * constant (ignore overflow)
          *
          * @param key the key to hash
          */
         constexpr forceinline T operator()(const T& key) const {
            constexpr auto t = sizeof(T) * 8;
            assert(p >= 0 && p <= t);
            return (key * constant) >> (t - p);
         }
      };

      const char MULT_PRIME[] = "mult_prime";
      const char MULT_FIBONACCI[] = "mult_fibonacci";
      const char MULT_FIBONACCI_PRIME[] = "mult_fibonacci_prime";
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

   /// Multiplicative 32-bit hashing with prime constants
   template<const uint8_t p = sizeof(HASH_32) * 8>
   using MultPrimeShift32 = _::MultiplicationHash<HASH_32, 0x238EF8E3LU, _::MULT_PRIME, p>;
   /// Multiplicative 64-bit hashing with prime constants
   template<const uint8_t p = sizeof(HASH_64) * 8>
   using MultPrimeShift64 = _::MultiplicationHash<HASH_64, 0xC7455FEC83DD661FLLU, _::MULT_PRIME, p>;

   /// Multiplicative 32-bit hashing with constants derived from the golden ratio
   template<const uint8_t p = sizeof(HASH_32) * 8>
   using FibonacciShift32 = _::MultiplicationHash<HASH_32, 0x9E3779B9LU, _::MULT_FIBONACCI, p>;
   /// Multiplicative 64-bit hashing with constants derived from the golden ratio
   template<const uint8_t p = sizeof(HASH_64) * 8>
   using FibonacciShift64 = _::MultiplicationHash<HASH_64, 0x9E3779B97F4A7C15LLU, _::MULT_FIBONACCI, p>;

   /// Multiplicative 32-bit hashing with prime constants derived from the golden ratio
   template<const uint8_t p = sizeof(HASH_32) * 8>
   using FibonacciPrimeShift32 = _::MultiplicationHash<HASH_32, 0x9e3779b1LU, _::MULT_FIBONACCI_PRIME, p>;
   /// Multiplicative 64-bit hashing with prime constants derived from the golden ratio
   template<const uint8_t p = sizeof(HASH_64) * 8>
   using FibonacciPrimeShift64 = _::MultiplicationHash<HASH_64, 0x9E3779B97F4A7C55LLU, _::MULT_FIBONACCI_PRIME, p>;
} // namespace hashing
