# Engine

The C++ trading system: a limit order book driven by a real market-data feed, quoted against by a simple strategy, with every component benchmarked. This is the half of the capstone that manufactures evidence of compiled-language systems work.

## Language

### Order book

**Order Book**:
The data structure holding resting Limit Orders on both sides of the market, matched by Price-Time Priority.
_Avoid_: Book (fine as shorthand once unambiguous in context), ledger

**Limit Order**:
An instruction to buy or sell a quantity at a specified price or better, resting in the Order Book until it matches or is cancelled.
_Avoid_: Order (too generic — this project deals only in limit orders, no market orders)

**Price-Time Priority**:
The matching rule: better-priced orders match first; among orders at the same price, the earliest-resting order matches first.
_Avoid_: FIFO matching, price-priority (incomplete — time is the tiebreaker, not a separate rule)

**Add / Cancel / Modify / Match**:
The four behaviours the Order Book exhibits — not four public methods. Add, Cancel, and Modify are caller-initiated; Match is not caller-initiated at all, it's the automatic side effect of an Add that crosses the book, reported via the Fills it produces.
_Modify_: decreasing an order's quantity alone keeps its original time-priority position; increasing quantity, or changing price, loses priority and re-queues the order at the back — see [ADR-0004](./docs/adr/0004-modify-priority-preserved-on-decrease-only.md).

**Order ID**:
An opaque handle the Order Book assigns when an order is added. The caller must hold onto it to Cancel or Modify that order later.
_Avoid_: Order number

**Ticks**:
The Order Book's price representation: an integer count of the instrument's minimum price increment. Never a floating-point price — see [ADR-0005](./docs/adr/0005-integer-tick-prices.md). Adopts ITCH's own native $0.0001 precision directly (e.g. $10.05 is the integer 100500) rather than converting to the standard $0.01 equity increment — see [ADR-0008](./docs/adr/0008-ticks-adopt-itch-native-precision.md).
_Avoid_: Price (say Ticks explicitly when precision matters)

**Fill**:
One partial or complete match between an incoming order and a single resting order, at the resting order's price. A single Add can produce multiple Fills — one per resting order it consumes on its way through the book.
_Avoid_: Trade (Fill is the Order Book's term for this; a "trade" implies settlement/reporting concerns this project doesn't model)

**Best Bid / Best Ask**:
The single best price on each side of the Order Book right now.

**Top of Book**:
Best Bid and Best Ask together — the tightest view of the market.

**Depth**:
Everything behind the Top of Book: the other price levels, showing size waiting beyond the best price.

**Spread**:
Best Ask minus Best Bid. What the Naive Market Maker's own quotes attempt to capture as profit (see Strategy, below).

### Market data

**ITCH**:
NASDAQ's binary market-data protocol (TotalView-ITCH 5.0). The Engine's Feed Parser consumes real ITCH sample data, not a synthetic substitute — see [ADR-0001](./docs/adr/0001-real-itch-data-over-synthetic-feed.md).
_Avoid_: "the feed format" (name it directly)

**Message**:
One ITCH protocol event (e.g. Add Order, Order Executed, Order Cancel) read off the feed. The unit the Feed Parser processes and the unit throughput is measured in.
_Avoid_: Tick (ambiguous — could mean a price increment instead of a feed event), event (too generic)

**Feed**:
The ordered stream of Messages that drives the Order Book. Distinct from the Order Book itself: the Feed is input, the Book is state.
_Avoid_: Market data (too generic on its own — use Feed when referring to the stream this system consumes)

**Feed Parser**:
The Engine component that turns raw ITCH bytes into calls against the Order Book, in three layers: framing (splitting the file into individual Messages' raw bytes), decoding (turning one Message's bytes into a strongly-typed representation), and book-driving (translating decoded Messages into Add/Cancel/Modify calls, filtered to one Stock Locate) — see [ADR-0006](./docs/adr/0006-feed-parser-three-layer-architecture.md). Replays real order flow through the Order Book's own crossing logic rather than trusting ITCH's reported executions as authoritative — see [ADR-0007](./docs/adr/0007-replay-drives-own-crossing-logic.md).

**Stock Locate**:
ITCH's per-day integer code identifying a symbol, established by a Stock Directory Message at the start of the day and referenced by every subsequent Message for that symbol. Reassigned fresh every trading day — never assume the same Stock Locate means the same symbol on a different day.
_Avoid_: Symbol code, ticker ID (say Stock Locate when referring specifically to ITCH's own per-day integer)

**ITCH Reference Number**:
The exchange-assigned integer ITCH uses to identify a specific order within its own protocol — not the Order Book's Order ID. A Replace Message re-keys an order to a *new* ITCH Reference Number even though the Order Book's Modify keeps the same Order ID throughout, so the Feed Parser maintains its own mapping between the two.
_Avoid_: Order ID (reserved for the Order Book's own identifier; these are never the same number)

### Strategy

**Naive Market Maker**:
The Engine's strategy: continuously quotes both sides of the market around a reference price and manages Inventory, deliberately simple rather than alpha-seeking — see [ADR-0002](./docs/adr/0002-naive-market-maker-strategy.md).
_Avoid_: Bot, algo (too generic)

**Inventory**:
The strategy's net position (contracts held, long or short) accumulated from its own Fills. The risk a market maker manages, not a book-wide concept.
_Avoid_: Position (fine as a close synonym, but prefer Inventory for consistency)

### Measurement

**Latency**:
Time from a Message arriving to the Engine producing a result from it (a Book update or a Strategy decision). Always reported as a distribution — p50/p99/p99.9 — never a single average, because tail behaviour is the point.
_Avoid_: Speed, performance (name the specific metric)

**Throughput**:
Messages processed per second, sustained. The Engine's capacity metric, distinct from Latency (capacity vs. response time).

**PnL**:
Realized and unrealized profit and loss from the strategy's fills, net of transaction costs and marked against current Inventory.
_Avoid_: Profit, returns (PnL is the specific, complete term — always include costs and inventory marking)

**Optimisation Arc**:
The Engine's optimisation method: profile → hypothesise → change one thing → measure → keep or revert. Every step in this loop is backed by a Benchmark; no change is kept on the strength of intuition alone.

**Benchmark**:
A measured before/after comparison tied to one specific code change, produced via Google Benchmark. Distinct from a correctness test: a Benchmark says "how fast," a test says "how correct."
_Avoid_: Test (reserve for correctness checks only)
