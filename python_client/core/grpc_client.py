import queue
from typing import List, Dict, Any

from PySide6.QtCore import QThread, Signal
import grpc

from proto import market_data_pb2
from proto import market_data_pb2_grpc

class OrderbookStreamThread(QThread):

    snapshot_received = Signal(int, dict)
    incremental_received = Signal(int, dict)
    connection_error = Signal(str)

    def __init__(self, host: str = "localhost", port: int = 50051, parent=None):
        super().__init__(parent)
        self.host = host
        self.port = port
        
        self.is_running = False
        self.request_queue = queue.Queue()
        self.channel = None

    def subscribe(self, instrument_ids: List[int]) -> None:
        subscribe_msg = market_data_pb2.Subscribe(instrument_ids=instrument_ids)
        request = market_data_pb2.SubscriptionRequest(subscribe=subscribe_msg)
        
        self.request_queue.put(request)


    def unsubscribe(self, instrument_ids: List[int]) -> None:
        unsubscribe_msg = market_data_pb2.UnSubscribe(instrument_ids=instrument_ids)
        request = market_data_pb2.SubscriptionRequest(unsubscribe=unsubscribe_msg)
        
        self.request_queue.put(request)

    def stop(self) -> None:

        self.is_running = False
      
        self.request_queue.put(None)
        
        if self.channel:
            self.channel.close()
        
        self.wait()

    def _request_generator(self):
        while self.is_running:
            try:
                request = self.request_queue.get(timeout=0.1)
                
                if request is None:
                    break
                
                yield request
            except queue.Empty:
                continue

    def run(self) -> None:
        self.is_running = True
        target = f"{self.host}:{self.port}"
        
        try:
            self.channel = grpc.insecure_channel(target)
            stub = market_data_pb2_grpc.OrderbookServiceStub(self.channel)
            
            response_iterator = stub.StreamOrderbookUpdates(self._request_generator())
            
            for update in response_iterator:
                if not self.is_running:
                    break
                
                instrument_id = update.instrument_id
                update_type = update.WhichOneof("update")
                
                if update_type == "snapshot":
                    snapshot_data = self._parse_snapshot(update.snapshot)
                    self.snapshot_received.emit(instrument_id, snapshot_data)
                    
                elif update_type == "incremental":
                    incremental_data = self._parse_incremental(update.incremental)
                    self.incremental_received.emit(instrument_id, incremental_data)

        except grpc.RpcError as e:
            if self.is_running:
                error_msg = f"gRPC connection error: {e.details()} (Code: {e.code()})"
                self.connection_error.emit(error_msg)
        except Exception as e:
            if self.is_running:
                self.connection_error.emit(f"Unexpected streaming err: {str(e)}")
        finally:
            if self.channel:
                self.channel.close()
                self.channel = None

    def _parse_snapshot(self, snapshot_msg) -> Dict[str, Any]:
       
        def parse_levels(levels):
            return [
                {
                    "price": lvl.price, 
                    "quantity": lvl.quantity, 
                    "is_buy": lvl.is_buy
                } 
                for lvl in levels
            ]
            
        return {
            "bids": parse_levels(snapshot_msg.bids),
            "asks": parse_levels(snapshot_msg.asks)
        }

    def _parse_incremental(self, incremental_msg) -> Dict[str, Any]:
        return {
            "type": incremental_msg.type,
            "level": {
                "price": incremental_msg.level.price,
                "quantity": incremental_msg.level.quantity,
                "is_buy": incremental_msg.level.is_buy
            }
        }
