from PySide6.QtWidgets import QPushButton

class PushButton(QPushButton):

    def __init__(self, text="Default", on_click_callback=None, parent=None):
        super().__init__(text, parent)
        
        if on_click_callback is not None:
            self.clicked.connect(on_click_callback)
            
        self.setMinimumHeight(30)
        self.setStyleSheet("""
            QPushButton {
                background-color: #2D2D2D;
                color: #FFFFFF;
                border: 1px solid #3D3D3D;
                border-radius: 4px;
                padding: 4px 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #3D3D3D;
                border: 1px solid #4D4D4D;
            }
            QPushButton:pressed {
                background-color: #1D1D1D;
            }
        """)
