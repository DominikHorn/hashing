#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include <hashing.hpp>
#include <benchmark/benchmark.h>

#include "./datasets.hpp"

const std::vector<std::int64_t> throughput_ds_sizes{200'000'000};
const std::vector<std::int64_t> throughput_ds{static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::UNIFORM)};
const std::vector<std::int64_t> scattering_ds_sizes{10'000'000};
const std::vector<std::int64_t> scattering_ds{static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::SEQUENTIAL),
                                              static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::GAPPED_10),
                                              static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::UNIFORM),
                                              static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::NORMAL),
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

   // alternatively, we could hash once per outer loop iteration. However, the overhead due to
   // gbench is too high for meaningful measurements of the fastest hashfns.
   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto hash = hashfn(key);
         const auto index = reductionfn(hash);
         benchmark::DoNotOptimize(index);
         __sync_synchronize();
      }
   }

   state.counters["dataset_size"] = dataset.size();
   state.SetLabel(Hashfn::name() + ":" + Reductionfn::name() + ":" + dataset::name(ds_id));
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
};

template<class Hashfn, class Reductionfn, class Data>
auto __BM_scattering = [](benchmark::State& state) {
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

   const auto N = 100;
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

   state.counters["dataset_size"] = dataset.size();
   state.SetLabel(Hashfn::name() + ":" + Reductionfn::name() + ":" + dataset::name(ds_id) + ":" + std::to_string(N));
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

   // alternatively, we could hash once per outer loop iteration. However, the overhead due to
   // gbench is too high for meaningful measurements of the fastest hashfns.
   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto index = hashfn(key);
         benchmark::DoNotOptimize(index);
         __sync_synchronize();
      }
   }

   state.counters["dataset_size"] = dataset.size();
   state.SetLabel(Hashfn::name() + ":" + dataset::name(ds_id));
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
};

template<class Hashfn, class Data>
auto __BM_biased_scattering = [](benchmark::State& state) {
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

   const auto N = 100;
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

   state.counters["dataset_size"] = dataset.size();
   state.SetLabel(Hashfn::name() + ":" + dataset::name(ds_id) + ":" + std::to_string(N));
   state.SetItemsProcessed(dataset.size() * static_cast<size_t>(state.iterations()));
   state.SetBytesProcessed(dataset.size() * static_cast<size_t>(state.iterations()) * sizeof(Data));
};

#define BENCHMARK_UNIFORM(Hashfn)                                                                            \
   benchmark::RegisterBenchmark("throughput_sync_synchronize",                                               \
                                __BM_throughput<Hashfn, hashing::reduction::DoNothing<T>, T>)                \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                    \
      ->Repetitions(10);                                                                                     \
   benchmark::RegisterBenchmark("throughput_sync_synchronize",                                               \
                                __BM_throughput<Hashfn, hashing::reduction::Fastrange<T>, T>)                \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                    \
      ->Repetitions(10);                                                                                     \
   benchmark::RegisterBenchmark("throughput_sync_synchronize",                                               \
                                __BM_throughput<Hashfn, hashing::reduction::Modulo<T>, T>)                   \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                    \
      ->Repetitions(10);                                                                                     \
   benchmark::RegisterBenchmark("throughput_sync_synchronize",                                               \
                                __BM_throughput<Hashfn, hashing::reduction::FastModulo<T>, T>)               \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                    \
      ->Repetitions(10);                                                                                     \
   benchmark::RegisterBenchmark("throughput_sync_synchronize",                                               \
                                __BM_throughput<Hashfn, hashing::reduction::BranchlessFastModulo<T>, T>)     \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                                    \
      ->Repetitions(10);                                                                                     \
   benchmark::RegisterBenchmark("scattering", __BM_scattering<Hashfn, hashing::reduction::Fastrange<T>, T>)  \
      ->ArgsProduct({scattering_ds_sizes, scattering_ds})                                                    \
      ->Iterations(1);                                                                                       \
   benchmark::RegisterBenchmark("scattering", __BM_scattering<Hashfn, hashing::reduction::Modulo<T>, T>)     \
      ->ArgsProduct({scattering_ds_sizes, scattering_ds})                                                    \
      ->Iterations(1);                                                                                       \
   benchmark::RegisterBenchmark("scattering", __BM_scattering<Hashfn, hashing::reduction::FastModulo<T>, T>) \
      ->ArgsProduct({scattering_ds_sizes, scattering_ds})                                                    \
      ->Iterations(1);

#define BENCHMARK_BIASED(Hashfn)                                                                  \
   benchmark::RegisterBenchmark("throughput_sync_synchronize", __BM_biased_throughput<Hashfn, T>) \
      ->ArgsProduct({throughput_ds_sizes, throughput_ds})                                         \
      ->Repetitions(10);                                                                          \
   benchmark::RegisterBenchmark("scattering_sync_synchronize", __BM_biased_scattering<Hashfn, T>) \
      ->ArgsProduct({scattering_ds_sizes, scattering_ds})                                         \
      ->Iterations(1);

template<class T>
struct DoNothing {
   static std::string name() {
      return "DoNothing" + std::to_string(sizeof(T) * 8);
   }

   constexpr forceinline T operator()(const T& key) const {
      return key;
   }
};

int main(int argc, char** argv) {
   // used to measure __sync_synchronize overhead
   {
      using T = HASH_32;

      benchmark::RegisterBenchmark("throughput_sync_synchronize",
                                   __BM_throughput<DoNothing<T>, hashing::reduction::DoNothing<T>, T>)
         ->ArgsProduct({throughput_ds_sizes, throughput_ds})
         ->Repetitions(10);

      BENCHMARK_BIASED(hashing::MultPrime32);
      BENCHMARK_BIASED(hashing::Fibonacci32);
      BENCHMARK_BIASED(hashing::FibonacciPrime32);
      BENCHMARK_UNIFORM(hashing::AquaHash<T>);
      BENCHMARK_UNIFORM(hashing::XXHash3<T>);
      BENCHMARK_UNIFORM(hashing::MurmurFinalizer<T>);
      BENCHMARK_UNIFORM(hashing::CityHash32<T>);
      BENCHMARK_UNIFORM(hashing::MeowHash32<T>);
      BENCHMARK_UNIFORM(hashing::TabulationHash<T>);
   }

   {
      using T = HASH_64;

      benchmark::RegisterBenchmark("throughput_sync_synchronize",
                                   __BM_throughput<DoNothing<T>, hashing::reduction::DoNothing<T>, T>)
         ->ArgsProduct({throughput_ds_sizes, throughput_ds})
         ->Repetitions(10);

      BENCHMARK_BIASED(hashing::MultPrime64);
      BENCHMARK_BIASED(hashing::Fibonacci64);
      BENCHMARK_BIASED(hashing::FibonacciPrime64);
      BENCHMARK_UNIFORM(hashing::AquaHash<T>);
      BENCHMARK_UNIFORM(hashing::XXHash3<T>);
      BENCHMARK_UNIFORM(hashing::MurmurFinalizer<T>);
      BENCHMARK_UNIFORM(hashing::CityHash64<T>);
      BENCHMARK_UNIFORM(hashing::MeowHash64<T>);
      BENCHMARK_UNIFORM(hashing::TabulationHash<T>);
   }

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
