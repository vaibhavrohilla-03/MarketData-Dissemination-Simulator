# Market Data Dissemination Client (PySide6)

This directory contains the Python-based GUI client for the C++ Market Data Dissemination simulator.

## Tech Stack
- **Language**: Python 3
- **Package Manager**: `uv` (Extremely fast Python package installer and resolver)
- **GUI Framework**: PySide6 (Qt for Python)
- **Charting**: pyqtgraph
- **Networking**: grpcio, grpcio-tools

## Folder Structure
```text
python_client/
├── README.md             # This plan and documentation file
├── pyproject.toml        # uv dependencies and project metadata
├── genproto/             # Auto-generated gRPC stubs from ../proto/market_data.proto
│   ├── market_data_pb2.py
│   └── market_data_pb2_grpc.py
├── core/
│   └── grpc_client.py    # QThread worker for non-blocking gRPC bidirectional streaming
├── ui/
│   ├── main_window.py    # Main PySide6 dashboard layout
│   └── components/
│       ├── orderbook_table.py  # QTableWidget for Bid/Ask display
│       └── depth_chart.py      # qtpygraph for Cumulative Liquidity
└── main.py               # Application entry point
```

## Running the Application
Since we are using `uv` for dependency management and execution, running the client is as simple as:

```bash
uv run main.py
```
*(This automatically handles the virtual environment and dependencies).*

## Development Plan

1. **Protocol Generation**: Use `grpcio-tools` to compile `market_data.proto` into Python classes inside `genproto/`.
2. **Core Background Worker**: Implement `OrderbookStreamThread` in `core/grpc_client.py`. This `QThread` will safely receive `Snapshot` and `Incremental` updates and emit them as `pyqtSignal` events to the main thread.
3. **UI Layout**: Build the `MainWindow` using `QVBoxLayout` and `QHBoxLayout`.
    - **Control Panel**: Instrument ID `QLineEdit`, Subscribe/Unsubscribe `QPushButton`s.
    - **DOM**: Ask `QTableWidget` (Red), Spread Label, Bid `QTableWidget` (Green).
    - **Chart**: `pyqtgraph` Step-plot for Market Depth.
    - **Log**: `QTextEdit` showing real-time event logs.
4. **Data Binding**: Connect the `QThread` signals to update the internal `self.books` state dictionary and trigger UI redraws.
