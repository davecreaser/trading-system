# Engine

A limit order book trading system, built from scratch in C++, replaying real NASDAQ TotalView-ITCH 5.0 market data.

## Build

```
cmake -S . -B build
cmake --build build
```

## Run the tests

```
./build/engine_tests
```

## Run the benchmarks

```
./build/engine_benchmarks
```

## Formatting and static analysis

```
cmake --build build --target format        # apply clang-format
cmake --build build --target format-check   # check formatting, no changes
cmake --build build --target tidy           # clang-tidy (misc-include-cleaner)
```

## Run the real-data demo

`engine_demo` replays a real trading day's NASDAQ ITCH feed through the order book and reports what happened — messages processed, AAPL's Stock Locate, fills produced, top-of-book changes, and validation results against ITCH's own reported executions.

It needs the real sample file, which isn't bundled in this repo (it's several gigabytes) and is never downloaded automatically:

1. Download NASDAQ's public ITCH 5.0 sample file (a full historical trading day, Jan 30 2019, ~4.76GB compressed):
   ```
   curl -O "https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/01302019.NASDAQ_ITCH50.gz"
   ```
2. Decompress it (the demo reads the plain binary file, not the `.gz`):
   ```
   gunzip 01302019.NASDAQ_ITCH50.gz
   ```
3. Run the demo against it:
   ```
   ./build/engine_demo 01302019.NASDAQ_ITCH50
   ```

This can take a while (millions of messages) — it prints progress to stderr every million messages processed.
