import logging
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QSplitter, 
    QLabel, QLineEdit, QComboBox, QTextEdit
)
from PySide6.QtCore import Qt, Slot

from .components import MainWindow, PushButton, OrderbookTable, DepthChart

class MarketDataDashboard(MainWindow):
    
    def __init__(self, app, title="Financial Depth of Market Simulator"):
        super().__init__(app, title)

        self.books = {}
        self.active_instrument_id = None
        
        self._setup_ui()
        self._setup_styling()
        
    def _setup_styling(self):
        self.setStyleSheet("""
            QMainWindow {
                background-color: #121212;
            }
            QLabel {
                color: #D4D4D4;
                font-weight: bold;
            }
            QLineEdit, QComboBox, QTextEdit {
                background-color: #1E1E1E;
                color: #D4D4D4;
                border: 1px solid #3D3D3D;
                border-radius: 4px;
                padding: 4px;
            }
            QSplitter::handle {
                background-color: #2D2D2D;
            }
        """)

    def _setup_ui(self):

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout(central_widget)
        
        header_layout = QHBoxLayout()
        
        self.inst_input = QLineEdit()
        self.inst_input.setPlaceholderText("Enter Instrument ID (e.g. 1)")
        self.inst_input.setMaximumWidth(200)
        
        self.btn_subscribe = PushButton("Subscribe")
        self.btn_unsubscribe = PushButton("Unsubscribe")
        
        self.combo_active_inst = QComboBox()
        self.combo_active_inst.setMinimumWidth(150)
        self.combo_active_inst.addItem("Select Instrument...")
        self.combo_active_inst.currentIndexChanged.connect(self._on_active_inst_changed)
        
        header_layout.addWidget(QLabel("Instrument ID:"))
        header_layout.addWidget(self.inst_input)
        header_layout.addWidget(self.btn_subscribe)
        header_layout.addWidget(self.btn_unsubscribe)
        header_layout.addStretch()
        header_layout.addWidget(QLabel("Active View:"))
        header_layout.addWidget(self.combo_active_inst)
        
        main_layout.addLayout(header_layout)
        
        middle_splitter = QSplitter(Qt.Orientation.Horizontal)
        

        ladder_splitter = QSplitter(Qt.Orientation.Vertical)
        
        self.ask_table = OrderbookTable(is_bid=False)
        self.bid_table = OrderbookTable(is_bid=True)
        
        self.spread_label = QLabel("Spread: 0.00")
        self.spread_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.spread_label.setStyleSheet("background-color: #2D2D2D; padding: 4px;")
        
        ladder_splitter.addWidget(self.ask_table)
        ladder_splitter.addWidget(self.spread_label)
        ladder_splitter.addWidget(self.bid_table)
        ladder_splitter.setSizes([300, 30, 300])
        
        self.depth_chart = DepthChart()
        
        middle_splitter.addWidget(ladder_splitter)
        middle_splitter.addWidget(self.depth_chart)
        middle_splitter.setSizes([400, 600])
        
        main_layout.addWidget(middle_splitter, stretch=3)
        
        self.console_log = QTextEdit()
        self.console_log.setReadOnly(True)
        self.console_log.setMaximumHeight(150)
        main_layout.addWidget(self.console_log, stretch=1)

    def log_msg(self, msg: str):
        self.console_log.append(msg)
        
    @Slot(str)
    def log_error(self, err_msg: str):
        self.console_log.append(f"<span style='color:red;'>{err_msg}</span>")

    def _on_active_inst_changed(self, index):
        text = self.combo_active_inst.itemText(index)
        if text.isdigit():
            self.active_instrument_id = int(text)
            self._redraw_ui()
        else:
            self.active_instrument_id = None
            self.ask_table.update_data([])
            self.bid_table.update_data([])
            self.depth_chart.update_plot([], [])
            self.spread_label.setText("Spread: 0.00")

    def _ensure_instrument_entry(self, inst_id: int):
        if inst_id not in self.books:
            self.books[inst_id] = {'bids': [], 'asks': []}
            
            for i in range(self.combo_active_inst.count()):
                if self.combo_active_inst.itemText(i) == str(inst_id):
                    return
            self.combo_active_inst.addItem(str(inst_id))

    @Slot(int, dict)
    def on_snapshot(self, inst_id: int, data: dict):
        self._ensure_instrument_entry(inst_id)
        self.books[inst_id] = data
        self.log_msg(f"Snapshot received for {inst_id}")
        
        if self.active_instrument_id == inst_id:
            self._redraw_ui()

    @Slot(int, dict)
    def on_incremental(self, inst_id: int, data: dict):
        self._ensure_instrument_entry(inst_id)
        
        op_type = data.get("type", 0) 
        level = data.get("level", {})
        
        book = self.books[inst_id]
        side = 'bids' if level.get('is_buy') else 'asks'
        
        price = level.get('price')
        existing = next((lvl for lvl in book[side] if lvl['price'] == price), None)
        
        if op_type == 0: # ADD
            if existing: existing['quantity'] += level['quantity']
            else: book[side].append(level)
        elif op_type == 1: # MODIFY
            if existing: existing['quantity'] = level['quantity']
            else: book[side].append(level)
        elif op_type == 2: # REMOVE
            if existing: 
                book[side].remove(existing)
                
        self.log_msg(f"Incremental received for {inst_id}: {level}")
        
        if self.active_instrument_id == inst_id:
            self._redraw_ui()

    def _redraw_ui(self):
        if not self.active_instrument_id:
            return
            
        book = self.books.get(self.active_instrument_id)
        if not book:
            return
            
        bids = book.get('bids', [])
        asks = book.get('asks', [])
 

        asks_sorted = sorted(asks, key=lambda x: x['price'], reverse=True)
        bids_sorted = sorted(bids, key=lambda x: x['price'], reverse=True)
 

        self.ask_table.update_data(asks_sorted)
        self.bid_table.update_data(bids_sorted)
        
        self.depth_chart.update_plot(bids, asks)
 
        if bids and asks:
            best_bid = max(b["price"] for b in bids)
            best_ask = min(a["price"] for a in asks)
            spread = best_ask - best_bid
            self.spread_label.setText(f"Spread: {spread:.2f}")
        else:
            self.spread_label.setText("Spread: N/A")
