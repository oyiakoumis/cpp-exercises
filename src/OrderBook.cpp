#include <algorithm>
#include <cfloat>
#include <deque>
#include <iostream>
#include <map>
#include <unordered_map>

class OrderBook {
  public:
    enum Side { BUY, SELL };

  private:
    struct Order {
        int orderID;
        double price;
        int quantity;
        long long timestamp;

        Order(int id, double p, int q, long long ts)
            : orderID(id), price(p), quantity(q), timestamp(ts) {}
    };

    std::map<double, std::deque<Order>, std::greater<double>> bids;
    std::map<double, std::deque<Order>> asks;

    std::unordered_map<int, std::pair<Side, double>> orderLocation;

    double bestBid;
    double bestAsk;

    long long currentTimestamp;

    void updateBestPrices() {
        bestBid = bids.empty() ? 0.0 : bids.begin()->first;
        bestAsk = asks.empty() ? DBL_MAX : asks.begin()->first;
    }

    void matchOrders(Side side, double price, int &quantity, int orderID) {
        if (side == BUY) {
            while (quantity > 0 && !asks.empty() && asks.begin()->first <= price) {
                auto &askQueue = asks.begin()->second;
                while (quantity > 0 && !askQueue.empty()) {
                    Order &askOrder = askQueue.front();
                    int tradeQty = std::min(quantity, askOrder.quantity);
                    quantity -= tradeQty;
                    askOrder.quantity -= tradeQty;

                    std::cout << "Trade executed: BUY ID=" << orderID
                              << " with SELL ID=" << askOrder.orderID
                              << " at Price=" << askOrder.price << " Qty=" << tradeQty << std::endl;

                    if (askOrder.quantity == 0) {
                        orderLocation.erase(askOrder.orderID);
                        askQueue.pop_front();
                    }
                }
                if (askQueue.empty()) {
                    asks.erase(asks.begin());
                }
            }
        } else {
            while (quantity > 0 && !bids.empty() && bids.begin()->first >= price) {
                auto &bidQueue = bids.begin()->second;
                while (quantity > 0 && !bidQueue.empty()) {
                    Order &bidOrder = bidQueue.front();
                    int tradeQty = std::min(quantity, bidOrder.quantity);
                    quantity -= tradeQty;
                    bidOrder.quantity -= tradeQty;

                    std::cout << "Trade executed: SELL ID=" << orderID
                              << " with BUY ID=" << bidOrder.orderID
                              << " at Price=" << bidOrder.price << " Qty=" << tradeQty << std::endl;

                    if (bidOrder.quantity == 0) {
                        orderLocation.erase(bidOrder.orderID);
                        bidQueue.pop_front();
                    }
                }
                if (bidQueue.empty()) {
                    bids.erase(bids.begin());
                }
            }
        }
    }

  public:
    OrderBook() : bestBid(0.0), bestAsk(DBL_MAX), currentTimestamp(0) {}

    // Add an order
    void addOrder(Side side, double price, int quantity, int orderID) {
        if (quantity <= 0) {
            std::cerr << "Quantity must be positive." << std::endl;
            return;
        }

        currentTimestamp++;

        matchOrders(side, price, quantity, orderID);

        if (side == BUY) {
            if (quantity > 0) {
                bids[price].emplace_back(orderID, price, quantity, currentTimestamp);
                orderLocation[orderID] = {BUY, price};
                std::cout << "Added BUY order: ID=" << orderID << ", Price=" << price
                          << ", Qty=" << quantity << std::endl;
            } else {
                std::cout << "BUY order ID=" << orderID << " fully executed upon entry."
                          << std::endl;
            }
        } else {
            if (quantity > 0) {
                asks[price].emplace_back(orderID, price, quantity, currentTimestamp);
                orderLocation[orderID] = {SELL, price};
                std::cout << "Added SELL order: ID=" << orderID << ", Price=" << price
                          << ", Qty=" << quantity << std::endl;
            } else {
                std::cout << "SELL order ID=" << orderID << " fully executed upon entry."
                          << std::endl;
            }
        }
        updateBestPrices();
    }

