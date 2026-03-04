import pyqtgraph as pg
from PySide6.QtWidgets import QWidget, QVBoxLayout
import numpy as np

class DepthChart(QWidget):
    def __init__(self, parent=None):
        
        super().__init__(parent)

        pg.setConfigOptions(antialias=True)
        pg.setConfigOption('background', '#1E1E1E')
        pg.setConfigOption('foreground', '#D4D4D4')

        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(0, 0, 0, 0)
        
        self.plot_widget = pg.PlotWidget(title="Market Depth")
        self.plot_widget.showGrid(x=True, y=True, alpha=0.2)
        
        self.plot_widget.getAxis('left').setPen(pg.mkPen(color='#2D2D2D'))
        self.plot_widget.getAxis('bottom').setPen(pg.mkPen(color='#2D2D2D'))
        
        self.layout.addWidget(self.plot_widget)

        self.bid_pen = pg.mkPen(color='#4CAF50', width=2)
        self.bid_brush = pg.mkBrush(color=(76, 175, 80, 60))
        
        self.ask_pen = pg.mkPen(color='#F44336', width=2)
        self.ask_brush = pg.mkBrush(color=(244, 67, 54, 60))
        
        
        self.bid_plot = None
        self.ask_plot = None

    def update_plot(self, bids: list, asks: list):
       
        self.plot_widget.clear()
        
        if bids:

            sorted_bids = sorted(bids, key=lambda x: x['price'], reverse=True)
            
            bid_prices = []
            bid_cum_vols = [] 
            cum_vol = 0
            
            for b in sorted_bids:
                bid_prices.append(b['price'])
                cum_vol += b['quantity']
                bid_cum_vols.append(cum_vol)

            bid_prices.reverse()
            bid_cum_vols.reverse()

            padding = bid_prices[1] - bid_prices[0] if len(bid_prices) > 1 else 1
            min_price = bid_prices[0] - padding


            step_x = np.array([min_price] + bid_prices)
            step_y = np.array(bid_cum_vols)
            
            self.bid_plot = pg.PlotDataItem(x=step_x, y=step_y, stepMode=True, fillLevel=0, brush=self.bid_brush, pen=self.bid_pen)
            self.plot_widget.addItem(self.bid_plot)
                
        if asks:

            sorted_asks = sorted(asks, key=lambda x: x['price'])
            
            ask_prices = []
            ask_cum_vols = []
            cum_vol = 0
            
            for a in sorted_asks:
                ask_prices.append(a['price'])
                cum_vol += a['quantity']
                ask_cum_vols.append(cum_vol)
                
            
            padding = ask_prices[1] - ask_prices[0] if len(ask_prices) > 1 else 1
            max_price = ask_prices[-1] + padding
                
            step_x = np.array(ask_prices + [max_price])
            step_y = np.array(ask_cum_vols)
            
            self.ask_plot = pg.PlotDataItem(x=step_x, y=step_y, stepMode=True, fillLevel=0, brush=self.ask_brush, pen=self.ask_pen)
            self.plot_widget.addItem(self.ask_plot)
