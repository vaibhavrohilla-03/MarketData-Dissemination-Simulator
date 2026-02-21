#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>


#include <grpcpp/grpcpp.h>

#include "OrderBook.h"
#include "OrderBookManager.h"
#include "OrderBookService.h"


void RunSimulation(OrderBookManager &manager, std::atomic<bool> &stop_flag) {

  std::random_device rd;
  std::mt19937 rng(rd());

  std::uniform_int_distribution<int> instrument_dist(1, 3);
  std::uniform_int_distribution<int> price_offset_dist(-100, 100);
  std::uniform_int_distribution<int> quantity_dist(1, 500);
  std::uniform_int_distribution<int> side_dist(0, 1);
  std::uniform_int_distribution<int> op_dist(0, 9);
  std::uniform_int_distribution<int> sleep_dist(200, 800);

  std::unordered_map<int, long long> base_prices = {
      {1, 13550},  
      {2, 22010},  
      {3, 9500000} 
  };

  std::cout << "Market simulation started.\n";

  while (!stop_flag.load()) {

    int id = instrument_dist(rng);
    bool is_buy = side_dist(rng) == 1;

    long long base = base_prices[id];
    long long price = base + price_offset_dist(rng);
    int quantity = quantity_dist(rng);

    int op_roll = op_dist(rng);
    Operation op;
    if (op_roll < 5) {
      op = Operation::ADD;
    } else if (op_roll < 8) {
      op = Operation::MODIFY;
    } else {
      op = Operation::REMOVE;
    }

    Level level{price, quantity, is_buy};

    std::cout << "Instrument " << id << " | " << (op == Operation::ADD ? "ADD": op == Operation::MODIFY ? "MOD" : "REM")
              << " | " << (is_buy ? "BID" : "ASK") << " | Price: " << price
              << " | Qty: " << quantity << "\n";

    manager.ProcessUpdate(id, op, level);

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(rng)));
  }

  std::cout << "Market simulation stopped.\n";
}

int main() {

  OrderBookManager manager;

  manager.LoadSnapShot("dummydata_snapshot.json");
  OrderBookService service(&manager);

  manager.RegisterListenerToAll(&service);

  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Market Data Server listening on port 50051\n";

  std::atomic<bool> stop_simulation{false};
  std::thread sim_thread(RunSimulation, std::ref(manager),
                         std::ref(stop_simulation));

  server->Wait();

  // Cleanup on shutdown
  stop_simulation.store(true);
  if (sim_thread.joinable()) {
    sim_thread.join();
  }

  return 0;
}