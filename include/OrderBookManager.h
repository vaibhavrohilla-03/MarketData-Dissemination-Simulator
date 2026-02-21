#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>


class OrderBook;
class IOrderBookListener;
enum class Operation;
struct Level;

class OrderBookManager {

private:
  std::shared_mutex m;

  std::unordered_map<int, std::unique_ptr<OrderBook>> OrderBooks;

public:
  OrderBookManager();

  void CreateInstrument(int id, std::string &instrument_name);

  OrderBook *GetOrderBook(int id);

  void LoadSnapShot(const std::string &json_file);

  void ProcessUpdate(int instrument_id, Operation op, const Level &level);

  void RegisterListenerToAll(IOrderBookListener *listener);

  ~OrderBookManager();
};