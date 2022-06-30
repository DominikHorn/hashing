/**
 * While this implementation is original, the idea of tabulation hashing is not.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <random>
#include <string>

#include "./convenience/builtins.hpp"

namespace hashing {

   template<class T, const T seed = 0, size_t COLUMNS = sizeof(T), size_t ROWS = std::numeric_limits<uint8_t>::max()>
   struct _TabulationHashImplementation {
      static std::string name() {
         return "tabulation_" + std::to_string(COLUMNS) + "x" + std::to_string(ROWS) + "_" +
            std::to_string(sizeof(T) * 8);
      }

      /**
       * Initializes a tabulation hash tables with random data, depending on seed
       * @tparam T hash output type, e.g., HASH_64
       * @param table, must have 255 rows
       * @param seed defaults to 1
       */
      _TabulationHashImplementation() {
         std::random_device dev;
         std::default_random_engine rng(dev());
         std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

         const auto gen_column = [&](std::array<T, ROWS>& column) {
            for (auto& r : column) {
               r = 0;
               for (size_t i = 0; i < sizeof(T); i++)
                  r |= dist(rng) << (8 * i);
            }
         };

         for (size_t c = 0; c < COLUMNS; c++)
            gen_column(table[c]);
      }

      constexpr forceinline T operator()(const T& key) const {
         T out = seed;

         for (size_t i = 0; i < sizeof(T); i++)
            out ^= table[i % COLUMNS][static_cast<uint8_t>(key >> (8 * i)) % ROWS];

         return out;
      }

     private:
      std::array<std::array<T, ROWS>, COLUMNS> table;

      void print_table() {
         std::cout << "addr\t";
         for (size_t c = 0; c < COLUMNS; c++) {
            std::cout << "col " << c << "\t";
         }
         std::cout << std::endl;

         for (size_t r = 0; r < ROWS; r++) {
            std::cout << std::hex << r << "\t\t";
            for (size_t c = 0; c < COLUMNS; c++) {
               std::cout << std::hex << table[c][r] << "\t\t";
            }
            std::cout << std::endl;
         }
      }
   };

   /**
 * Small tabulation hash, i.e., single column
 */
   template<class T, const T seed = 0>
   using SmallTabulationHash = _TabulationHashImplementation<T, seed, 1, 0xFF>;

   /**
 * Medium tabulation hash, i.e., four columns
 */
   template<class T, const T seed = 0>
   using MediumTabulationHash = _TabulationHashImplementation<T, seed, 4, 0xFF>;

   /**
 * Large tabulation hash, i.e., eight columns
 */
   template<class T, const T seed = 0>
   using LargeTabulationHash = _TabulationHashImplementation<T, seed, 8, 0xFF>;

} // namespace hashing
