#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

template <typename T> class ThreadSafeQueue {
  private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable condition_;

  public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
        condition_.notify_one();
    }

    bool tryPop(T &item) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return false;
        }
        item = queue_.front();
        queue_.pop();
        return true;
    }

    bool waitAndPop(T &item, const std::chrono::milliseconds &timeout) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (condition_.wait_for(lock, timeout, [this] { return !queue_.empty(); })) {
            item = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
};

struct Tick {
    std::string symbol;
    double price;
    int volume;

    Tick() = default;
    Tick(std::string s, double p, int v) : symbol(s), price(p), volume(v) {}
};

struct VWAPData {
    double vwap = 0.0;
    double totalValue = 0.0;
    int totalVolume = 0;
};

class TickProcessor {
  private:
    ThreadSafeQueue<Tick> tickQueue_;
    std::unordered_map<std::string, VWAPData> vwapData_;
    mutable std::shared_mutex vwapMutex_;

    std::thread processorThread_;
    std::atomic<bool> running_{false};
    std::atomic<int> ticksProcessed_{0};

  public:
    ~TickProcessor() {
        if (running_.load()) {
            stop();
        }
    }

    void start() {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            processorThread_ = std::thread(&TickProcessor::processorLoop, this);
        }
    }

    void stop() {
        if (running_.load()) {
            running_.store(false);
            if (processorThread_.joinable()) {
                processorThread_.join();
            }
        }
    }

    void addTick(const Tick &tick) {
        if (running_.load()) {
            tickQueue_.push(tick);
        }
    }

    double getVWAP(const std::string &symbol) const {
        std::shared_lock<std::shared_mutex> lock(vwapMutex_);
        auto it = vwapData_.find(symbol);
        if (it != vwapData_.end()) {
            return it->second.vwap;
        }
        return 0.0;
    }

    int getProcessedCount() const {
        return ticksProcessed_.load();
    }

  private:
    void processorLoop() {
        while (running_.load()) {
            Tick tick;
            if (tickQueue_.waitAndPop(tick, std::chrono::milliseconds(100))) {
                if (validateTick(tick)) {
                    updateVWAP(tick);
                    ticksProcessed_.fetch_add(1);
                }
            }
        }

        Tick tick;
        while (tickQueue_.tryPop(tick)) {
            if (validateTick(tick)) {
                updateVWAP(tick);
                ticksProcessed_.fetch_add(1);
            }
        }
    }

    bool validateTick(const Tick &tick) const {
        return !tick.symbol.empty() && tick.price > 0.0 && tick.volume > 0;
    }

    void updateVWAP(const Tick &tick) {
        std::unique_lock<std::shared_mutex> lock(vwapMutex_);

        VWAPData &vwap_data = vwapData_[tick.symbol];

        vwap_data.totalValue += tick.price * tick.volume;
        vwap_data.totalVolume += tick.volume;

        if (vwap_data.totalVolume > 0) {
            vwap_data.vwap = vwap_data.totalValue / vwap_data.totalVolume;
        }
    }
};

int main() {
    std::cout << "=== Tests for the Tick Processor ===" << std::endl;

    TickProcessor processor;
    processor.start();

    std::vector<Tick> testTicks = {{"AAPL", 150.0, 100},
                                   {"AAPL", 151.0, 200},
                                   {"GOOGL", 2800.0, 50},
                                   {"AAPL", 149.0, 150},
                                   {"GOOGL", 2810.0, 75}};

    std::cout << testTicks.size() << " Ticks added..." << std::endl;

    for (const auto &tick : testTicks) {
        processor.addTick(tick);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Ticks treated: " << processor.getProcessedCount() << std::endl;
    std::cout << "VWAP AAPL: " << processor.getVWAP("AAPL") << std::endl;
    std::cout << "VWAP GOOGL: " << processor.getVWAP("GOOGL") << std::endl;

    processor.stop();

    return 0;
}