import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtGui
import numpy as np

app = pg.mkQApp("test")
win = pg.plot(title="test")

x = np.array([1, 2, 3, 4, 5, 6])
y = np.array([1, 5, 2, 8, 3])

curve = pg.PlotCurveItem(x, y, stepMode=True, pen='y')
win.addItem(curve)

if __name__ == '__main__':
    pg.exec()
