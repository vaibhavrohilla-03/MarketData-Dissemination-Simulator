#pragma once

#include <mutex>

#include "market_data.pb.h"

class IOrderBookListener;

enum class Operation {

	ADD,
	REMOVE,
	MODIFY
};

struct Level {

	long long price;
	
	int quantity;

	bool is_buy;
};

class OrderBook
{
private:
	
	int instrument_id;

	std::string instrument_name;

	std::mutex bookmutex;

	std::map<long long , int, std::greater<long long>> bids;

	std::map<long long, int> asks;

	std::vector<IOrderBookListener*> listeners;

	void GetProtoIncrementalUpdate(marketdata::OrderbookIncrementalUpdate* Update, Operation op, const Level& level);

public:

	OrderBook(const std::string& json_file);

	OrderBook(int instrument_id_, const std::string& instrument_name_);

	OrderBook(int instrument_id_, const std::string& instrument_name_, const std::map<long long, int,
		std::greater<long long>>& bids_in, const std::map<long long, int>& asks_in);

	OrderBook(int instrument_id_, const std::string& instrument_name_, const std::vector<Level>& levels);

	void ProcessLevel(Operation operation, const Level& level);
	

	void GetProtoSnapShot(marketdata::OrderbookSnapshotUpdate* snapshot);


	void RegisterListener(IOrderBookListener* listener);

	void UnRegisterListener(IOrderBookListener* listener);

	void NotifyUpdate(const marketdata::OrderbookIncrementalUpdate& update);

		

	~OrderBook() = default;
};