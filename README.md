# Market Data Dissemination Simulator

## Project Overview

This project is a high-performance market data dissemination simulator designed to process and distribute level 3 orderbook data connected clients through grpc. It simulates the behavior of a trading venue's middleware component.

I was inspired to make this project by this video 
The architecture I followed was heavily referenced from this video

*[Coding Jesus - Building a Market Data Dissemination Simulator](https://youtu.be/GW0Wh1qjbi0?si=sX3NtKG4AGgo3XjD).*
### Data Model: Level 3 Orderbook

The system is built to handle Level 3 orderbook data. A Level 3 orderbook provides the most granular view of the market by tracking individual orders. 

The simulator processes individual order life cycles:
* Adding new orders to the book.
* Modifying existing active orders. (for now I haven't taken into account of changing buy to sell orders it just modifies the existing quantity  )
* Removing or canceling orders.

By disseminating these individual discrete events, connected clients can reconstruct the full depth of the orderbook. (this is also visualized through an depth chart on pyside6 gui)

## Architecture and Protocol Logic

The communication between the server and its clients is governed by gRPC and Protocol Buffers. The logic behind the `.proto` file definitions is structured to handle asynchronous (any of the asynchronous streaming is handled by grpc itself the code itself is written to be synchronously executed), bidirectional streams of financial data efficiently:

* **Separation of Updates**: The data contract defines two primary update messages. `OrderbookSnapshotUpdate` provides the complete current state of the orderbook, allowing clients to build their initial state or reset upon connection issues. `OrderbookIncrementalUpdate` provides the specific ADD, MODIFY, or REMOVE event (Level 3 deltas) to keep the client synchronized without the overhead of sending the full book.
* **Efficient Payload**: Both snapshot and incremental updates are wrapped in a generic `OrderbookUpdate` message utilizing a `oneof` field. This ensures that a single stream can handle any internal message type dynamically while maintaining a small network serialization footprint.
* **Bidirectional Streaming**: The `OrderbookService` implements a bidirectional gRPC stream. This enables the client to push dynamic `SubscriptionRequest` messages (subscribing or unsubscribing from specific instrument IDs on the fly) on the same persistent connection that the server uses to push continuous market data events back to the client.

## Build and Run Instructions

This project consists of a C++ backend server and a Python client frontend.

### Prerequisites
* CMake (v3.15+)
* MSVC (Visual Studio Build Tools) (if preferable)
* Python 3.10+ and [uv](https://github.com/astral-sh/uv)

### Configuration
Instruments are loaded dynamically from a file . You can configure which tickers the server simulates by modifying the JSON file ```dummydata_snapshot.json``` in the root directory before running.

### C++ Server

The server is compiled using MSVC on Windows. Please modify the `CMakeLists.txt` file to suit your specific build process if necessary.
grpc here is linked as a static library for no reasons to benefit it was just a personal preference.

Steps to get the project working :
1. In the thirdparty folder do (if not done yet)
```shell
   git clone --recurse-submodules -b v1.78.1 --depth 1 --shallow-submodules https://github.com/grpc/grpc
```
2. Generate the build files:
   ```shell
   cmake -G \"Ninja\" -B build_msvc -DCMAKE_BUILD_TYPE=Debug
   ```
3. Compile the project:
   ```shell
   cmake --build build_msvc
   ```

### Python Client

The Python client utilizes `uv` to handle its virtual environment and dependencies efficiently.

1. Navigate to the `python_client` directory.

2. run `uv sync` :
   ```shell
   uv sync
   ```
5. Run the client application:
   ```shell
   uv run main.py
   ```
