#include <benchmark/benchmark.h>

static void BM_Incremet(benchmark::State& state) {
  // Setup
  int a = 0;

  // Benchmark code is within this loop
  for (auto _ : state) {
    const auto res = a++;
    benchmark::DoNotOptimize(res);
  }
}

// Register benchmarks
BENCHMARK(BM_Incremet);

// Run the benchmarks
BENCHMARK_MAIN();
