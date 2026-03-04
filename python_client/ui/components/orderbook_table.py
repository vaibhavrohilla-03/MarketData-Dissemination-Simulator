from PySide6.QtWidgets import QTableWidget, QTableWidgetItem, QAbstractItemView, QHeaderView
from PySide6.QtCore import Qt
from PySide6.QtGui import QColor, QBrush, QFont

class OrderbookTable(QTableWidget):
    def __init__(self, parent=None, is_bid=True):
        super().__init__(0, 2, parent)
        self.is_bid = is_bid
        
        self.setHorizontalHeaderLabels(["Price", "Quantity"])
        self._setup_styling()
        
    def _setup_styling(self):

        self.setStyleSheet("""
            QTableWidget {
                background-color: #1E1E1E;
                color: #D4D4D4;
                gridline-color: #2D2D2D;
                border: none;
            }
            QHeaderView::section {
                background-color: #252526;
                color: #A0A0A0;
                border: 1px solid #2D2D2D;
                padding: 4px;
                font-weight: bold;
            }
        """)

        
        self.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        self.setSelectionMode(QAbstractItemView.SelectionMode.NoSelection)
        
        self.setAlternatingRowColors(True)
        self.setShowGrid(False)
        self.verticalHeader().setVisible(False)
        
        header = self.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.ResizeMode.Stretch)

        self.bg_color = QColor("#1E1E1E")
        if self.is_bid:
            self.text_color = QColor("#4CAF50") 
        else:
            self.text_color = QColor("#F44336") 
            
        self.font_bold = QFont()
        self.font_bold.setBold(True)


    def update_data(self, levels: list):

        self.setRowCount(0)
        self.setRowCount(len(levels))
        
        for row_idx, level in enumerate(levels):
            price = level.get("price", 0.0)
            quantity = level.get("quantity", 0)

            price_str = f"{price:,.2f}" 
            qty_str = f"{quantity:,}"
            
            items = [
                QTableWidgetItem(price_str),
                QTableWidgetItem(qty_str)
            ]
            
            for col_idx, item in enumerate(items):
                item.setTextAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)
                
                if col_idx == 0:    
                    item.setForeground(QBrush(self.text_color))
                    item.setFont(self.font_bold)
                else:
                    item.setForeground(QBrush(QColor("#D4D4D4")))
                    
                self.setItem(row_idx, col_idx, item)
