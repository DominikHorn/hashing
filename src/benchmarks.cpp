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

using namespace hashing;
using namespace reduction;

#define BENCHMARK_THROUGHPUT(Hashfn)                               \
   BENCHMARK_TEMPLATE(BM_THROUGHPUT, Hashfn, Fastrange<HASH_32>);  \
   BENCHMARK_TEMPLATE(BM_THROUGHPUT, Hashfn, Fastrange<HASH_64>);  \
   BENCHMARK_TEMPLATE(BM_THROUGHPUT, Hashfn, FastModulo<HASH_32>); \
   BENCHMARK_TEMPLATE(BM_THROUGHPUT, Hashfn, FastModulo<HASH_64>);

// Register benchmarks
BENCHMARK_THROUGHPUT(MultPrime32)
BENCHMARK_THROUGHPUT(MultPrime64)
BENCHMARK_THROUGHPUT(MurmurFinalizer<HASH_32>)
BENCHMARK_THROUGHPUT(MurmurFinalizer<HASH_64>)
BENCHMARK_THROUGHPUT(AquaHash<HASH_32>)
BENCHMARK_THROUGHPUT(AquaHash<HASH_64>)

// Run the benchmarks
BENCHMARK_MAIN();
