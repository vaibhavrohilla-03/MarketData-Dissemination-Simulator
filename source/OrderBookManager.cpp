#include "OrderBookManager.h"
#include "OrderBook.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <mutex>

using json = nlohmann::json;

void OrderBookManager::CreateInstrument(int id, std::string &instrument_name) {

  if (id == 0 || instrument_name.empty()) {

    throw std::invalid_argument("OrderBook : empty string or invalid id");
  }

  std::unique_lock<std::shared_mutex> lock(m);

  OrderBooks.try_emplace(id, std::make_unique<OrderBook>(id, instrument_name));
}

OrderBook *OrderBookManager::GetOrderBook(int id) {

  std::shared_lock<std::shared_mutex> lock(m);

  if (OrderBooks.find(id) != OrderBooks.end()) {

    return OrderBooks[id].get();
  }

  return nullptr;
}

void OrderBookManager::LoadSnapShot(const std::string &json_file) {

  std::unique_lock<std::shared_mutex> lock(m);

  std::ifstream json_file_stream(json_file);

  if (!json_file_stream.is_open()) {
    std::cerr << "OrderBookManager :: Error opening snapshot file " << json_file
              << std::endl;
    return;
  }

  try {

    json Data = json::parse(json_file_stream);

    for (const auto &instruments : Data["Instruments"]) {

      int id = instruments["Id"];
      std::string ticker = instruments["Symbol"];

      auto it = OrderBooks.find(id);

      if (it == OrderBooks.end()) {

        auto result =
            OrderBooks.try_emplace(id, std::make_unique<OrderBook>(id, ticker));
        it = result.first;
      }

      OrderBook *book = it->second.get();

      for (auto &lvl_json : instruments["levels"]) {

        Level lvl{lvl_json["price"].get<long long>(),
                  lvl_json["quantity"].get<int>(),
                  lvl_json["is_buy"].get<bool>()};

        book->ProcessLevel(Operation::ADD, lvl);
      }
    }

  }

  catch (const json::parse_error &e) {

    std::cout << e.what();
  }
}

void OrderBookManager::ProcessUpdate(int instrument_id, Operation op,
                                     const Level &level) {

  std::shared_lock<std::shared_mutex> lock(m);

  auto it = OrderBooks.find(instrument_id);

  if (it != OrderBooks.end()) {

    it->second->ProcessLevel(op, level);
  } else {

    std::cout << "OrderBookManager :: Error :: Instrument ID " << instrument_id
              << " not found." << std::endl;
  }
}

void OrderBookManager::RegisterListenerToAll(IOrderBookListener *listener) {

  std::lock_guard<std::shared_mutex> lock(m);

  for (const auto &pair : OrderBooks) {

    pair.second.get()->RegisterListener(listener);
  }
}

OrderBookManager::OrderBookManager() = default;
OrderBookManager::~OrderBookManager() = default;
