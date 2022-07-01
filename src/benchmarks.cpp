#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include <hashing.hpp>
#include <benchmark/benchmark.h>

#include "./datasets.hpp"

const std::vector<std::int64_t> throughput_ds_sizes{200'000'000};
const std::vector<std::int64_t> throughput_ds{static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::UNIFORM)};
const std::vector<std::int64_t> collision_ds_sizes{100'000'000};
const std::vector<std::int64_t> collision_ds{static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::SEQUENTIAL),
                                             static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::GAPPED_10),
                                             static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::UNIFORM),
                                             static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::BOOKS),
                                             static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::FB),
                                             static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::OSM),
                                             static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::WIKI)};

template<class Hashfn, class Reductionfn, class Data>
auto __BM_throughput = [](benchmark::State& state) {
   const auto ds_size = state.range(0);
   const auto ds_id = static_cast<dataset::ID>(state.range(1));

   // load dataset
   auto dataset = dataset::load_cached(ds_id, ds_size);
   if (dataset.empty())
      throw std::runtime_error("benchmark dataset empty");

   // shuffle dataset
   std::random_device rd_dev;
   std::default_random_engine rng(rd_dev());
   std::shuffle(dataset.begin(), dataset.end(), rng);

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
auto __BM_collisions = [](benchmark::State& state) {
   const auto ds_size = state.range(0);
   const auto ds_id = static_cast<dataset::ID>(state.range(1));

   // load dataset
   auto dataset = dataset::load_cached(ds_id, ds_size);
   if (dataset.empty())
      throw std::runtime_error("benchmark dataset empty");

   // shuffle dataset
   std::random_device rd_dev;
   std::default_random_engine rng(rd_dev());
   std::shuffle(dataset.begin(), dataset.end(), rng);

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
auto __BM_biased_throughput = [](benchmark::State& state) {
   const auto ds_size = state.range(0);
   const auto ds_id = static_cast<dataset::ID>(state.range(1));

   // load dataset
   auto dataset = dataset::load_cached(ds_id, ds_size);
   if (dataset.empty())
      throw std::runtime_error("benchmark dataset empty");

   // shuffle dataset
   std::random_device rd_dev;
   std::default_random_engine rng(rd_dev());
   std::shuffle(dataset.begin(), dataset.end(), rng);

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
auto __BM_biased_collisions = [](benchmark::State& state) {
   const auto ds_size = state.range(0);
   const auto ds_id = static_cast<dataset::ID>(state.range(1));

   // load dataset
   auto dataset = dataset::load_cached(ds_id, ds_size);
   if (dataset.empty())
      throw std::runtime_error("benchmark dataset empty");

   // shuffle dataset
   std::random_device rd_dev;
   std::default_random_engine rng(rd_dev());
   std::shuffle(dataset.begin(), dataset.end(), rng);

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

#define BENCHMARK_UNIFORM(Hashfn)                                                                                      \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::DoNothing<T>, T>)            \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                              \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::Fastrange<T>, T>)            \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                              \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::Modulo<T>, T>)               \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                              \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::FastModulo<T>, T>)           \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                              \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, hashing::reduction::BranchlessFastModulo<T>, T>) \
      ->Repetitions(10);                                                                                               \
   benchmark::RegisterBenchmark("collisions", __BM_collisions<Hashfn, hashing::reduction::Fastrange<T>, T>)            \
      ->ArgsProduct({collision_ds_sizes, collision_ds})                                                                \
      ->Iterations(1);                                                                                                 \
   benchmark::RegisterBenchmark("collisions", __BM_collisions<Hashfn, hashing::reduction::Modulo<T>, T>)               \
      ->ArgsProduct({collision_ds_sizes, collision_ds})                                                                \
      ->Iterations(1);                                                                                                 \
   benchmark::RegisterBenchmark("collisions", __BM_collisions<Hashfn, hashing::reduction::FastModulo<T>, T>)           \
      ->ArgsProduct({collision_ds_sizes, collision_ds})                                                                \
      ->Iterations(1);

#define BENCHMARK_BIASED(Hashfn)                                                 \
   benchmark::RegisterBenchmark("throughput", __BM_biased_collisions<Hashfn, T>) \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                        \
      ->Repetitions(10);                                                         \
   benchmark::RegisterBenchmark("collisions", __BM_biased_collisions<Hashfn, T>) \
      ->ArgsProduct({collision_ds_sizes, collision_ds})                          \
      ->Iterations(1);

int main(int argc, char** argv) {
   {
      using T = HASH_32;

      BENCHMARK_BIASED(hashing::MultPrime32);
      BENCHMARK_BIASED(hashing::Fibonacci32);
      BENCHMARK_BIASED(hashing::FibonacciPrime32);
      BENCHMARK_UNIFORM(hashing::AquaHash<T>);
      BENCHMARK_UNIFORM(hashing::XXHash3<T>);
      BENCHMARK_UNIFORM(hashing::MurmurFinalizer<T>);
      BENCHMARK_UNIFORM(hashing::CityHash32<T>);
      BENCHMARK_UNIFORM(hashing::MeowHash32<T>);
      BENCHMARK_UNIFORM(hashing::SmallTabulationHash<T>);
      BENCHMARK_UNIFORM(hashing::MediumTabulationHash<T>);
   }

   {
      using T = HASH_64;

      BENCHMARK_BIASED(hashing::MultPrime64);
      BENCHMARK_BIASED(hashing::Fibonacci64);
      BENCHMARK_BIASED(hashing::FibonacciPrime64);
      BENCHMARK_UNIFORM(hashing::AquaHash<T>);
      BENCHMARK_UNIFORM(hashing::XXHash3<T>);
      BENCHMARK_UNIFORM(hashing::MurmurFinalizer<T>);
      BENCHMARK_UNIFORM(hashing::CityHash64<T>);
      BENCHMARK_UNIFORM(hashing::reduction::Lower<hashing::CityHash128<T>>);
      BENCHMARK_UNIFORM(hashing::reduction::Higher<hashing::CityHash128<T>>);
      BENCHMARK_UNIFORM(hashing::MeowHash64<T>);
      BENCHMARK_UNIFORM(hashing::reduction::Lower<hashing::MeowHash128<T>>);
      BENCHMARK_UNIFORM(hashing::reduction::Higher<hashing::MeowHash128<T>>);
      BENCHMARK_UNIFORM(hashing::SmallTabulationHash<T>);
      BENCHMARK_UNIFORM(hashing::MediumTabulationHash<T>);
      BENCHMARK_UNIFORM(hashing::LargeTabulationHash<T>);
   }

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
