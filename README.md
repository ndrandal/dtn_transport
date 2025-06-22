# DTN Transport Ingest Service

`ingest_server` is a lightweight C++ service that connects to DTN’s Level 1 (L1) and Level 2 (L2) data feeds on `localhost`, parses incoming CSV messages into JSON (using RapidJSON), and broadcasts them over a WebSocket server for downstream middleware clients.

---

## Features

- **L1 Feed** (ticks): Connects to port 5009, parses real-time trade/quote messages.  
- **L2 Feed** (depth): Connects to port 9200, subscribes to order book or price-level depth for symbols defined in `config/symbols.csv`.  
- **Schema‐driven**: CSV header files (`config/L1FeedMessages.csv` & `config/MarketDepthMessages.csv`) define JSON keys via `SchemaLoader`.  
- **Type‐aware**: Numeric fields automatically become JSON numbers; Date+Time merge into ISO-8601 `timestamp`.  
- **WebSocket broadcast**: All parsed messages are tagged (`feed`, `messageType`) and broadcast on `ws://<host>:8080`.  
- **Configurable symbols**: Update `config/symbols.csv` (one symbol per line) to change depth subscriptions without code changes.

---

## Prerequisites

- **C++17** compatible compiler (MSVC, GCC, Clang)  
- **CMake** ≥ 3.15  
- **Boost.System** (static, multithreaded)  
- **Standalone Asio** headers  
- **RapidJSON** headers  
- **Threads** (std::thread via CMake `Threads` package)  

Place third‐party libraries under a root directory and pass its path via CMake.

---

## Directory Structure

```
/ (project root)
├── CMakeLists.txt
├── include/
│   ├── ConnectionManager.h
│   ├── SchemaLoader.h
│   ├── MessageDecoder.h
│   └── WebSocketServer.h
├── src/
│   ├── main.cpp
│   ├── ConnectionManager.cpp
│   ├── SchemaLoader.cpp
│   ├── MessageDecoder.cpp
│   └── WebSocketServer.cpp
├── config/
│   ├── L1FeedMessages.csv        # header row for L1 fields
│   ├── MarketDepthMessages.csv   # header row for L2 depth fields
│   └── symbols.csv               # one symbol per line for depth
├── .gitignore
└── README.md
```

---

## Build Instructions

1. **Create build directory**:
   ```bash
   mkdir build && cd build
   ```
2. **Run CMake** (replace the third-party root as needed):
   ```bash
   cmake -DTHIRD_PARTY_DIR="C:/thirdparty" ..
   ```
3. **Compile**:
   ```bash
   cmake --build . --config Release
   ```
4. **Run**:
   ```bash
   ./ingest_server    # or ingest_server.exe on Windows
   ```

By default, the WebSocket server listens on port **8080**.

---

## Configuration

All runtime settings are driven by files in `config/`:

- **`L1FeedMessages.csv`** – CSV header with fields for L1 messages.  
- **`MarketDepthMessages.csv`** – CSV header with fields for L2 depth messages.  
- **`symbols.csv`** – one symbol per line for depth subscriptions (`WOR,<symbol>`).  

Ensure these files are copied into your build output via the CMake post-build command.

---

## Usage Example

Connect a WebSocket client to receive parsed JSON:

```bash
wscat -c ws://localhost:8080
```

Each message will look like:

```json
{
  "feed": "L2",
  "messageType": "3",
  "SYMBOL": "AAPL",
  "Order ID": 123456789,
  "Side": "B",
  "Price": 157.25,
  "Order Size": 100,
  "timestamp": "2025-06-22T15:34:12.123456Z"
}
```

Use **ParameterReference.md** for a complete list of available fields.

---

## Temporary Use Notice

This service is intended as a short-term solution (<1 month). It prioritizes speed of integration over long-term maintainability. For production, consider enhancements like structured logging, metrics, graceful shutdown, and robust error handling.

---

## License & Attribution

Built in-house for internal middleware adoption. Based on DTN/Driver-master examples and IQFeed protocols.
