#include <iostream>
#include "market_data.grpc.pb.h"
#include "market_data.pb.h"
#include "IOrderBookListener.h"

class OrderBookManager;

class OrderBookService final : public marketdata::OrderbookService::Service, public IOrderBookListener {

private:

	OrderBookManager* manager_;

	struct clientContext {

		grpc::ServerReaderWriter<marketdata::OrderbookUpdate, marketdata::SubscriptionRequest>* stream;
		std::mutex	stream_mutex;
		std::set<int> subscribed_instruments;

	};

	std::vector<clientContext*> active_clients;
	std::mutex clients_mutex;

public:

	inline explicit OrderBookService(OrderBookManager* manager) : manager_(manager) {}
	
	grpc::Status StreamOrderbookUpdates
	(grpc::ServerContext* context, grpc::ServerReaderWriter<marketdata::OrderbookUpdate, marketdata::SubscriptionRequest>* stream) override;


	void OnOrderBookUpdate(int instrument_id, const marketdata::OrderbookIncrementalUpdate& update) override;
};