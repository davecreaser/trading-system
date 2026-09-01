# ITCH 5.0 message formats

Byte-level field layouts for the message types this project's Feed Parser decodes into strongly-typed structs (see [ADR-0006](./adr/0006-feed-parser-three-layer-architecture.md)). Transcribed from the official specification:

> NASDAQ TotalView-ITCH 5.0 Interface Specification, Version 5.0, 03/06/2015
> https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification_5.0.pdf

All other ITCH 5.0 message types (System Event, Stock Trading Action, Reg SHO, Market Participant Position, MWCB, IPO Quoting Period Update, Trade Message, Cross Trade, Broken Trade, NOII, RPII) are handled by the decoding layer's thin catch-all, not decoded field-by-field, since this project never acts on them.

## Data types

- All integer fields are big-endian (network byte order), unsigned unless noted.
- All alpha fields are ASCII, left-justified, padded on the right with spaces.
- Price fields are integers with an implied number of decimal places (e.g. `Price (4)` has 4 implied decimal places). This project decodes prices directly into `Ticks` at ITCH's native precision — see [ADR-0008](./adr/0008-ticks-adopt-itch-native-precision.md).
- Timestamps are nanoseconds since midnight.

## Stock Directory ("R")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "R" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Stock | 11 | 8 | Alpha |
| Market Category | 19 | 1 | Alpha |
| Financial Status Indicator | 20 | 1 | Alpha |
| Round Lot Size | 21 | 4 | Integer |
| Round Lots Only | 25 | 1 | Alpha |
| Issue Classification | 26 | 1 | Alpha |
| Issue Sub-Type | 27 | 2 | Alpha |
| Authenticity | 29 | 1 | Alpha |
| Short Sale Threshold Indicator | 30 | 1 | Alpha |
| IPO Flag | 31 | 1 | Alpha |
| LULD Reference Price Tier | 32 | 1 | Alpha |
| ETP Flag | 33 | 1 | Alpha |
| ETP Leverage Factor | 34 | 4 | Integer |
| Inverse Indicator | 38 | 1 | Alpha |

Total length: 39 bytes. This project's `StockDirectory` struct only captures Stock Locate and Stock — the rest are never consumed anywhere downstream.

## Add Order – No MPID Attribution ("A")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "A" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Order Reference Number | 11 | 8 | Integer |
| Buy/Sell Indicator | 19 | 1 | Alpha ("B"/"S") |
| Shares | 20 | 4 | Integer |
| Stock | 24 | 8 | Alpha |
| Price | 32 | 4 | Price (4) |

Total length: 36 bytes.

## Add Order – With MPID Attribution ("F")

Same as above, plus:

| Name | Offset | Length | Value |
|---|---|---|---|
| Attribution | 36 | 4 | Alpha |

Total length: 40 bytes.

## Order Executed ("E")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "E" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Order Reference Number | 11 | 8 | Integer |
| Executed Shares | 19 | 4 | Integer |
| Match Number | 23 | 8 | Integer |

Total length: 31 bytes.

## Order Executed With Price ("C")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "C" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Order Reference Number | 11 | 8 | Integer |
| Executed Shares | 19 | 4 | Integer |
| Match Number | 23 | 8 | Integer |
| Printable | 31 | 1 | Alpha ("Y"/"N") |
| Execution Price | 32 | 4 | Price (4) |

Total length: 36 bytes.

## Order Cancel ("X")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "X" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Order Reference Number | 11 | 8 | Integer |
| Canceled Shares | 19 | 4 | Integer |

Total length: 23 bytes.

## Order Delete ("D")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "D" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Order Reference Number | 11 | 8 | Integer |

Total length: 19 bytes.

## Order Replace ("U")

| Name | Offset | Length | Value |
|---|---|---|---|
| Message Type | 0 | 1 | "U" |
| Stock Locate | 1 | 2 | Integer |
| Tracking Number | 3 | 2 | Integer |
| Timestamp | 5 | 6 | Integer |
| Original Order Reference Number | 11 | 8 | Integer |
| New Order Reference Number | 19 | 8 | Integer |
| Shares | 27 | 4 | Integer |
| Price | 31 | 4 | Price (4) |

Total length: 35 bytes. Side, stock symbol, and attribution cannot change on a Replace, so they aren't included — they're carried over from the original Add Order (see [ADR-0007](./adr/0007-replay-drives-own-crossing-logic.md) for how this project tracks that).
