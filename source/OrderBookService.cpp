#include <mutex>

#include "OrderBookService.h"
#include "OrderBookManager.h"
#include "OrderBook.h"


grpc::Status OrderBookService::StreamOrderbookUpdates
	(grpc::ServerContext* context, grpc::ServerReaderWriter<marketdata::OrderbookUpdate, marketdata::SubscriptionRequest>* stream) {

		clientContext client;
		client.stream = stream;
		{
			std::lock_guard<std::mutex> lock(clients_mutex);
        	active_clients.push_back(&client);
		}	

		marketdata::SubscriptionRequest request;
		
		while(stream->Read(&request)) {

			if(request.has_subscribe()) {
				
				for(int id : request.subscribe().instrument_ids()) {
					{

						std::lock_guard<std::mutex> lock(client.stream_mutex);
						client.subscribed_instruments.insert(id);
					}

					OrderBook* instrumentOrderBook =  manager_->GetOrderBook(id);
					
					if(instrumentOrderBook) {
						
						marketdata::OrderbookUpdate update_message;
						update_message.set_instrument_id(id);
							
						instrumentOrderBook->GetProtoSnapShot(update_message.mutable_snapshot());
						
						{
							std::lock_guard<std::mutex> lock(client.stream_mutex);
							stream->Write(update_message);
						}


					}

					else {
						std::cout << "OrderBookService :: StreamOrderbookUpdates :: invalid orderbook id" << id << '\n';
					}


				}

			}
			else if(request.has_unsubscribe()) {

				std::lock_guard<std::mutex> lock(client.stream_mutex);
				
				for(int id : request.unsubscribe().instrument_ids()) {
					

					client.subscribed_instruments.erase(id);

				}
			}




		}

		{
        	std::lock_guard<std::mutex> lock(clients_mutex);
        
        	auto it = std::find(active_clients.begin(), active_clients.end(), &client);
        	if (it != active_clients.end()) { active_clients.erase(it); }
    	}


	return grpc::Status::OK;

}


void OrderBookService::OnOrderBookUpdate(int instrument_id, const marketdata::OrderbookIncrementalUpdate& update) {


	marketdata::OrderbookUpdate out_msg;
    out_msg.set_instrument_id(instrument_id);

    *out_msg.mutable_incremental() = update;

	std::lock_guard<std::mutex> lock(clients_mutex);

	for(const auto& client : active_clients) {
		
		std::lock_guard<std::mutex> lock(client->stream_mutex);

		if(client->subscribed_instruments.contains(instrument_id)) {

			client->stream->Write(out_msg);
		}

	}

}