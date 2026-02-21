#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "IOrderBookListener.h"
#include "OrderBook.h"
#include "nlohmann/json.hpp"


using json = nlohmann::json;

void OrderBook::ProcessLevel(Operation operation, const Level &level) {

  std::lock_guard<std::mutex> lock(bookmutex);

  long long levelprice = level.price;
  int levelquantity = level.quantity;

  bool is_updated = false;

  switch (operation) {

  case Operation::ADD: {

    if (level.is_buy) {

      auto result = bids.try_emplace(levelprice, levelquantity);

      if (!result.second) {

        std::cout << "OrderBook :: ADD :: level already exists" << '\n';
      } else {

        is_updated = true;
      }

    }

    else {

      auto result = asks.try_emplace(levelprice, levelquantity);

      if (!result.second) {

        std::cout << "OrderBook :: ADD :: level already exists" << '\n';
      } else {

        is_updated = true;
      }
    }
    break;
  }

  case Operation::MODIFY: {

    if (level.is_buy) {

      auto it = bids.find(levelprice);

      if (it != bids.end()) {

        is_updated = true;

        if (levelquantity <= 0) {

          bids.erase(it);
        } else {

          it->second = levelquantity;
        }
      } else {

        std::cout << "OrderBook :: MODIFY :: Bid level not found" << '\n';
      }

    } else {

      auto it = asks.find(levelprice);

      if (it != asks.end()) {

        is_updated = true;

        if (levelquantity <= 0) {

          asks.erase(it);
        } else {

          it->second = levelquantity;
        }
      }

      else {

        std::cout << "OrderBook :: ModIFY :: ask level not found" << '\n';
      }
    }

    break;
  }

  case Operation::REMOVE: {

    if (level.is_buy) {

      if (bids.find(levelprice) != bids.end()) {

        bids.erase(levelprice);

        is_updated = true;
      }

      else {
        std::cout << "OrderBook :: REMOVE :: nothing to remove" << '\n';
      }
    }

    else {

      if (asks.find(levelprice) != asks.end()) {

        asks.erase(levelprice);

        is_updated = true;
      }

      else {
        std::cout << "OrderBook :: REMOVE :: nothing to remove" << '\n';
      }
    }

    break;
  }
  }

  if (is_updated) {
    marketdata::OrderbookIncrementalUpdate update_msg;

    GetProtoIncrementalUpdate(&update_msg, operation, level);

    NotifyUpdate(update_msg);
  }
}

OrderBook::OrderBook(const std::string &json_file) {

  std::lock_guard<std::mutex> lock(bookmutex);

  std::ifstream json_file_stream(json_file);

  if (!json_file_stream.is_open()) {
    std::cerr << "OrderBook :: Error opening snapshot file " << json_file
              << std::endl;
    return;
  }

  try {

    json Data = json::parse(json_file_stream);

    instrument_id = Data["instrument_id"].get<int>();
    instrument_name = Data["symbol"].get<std::string>();

    for (const auto &item : Data["bids"]) {

      bids[item["price"].get<long long>()] = item["quantity"].get<int>();
    }

    for (const auto &item : Data["asks"]) {

      asks[item["price"].get<long long>()] = item["quantity"].get<int>();
    }

  }

  catch (const json::parse_error &e) {

    std::cout << e.what();
  }
}

OrderBook::OrderBook(int instrument_id_, const std::string &instrument_name_)
    : instrument_id(instrument_id_), instrument_name(instrument_name_) {}

OrderBook::OrderBook(
    int instrument_id_, const std::string &instrument_name_,
    const std::map<long long, int, std::greater<long long>> &bids_in,
    const std::map<long long, int> &asks_in)
    : instrument_id(instrument_id_), instrument_name(instrument_name_) {

  if (instrument_id_ == 0 || instrument_name_.empty()) {

    throw std::invalid_argument("OrderBook : empty string or invalid id");
  }

  std::lock_guard<std::mutex> lock(bookmutex);

  bids = bids_in;
  asks = asks_in;
}

OrderBook::OrderBook(int instrument_id_, const std::string &instrument_name_,
                     const std::vector<Level> &levels)
    : instrument_id(instrument_id_), instrument_name(instrument_name_) {

  if (instrument_id_ == 0 || instrument_name_.empty()) {

    throw std::invalid_argument("OrderBook : empty string or invalid id");
  }

  std::lock_guard<std::mutex> lock(bookmutex);

  for (const auto &lvl : levels) {
    if (lvl.is_buy) {
      bids[lvl.price] = lvl.quantity;
    } else {
      asks[lvl.price] = lvl.quantity;
    }
  }
}

void OrderBook::GetProtoSnapShot(
    marketdata::OrderbookSnapshotUpdate *snapshot) {

  std::lock_guard<std::mutex> lock(bookmutex);

  for (const auto &[price, qty] : bids) {

    auto *lvl = snapshot->add_bids();

    lvl->set_price(static_cast<int64_t>(price));

    lvl->set_quantity(qty);

    lvl->set_is_buy(true);
  }

  for (const auto &[price, qty] : asks) {

    auto *lvl = snapshot->add_asks();

    lvl->set_price(static_cast<int64_t>(price));

    lvl->set_quantity(qty);

    lvl->set_is_buy(false);
  }
}

void OrderBook::GetProtoIncrementalUpdate(
    marketdata::OrderbookIncrementalUpdate *Update, Operation op,
    const Level &level) {

  switch (op) {

  case Operation::ADD: {

    Update->set_type(marketdata::UpdateType::ADD);

    break;
  }

  case Operation::MODIFY: {

    Update->set_type(marketdata::UpdateType::MODIFY);

    break;
  }

  case Operation::REMOVE: {

    Update->set_type(marketdata::UpdateType::REMOVE);

    break;
  }
  }

  auto *proto_lvl = Update->mutable_level();

  proto_lvl->set_price(static_cast<int64_t>(level.price));

  proto_lvl->set_quantity(level.quantity);

  proto_lvl->set_is_buy(level.is_buy);
}

void OrderBook::RegisterListener(IOrderBookListener *listener) {

  std::lock_guard<std::mutex> lock(bookmutex);

  listeners.push_back(listener);
}

void OrderBook::UnRegisterListener(IOrderBookListener *listener) {

  std::lock_guard<std::mutex> lock(bookmutex);

  std::erase(listeners, listener);
}

void OrderBook::NotifyUpdate(
    const marketdata::OrderbookIncrementalUpdate &update) {

  for (const auto &listener : listeners) {

    listener->OnOrderBookUpdate(instrument_id, update);
  }
}
