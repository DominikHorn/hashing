#include <iostream>

#include <hashing.hpp>
#include <benchmark/benchmark.h>

template<class Hashfn, class Reductionfn>
void BM_THROUGHPUT(benchmark::State& state) {
   Hashfn hashfn;
   Reductionfn reductionfn(200000000);

   // TODO: hash actual dataset
   std::uint64_t key = 0;

   for (auto _ : state) {
      const auto hash = hashfn(key);
      const auto index = reductionfn(hash);
      benchmark::DoNotOptimize(index);
      key++;
   }
}

// Register benchmarks
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MultPrime32, hashing::reduction::FastModulo<HASH_32>);
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MultPrime64, hashing::reduction::FastModulo<HASH_32>);
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MurmurFinalizer<HASH_32>, hashing::reduction::FastModulo<HASH_32>);
BENCHMARK_TEMPLATE(BM_THROUGHPUT, hashing::MurmurFinalizer<HASH_64>, hashing::reduction::FastModulo<HASH_32>);

// Run the benchmarks
BENCHMARK_MAIN();
