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
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
};

template<class Hashfn, class Reductionfn, class Data>
auto __BM_collisions = [](benchmark::State& state, const std::vector<Data>& dataset) {
   const auto N = 1000;
   std::array<size_t, N> buckets;
   std::fill(buckets.begin(), buckets.end(), 0);

   const Hashfn hashfn;
   const Reductionfn reductionfn(N);

   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto hash = hashfn(key);
         const auto index = reductionfn(hash);
         buckets[index]++;
      }
   }

   for (size_t i = 0; i < N; i++)
      state.counters["bucket_" + std::to_string(i)] = buckets[i];

   state.SetLabel(Hashfn::name() + ":" + Reductionfn::name() + ":" + std::to_string(N));
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
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
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
};

template<class Hashfn, class Data>
auto __BM_biased_collisions = [](benchmark::State& state, const std::vector<Data>& dataset) {
   const auto N = 1000;
   std::array<size_t, N> buckets;
   std::fill(buckets.begin(), buckets.end(), 0);

   const Hashfn hashfn(N);

   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto index = hashfn(key);
         buckets[index]++;
      }
   }

   for (size_t i = 0; i < N; i++)
      state.counters["bucket_" + std::to_string(i)] = buckets[i];

   state.SetLabel(Hashfn::name() + ":" + std::to_string(N));
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
};

#define BENCHMARK_UNIFORM(Hashfn, dataset)                                                                             \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::DoNothing<T>, T>, dataset)   \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::Fastrange<T>, T>, dataset)   \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::Modulo<T>, T>, dataset)      \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::FastModulo<T>, T>, dataset)  \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::BranchlessFastModulo<T>, T>, \
                                dataset)                                                                               \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("collisions", __BM_collisions<Hashfn, hashing::reduction::Fastrange<T>, T>, dataset)   \
      ->Repetitions(1);                                                                                                \
   benchmark::RegisterBenchmark("collisions", __BM_collisions<Hashfn, hashing::reduction::Modulo<T>, T>, dataset)      \
      ->Repetitions(1);                                                                                                \
   benchmark::RegisterBenchmark("collisions", __BM_collisions<Hashfn, hashing::reduction::FastModulo<T>, T>, dataset)  \
      ->Repetitions(1);

#define BENCHMARK_BIASED(Hashfn, dataset)                                                                   \
   benchmark::RegisterBenchmark("throughput", __BM_biased_collisions<Hashfn, T>, dataset)->Repetitions(10); \
   benchmark::RegisterBenchmark("collisions", __BM_biased_collisions<Hashfn, T>, dataset)->Repetitions(1);

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
      BENCHMARK_BIASED(hashing::MultPrime64, dataset);
      BENCHMARK_BIASED(hashing::Fibonacci64, dataset);
      BENCHMARK_BIASED(hashing::FibonacciPrime64, dataset);
      BENCHMARK_UNIFORM(hashing::AquaHash<HASH_64>, dataset);
      BENCHMARK_UNIFORM(hashing::XXHash3<HASH_64>, dataset);
      BENCHMARK_UNIFORM(hashing::MurmurFinalizer<HASH_64>, dataset);
      BENCHMARK_UNIFORM(hashing::CityHash64<HASH_64>, dataset);
      BENCHMARK_UNIFORM(hashing::reduction::Lower<hashing::CityHash128<HASH_64>>, dataset);
      BENCHMARK_UNIFORM(hashing::reduction::Higher<hashing::CityHash128<HASH_64>>, dataset);
      BENCHMARK_UNIFORM(hashing::MeowHash64<HASH_64>, dataset);
      BENCHMARK_UNIFORM(hashing::reduction::Lower<hashing::MeowHash128<HASH_64>>, dataset);
      BENCHMARK_UNIFORM(hashing::reduction::Higher<hashing::MeowHash128<HASH_64>>, dataset);
   } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
      BENCHMARK_BIASED(hashing::MultPrime32, dataset);
      BENCHMARK_BIASED(hashing::Fibonacci32, dataset);
      BENCHMARK_BIASED(hashing::FibonacciPrime32, dataset);
      BENCHMARK_UNIFORM(hashing::AquaHash<HASH_32>, dataset);
      BENCHMARK_UNIFORM(hashing::XXHash3<HASH_32>, dataset);
      BENCHMARK_UNIFORM(hashing::MurmurFinalizer<HASH_32>, dataset);
      BENCHMARK_UNIFORM(hashing::CityHash32<HASH_32>, dataset);
      BENCHMARK_UNIFORM(hashing::MeowHash32<HASH_32>, dataset);
   }
}

int main(int argc, char** argv) {
   const size_t dataset_size = 200'000'000ULL;

   BenchmarkThroughput<uint32_t>(dataset_size);
   BenchmarkThroughput<uint64_t>(dataset_size);

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