    // Cancel an order
    bool cancelOrder(int orderID) {
        auto it = orderLocation.find(orderID);
        if (it == orderLocation.end()) {
            std::cerr << "Order ID=" << orderID << " not found for cancellation." << std::endl;
            return false;
        }
        Side side = it->second.first;
        double price = it->second.second;
        auto &orderQueue = (side == BUY) ? bids[price] : asks[price];
        orderQueue.erase(std::remove_if(orderQueue.begin(), orderQueue.end(),
                                        [orderID](const Order &o) { return o.orderID == orderID; }),
                         orderQueue.end());
        if (orderQueue.empty()) {
            if (side == BUY) {
                bids.erase(price);
            } else {
                asks.erase(price);
            }
        }

        orderLocation.erase(it);
        std::cout << "Cancelled order ID=" << orderID << std::endl;
        updateBestPrices();
        return true;
    }

    // Get best bid/ask in O(1)
    double getBestBid() const {
        return bestBid;
    }
    double getBestAsk() const {
        return bestAsk;
    }

    // Display order book (for debugging)
    void printOrderBook() const {
        std::cout << "\n=== ORDER BOOK ===" << std::endl;

        // Display asks in descending order (highest ask at top)
        std::cout << "ASKS (SELL orders):" << std::endl;
        for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
            int totalQty = 0;
            for (const auto &order : it->second) {
                totalQty += order.quantity;
            }
            std::cout << "  $" << it->first << " | " << totalQty << " shares" << std::endl;
        }

        std::cout << "---" << std::endl;
        std::cout << "Best Ask: $" << (bestAsk == DBL_MAX ? 0 : bestAsk) << std::endl;
        std::cout << "Best Bid: $" << bestBid << std::endl;
        std::cout << "---" << std::endl;

        // Display bids in descending order (highest bid at top)
        std::cout << "BIDS (BUY orders):" << std::endl;
        for (const auto &level : bids) {
            int totalQty = 0;
            for (const auto &order : level.second) {
                totalQty += order.quantity;
            }
            std::cout << "  $" << level.first << " | " << totalQty << " shares" << std::endl;
        }
        std::cout << "==================\n" << std::endl;
    }
};

// Usage example and test
int main() {
    OrderBook book;

    std::cout << "=== ORDER BOOK MATCHING ENGINE TEST ===" << std::endl;

    // Test case from the problem
    std::cout << "\n1. Adding BUY order: $100.0, qty=10, id=1" << std::endl;
    book.addOrder(OrderBook::BUY, 100.0, 10, 1);
    book.printOrderBook();

    std::cout << "2. Adding SELL order: $101.0, qty=5, id=2" << std::endl;
    book.addOrder(OrderBook::SELL, 101.0, 5, 2);
    book.printOrderBook();

    std::cout << "3. Adding SELL order: $99.0, qty=8, id=3 (should match with "
                 "order 1)"
              << std::endl;
    book.addOrder(OrderBook::SELL, 99.0, 8, 3);
    book.printOrderBook();

    // Additional test cases
    std::cout << "4. Adding multiple orders to test price-time priority" << std::endl;
    book.addOrder(OrderBook::BUY, 98.0, 5, 4);
    book.addOrder(OrderBook::BUY, 98.0, 3,
                  5); // Same price, should be second in queue
    book.addOrder(OrderBook::SELL, 102.0, 7, 6);
    book.printOrderBook();

    std::cout << "5. Testing order cancellation" << std::endl;
    book.cancelOrder(5); // Cancel order 5
    book.printOrderBook();

    std::cout << "6. Adding SELL order that matches multiple bids" << std::endl;
    book.addOrder(OrderBook::SELL, 97.0, 10,
                  7); // Should match with multiple buy orders
    book.printOrderBook();

    std::cout << "Best Bid: $" << book.getBestBid() << std::endl;
    std::cout << "Best Ask: $" << book.getBestAsk() << std::endl;

    return 0;
}