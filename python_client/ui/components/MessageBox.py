from PySide6.QtWidgets import QMessageBox

class MessageBox(QMessageBox):
	def __init__(self, title = "default"):
		super().__init__()
		self.setWindowTitle(title)



