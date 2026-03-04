import sys
import logging
from PySide6.QtWidgets import QApplication

from core.grpc_client import OrderbookStreamThread
from ui.main_window import MarketDataDashboard

def main():
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    
    app = QApplication(sys.argv)
    
    dashboard = MarketDataDashboard(app)
    dashboard.resize(1024, 768)
    dashboard.show()
    
    stream_thread = OrderbookStreamThread(host="localhost", port=50051)
    
    stream_thread.snapshot_received.connect(dashboard.on_snapshot)
    stream_thread.incremental_received.connect(dashboard.on_incremental)
    stream_thread.connection_error.connect(dashboard.log_error)
    
    def handle_subscribe():
        text = dashboard.inst_input.text()
        if text.isdigit():
            inst_id = int(text)
            stream_thread.subscribe([inst_id])
            dashboard.log_msg(f"Sent Subscription for {inst_id}")
            dashboard._ensure_instrument_entry(inst_id)
            
    def handle_unsubscribe():
        text = dashboard.inst_input.text()
        if text.isdigit():
            inst_id = int(text)
            stream_thread.unsubscribe([inst_id])
            dashboard.log_msg(f"Sent Unsubscription for {inst_id}")

    dashboard.btn_subscribe.clicked.connect(handle_subscribe)
    dashboard.btn_unsubscribe.clicked.connect(handle_unsubscribe)
    
    dashboard.log_msg("Starting gRPC Stream Thread Subscribe instruments to visualize")
    stream_thread.start()
    
    app.aboutToQuit.connect(stream_thread.stop)
    
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
