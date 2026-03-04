from PySide6.QtWidgets import QPushButton, QWidget, QVBoxLayout


class BaseWidget (QWidget):

	def __init__(self, title = "Default"):
		super().__init__()
		self.setWindowTitle(title)
		