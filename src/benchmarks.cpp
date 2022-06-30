#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include <hashing.hpp>
#include <benchmark/benchmark.h>

template<class Hashfn, class Reductionfn, class Data>
auto __BM_throughput = [](benchmark::State& state, const std::vector<Data>& dataset) {
   const Hashfn hashfn;
   const Reductionfn reductionfn(dataset.size());

   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto hash = hashfn(key);
         const auto index = reductionfn(hash);
         benchmark::DoNotOptimize(index);
      }
   }

   state.SetLabel(Hashfn::name() + ":" + Reductionfn::name());
   state.SetItemsProcessed(dataset.size());
   state.SetBytesProcessed(dataset.size() * sizeof(Data));
};

template<class Hashfn, class Data>
auto __BM_biased_throughput = [](benchmark::State& state, const std::vector<Data>& dataset) {
   const Hashfn hashfn(dataset.size());

   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto index = hashfn(key);
         benchmark::DoNotOptimize(index);
      }
   }

   state.SetLabel(Hashfn::name());
   state.SetItemsProcessed(dataset.size());
   state.SetBytesProcessed(dataset.size() * sizeof(Data));
};

#define BENCHMARK_THROUGHPUT(Hashfn, dataset)                                                                          \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::DoNothing<T>, T>, dataset);  \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::Fastrange<T>, T>, dataset);  \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::Modulo<T>, T>, dataset);     \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::FastModulo<T>, T>, dataset); \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::BranchlessFastModulo<T>, T>, \
                                dataset);

#define BENCHMARK_BIASED_THROUGHPUT(Hashfn, dataset) \
   benchmark::RegisterBenchmark("throughput", __BM_biased_throughput<Hashfn, T>, dataset);

template<class T>
void BenchmarkThroughput(const size_t& dataset_size) {
   // generate uniform random numbers dataset
   std::default_random_engine rng_gen(42);
   std::uniform_int_distribution<T> dist(0, 1ULL << (sizeof(T) * 8 - 1));

   std::vector<T> dataset;
   dataset.reserve(dataset_size);
   for (size_t i = 0; i < dataset_size; i++)
      dataset.push_back(dist(rng_gen));

   assert(dataset.size() == dataset_size);

   static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) == sizeof(uint32_t), "unimplemented benchmark");
   if constexpr (sizeof(T) == sizeof(uint64_t)) {
      BENCHMARK_BIASED_THROUGHPUT(hashing::MultPrime64, dataset);
      BENCHMARK_BIASED_THROUGHPUT(hashing::Fibonacci64, dataset);
      BENCHMARK_BIASED_THROUGHPUT(hashing::FibonacciPrime64, dataset);
      BENCHMARK_THROUGHPUT(hashing::AquaHash<HASH_64>, dataset);
      BENCHMARK_THROUGHPUT(hashing::XXHash3<HASH_64>, dataset);
      BENCHMARK_THROUGHPUT(hashing::MurmurFinalizer<HASH_64>, dataset);
   } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
      BENCHMARK_BIASED_THROUGHPUT(hashing::MultPrime32, dataset);
      BENCHMARK_BIASED_THROUGHPUT(hashing::Fibonacci32, dataset);
      BENCHMARK_BIASED_THROUGHPUT(hashing::FibonacciPrime32, dataset);
      BENCHMARK_THROUGHPUT(hashing::AquaHash<HASH_32>, dataset);
      BENCHMARK_THROUGHPUT(hashing::XXHash3<HASH_32>, dataset);
      BENCHMARK_THROUGHPUT(hashing::MurmurFinalizer<HASH_32>, dataset);
   }
}

int main(int argc, char** argv) {
   const size_t dataset_size = 500'000'000ULL;

   BenchmarkThroughput<uint32_t>(dataset_size);
   BenchmarkThroughput<uint64_t>(dataset_size);

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
