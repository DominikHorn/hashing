#include <iostream>

#include <hashing.hpp>
#include <benchmark/benchmark.h>

template<class Hashfn>
void BM_THROUGHPUT(benchmark::State& state) {
   Hashfn hashfn;

   // Benchmark code is within this loop
   std::uint64_t key = 0;
   for (auto _ : state) {
      const auto index = hashfn(key++);
      benchmark::DoNotOptimize(index);
   }
}

// Register benchmarks
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MultPrime32);
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MultPrime64);
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MurmurFinalizer<HASH_32>);
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MurmurFinalizer<HASH_64>);

// Run the benchmarks
BENCHMARK_MAIN();
