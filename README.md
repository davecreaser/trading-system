# trading-system

A limit order book trading system, built from scratch in C++, to learn low-latency engineering.

An order book matched by price-time priority, driven by a real NASDAQ ITCH market-data feed, quoted against by a simple market-making strategy — with every component benchmarked. See [`CONTEXT.md`](./CONTEXT.md) for the domain vocabulary and [`docs/adr/`](./docs/adr/) for the design decisions behind it.

## Building

Requires CMake 3.24+ and a C++20 compiler.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run tests:

```sh
ctest --test-dir build --output-on-failure
```

Run benchmarks:

```sh
./build/engine_benchmarks
```

CI runs the same steps on every push/PR — see [`.github/workflows/ci.yml`](./.github/workflows/ci.yml).

## Roadmap

- [x] **Foundations**: C++20 scaffold — CMake, Catch2, Google Benchmark, CI.
- [x] **Order book**: limit order book with add/cancel/modify/match by price-time priority. Correct and fully tested first, optimised later.
- [ ] **Market-data feed**: NASDAQ ITCH parser driving the book.
- [ ] **Strategy**: a naive market maker — simplicity is the point, not chasing alpha.
- [ ] **Measurement**: latency (p50/p99/p99.9), throughput, realistic PnL including transaction costs and inventory.
- [ ] **Optimisation arc**: profile → hypothesise → change one thing → measure → keep/revert, with a before/after number for every change.

This repository tracks the engine only, synced from a private working repository at each milestone. Issue history and day-to-day notes live there.
