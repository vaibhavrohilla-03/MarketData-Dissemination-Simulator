from PySide6.QtCore import QSize
from PySide6.QtWidgets import QMainWindow, QToolBar

class MainWindow(QMainWindow):
	def __init__(self, app, title = "Default"):
		super().__init__()
		self.app = app
		self.setWindowTitle(title)
		menu_bar = self.menuBar()
		