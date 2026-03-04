// #include <atomic>
// #include <iomanip>
// #include <iostream>
// #include <map>
// #include <memory>
// #include <mutex>
// #include <sstream>
// #include <string>
// #include <thread>
// #include <grpcpp/grpcpp.h>

// #include "market_data.grpc.pb.h"
// #include "market_data.pb.h"


// struct LocalOrderBook {
//   std::map<long long, int, std::greater<long long>> bids; 
//   std::map<long long, int> asks;                          
// };

// class MarketDataClient {

// public:
//   MarketDataClient(std::shared_ptr<grpc::ChannelInterface> channel)
//       : stub_(marketdata::OrderbookService::NewStub(channel)) {}

//   void Run() {

//     grpc::ClientContext context;
//     stream_ = stub_->StreamOrderbookUpdates(&context);

//     std::thread input_thread(&MarketDataClient::HandleUserInput, this);

//     marketdata::OrderbookUpdate update_msg;

//     std::cout << "Listening for market data...\n\n";

//     while (stream_->Read(&update_msg)) {

//       int id = update_msg.instrument_id();

//       if (update_msg.has_snapshot()) {

//         HandleSnapshot(id, update_msg.snapshot());

//       } else if (update_msg.has_incremental()) {

//         HandleIncremental(id, update_msg.incremental());
//       }

//       PrintOrderBook(id);
//     }

//     std::cout << "\nStream ended.\n";
//     running_.store(false);

//     if (input_thread.joinable()) {
//       input_thread.join();
//     }

//     grpc::Status status = stream_->Finish();           
//     if (!status.ok()) {
//       std::cerr << "\n RPC finished with error: "
//                 << status.error_message() << "\n";
//     }
//   }

// private:
//   std::unique_ptr<marketdata::OrderbookService::Stub> stub_;
//   std::unique_ptr<grpc::ClientReaderWriter<marketdata::SubscriptionRequest,
//                                            marketdata::OrderbookUpdate>>
//       stream_;

//   std::mutex book_mutex_;
//   std::map<int, LocalOrderBook> books_;

//   std::atomic<bool> running_{true};


//   void HandleSnapshot(int id,
//                       const marketdata::OrderbookSnapshotUpdate &snapshot) {

//     std::lock_guard<std::mutex> lock(book_mutex_);

//     auto &book = books_[id];
//     book.bids.clear();
//     book.asks.clear();

//     for (const auto &lvl : snapshot.bids()) {
//       book.bids[lvl.price()] = lvl.quantity();
//     }
//     for (const auto &lvl : snapshot.asks()) {
//       book.asks[lvl.price()] = lvl.quantity();
//     }

//     std::cout << "\n SNAPSHOT received for Instrument " << id << " \n";
//   }


//   void HandleIncremental(int id,
//                          const marketdata::OrderbookIncrementalUpdate &update) {

//     std::lock_guard<std::mutex> lock(book_mutex_);

//     auto &book = books_[id];
//     const auto &lvl = update.level();
//     long long price = lvl.price();
//     int qty = lvl.quantity();
//     bool is_buy = lvl.is_buy();

//     auto applyUpdate = [&](auto &side_map) {
//       switch (update.type()) {
//       case marketdata::UpdateType::ADD:
//         side_map[price] = qty;
//         break;
//       case marketdata::UpdateType::MODIFY:
//         if (side_map.count(price)) {
//           if (qty <= 0) {
//             side_map.erase(price);
//           } else {
//             side_map[price] = qty;
//           }
//         }
//         break;
//       case marketdata::UpdateType::REMOVE:
//         side_map.erase(price);
//         break;
//       default:
//         break;
//       }
//     };

//     if (is_buy) {
//       applyUpdate(book.bids);
//     } else {
//       applyUpdate(book.asks);
//     }

//     std::cout << "\nINCREMENTAL" << "id : " << id << '-' << (update.type() == marketdata::ADD ? "ADD": update.type() == marketdata::MODIFY ? "MOD": "REM")
//               << "  " << (is_buy ? "BID" : "ASK") << "  Price: " << price
//               << "  Qty: " << qty << "\n";
//   }

//   void PrintOrderBook(int id) {

//     std::lock_guard<std::mutex> lock(book_mutex_);

//     auto it = books_.find(id);
//     if (it == books_.end())
//       return;

//     const auto &book = it->second;

//     std::cout << "\n\n";
//     std::cout << "  Order Book   Instrument " << id << "\n\n";
//     std::cout << "  ASKS (top 5)\n";
//     std::cout << "  " << std::setw(10) << "Price" << "    " << std::setw(10)
//               << "Qty" << "\n\n";

//     std::vector<std::pair<long long, int>> top_asks;
//     int count = 0;
//     for (const auto &[price, qty] : book.asks) {
//       if (count++ >= 5)
//         break;
//       top_asks.push_back({price, qty});
//     }
//     for (auto rit = top_asks.rbegin(); rit != top_asks.rend(); ++rit) {
//       std::cout << "  " << std::setw(10) << rit->first << "    "
//                 << std::setw(10) << rit->second << "\n";
//     }

//     std::cout << "\n SPREAD\n\n";
//     std::cout << "  BIDS (top 5)\n";
//     std::cout << "  " << std::setw(10) << "Price" << "    " << std::setw(10)
//               << "Qty" << "\n\n";

//     count = 0;
//     for (const auto &[price, qty] : book.bids) {
//       if (count++ >= 5)
//         break;
//       std::cout << "  " << std::setw(10) << price << "    " << std::setw(10)
//                 << qty << "\n";
//     }

//     std::cout << "\n";
//   }

  

//   void HandleUserInput() {

//     std::cout << "\n";
//     std::cout << "  Commands:\n";
//     std::cout << "    sub id1 id2 ...   - Subscribe\n";
//     std::cout << "    unsub id1 id2 ... - Unsubscribe\n";
//     std::cout << "    quit                  - Exit\n";

//     std::string line;
//     while (running_.load() && std::getline(std::cin, line)) {

//       if (line.empty())
//         continue;

//       std::istringstream iss(line);
//       std::string command;
//       iss >> command;

//       if (command == "quit") {
//         std::cout << "[Client] Shutting down...\n";
//         stream_->WritesDone();
//         break;
//       } else if (command == "sub") {

//         marketdata::SubscriptionRequest req;
//         auto *sub = req.mutable_subscribe();
//         int id;
//         while (iss >> id) {
//           sub->add_instrument_ids(id);
//           std::cout << " Subscribing to instrument " << id << "\n";
//         }
//         stream_->Write(req);
//       } else if (command == "unsub") {

//         marketdata::SubscriptionRequest req;
//         auto *unsub = req.mutable_unsubscribe();
//         int id;
//         while (iss >> id) {
//           unsub->add_instrument_ids(id);
//           std::cout << " Unsubscribing from instrument " << id << "\n";
//         }
//         stream_->Write(req);
//       } else {
//         std::cout << " Unknown command: " << command << "\n";
//       }
//     }
//   }
// };

// int main() {

//   std::string server_address("localhost:50051");

//   std::shared_ptr<grpc::Channel> channel =
//       grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

//   MarketDataClient client(channel);

//   std::cout << "Connecting to market data server at " << server_address
//             << "...\n";

//   client.Run();

//   return 0;
// }