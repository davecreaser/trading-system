#include <benchmark/benchmark.h>

#include "engine/version.hpp"

static void BM_ScaffoldVersion(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(engine::scaffold_version());
  }
}
BENCHMARK(BM_ScaffoldVersion);

BENCHMARK_MAIN();
