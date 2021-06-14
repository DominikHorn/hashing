#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include <hashing.hpp>
#include <benchmark/benchmark.h>

template<class Hashfn, class Reductionfn, class Data>
auto __BM_throughput = [](benchmark::State& state, const std::vector<Data>* dataset) {
   const Hashfn hashfn;
   const Reductionfn reductionfn(dataset->size());

   for (auto _ : state) {
      for (const auto& key : *dataset) {
         const auto hash = hashfn(key);
         const auto index = reductionfn(hash);
         benchmark::DoNotOptimize(index);
      }
   }

   state.SetLabel(Hashfn::name() + ":" + Reductionfn::name());
   state.SetItemsProcessed(dataset->size());
   state.SetBytesProcessed(dataset->size() * sizeof(Data));
};

#define BENCHMARK_THROUGHPUT(Hashfn, dataset)                                                                        \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput", __BM_throughput<Hashfn, hashing::reduction::Fastrange<HASH_32>, decltype(dataset)::value_type>,  \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput", __BM_throughput<Hashfn, hashing::reduction::Fastrange<HASH_64>, decltype(dataset)::value_type>,  \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput", __BM_throughput<Hashfn, hashing::reduction::Modulo<HASH_32>, decltype(dataset)::value_type>,     \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput", __BM_throughput<Hashfn, hashing::reduction::Modulo<HASH_64>, decltype(dataset)::value_type>,     \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput", __BM_throughput<Hashfn, hashing::reduction::FastModulo<HASH_32>, decltype(dataset)::value_type>, \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput", __BM_throughput<Hashfn, hashing::reduction::FastModulo<HASH_64>, decltype(dataset)::value_type>, \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput",                                                                                                  \
      __BM_throughput<Hashfn, hashing::reduction::BranchlessFastModulo<HASH_32>, decltype(dataset)::value_type>,     \
      &dataset);                                                                                                     \
   benchmark::RegisterBenchmark(                                                                                     \
      "throughput",                                                                                                  \
      __BM_throughput<Hashfn, hashing::reduction::BranchlessFastModulo<HASH_64>, decltype(dataset)::value_type>,     \
      &dataset);

int main(int argc, char** argv) {
   const size_t dataset_size = 200000000;
   std::vector<std::uint64_t> dataset;

   // generate uniform random numbers dataset
   std::default_random_engine rng_gen;
   std::uniform_int_distribution<decltype(dataset)::value_type> dist(0, static_cast<size_t>(0x1) << 50);
   dataset.resize(dataset_size);
   for (size_t i = 0; i < dataset_size; i++)
      dataset[i] = dist(rng_gen);

   BENCHMARK_THROUGHPUT(hashing::MultPrime64, dataset);
   BENCHMARK_THROUGHPUT(hashing::Fibonacci64, dataset);
   BENCHMARK_THROUGHPUT(hashing::FibonacciPrime64, dataset);
   BENCHMARK_THROUGHPUT(hashing::AquaHash<HASH_64>, dataset);
   BENCHMARK_THROUGHPUT(hashing::MurmurFinalizer<HASH_64>, dataset);

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
