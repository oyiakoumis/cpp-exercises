#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>

struct Tick {
    long long timestamp; // milliseconds since epoch
    std::string symbol;
    double price;
    int volume;
};

class MarketDataProcessor {
  private:
    std::unordered_map<std::string, std::deque<Tick>> symbolMap;

    static const long TIME_WINDOW_MS = 60000;
    static const int MIN_PRICES_FOR_STDDEV = 20;

    // Remove old ticks outside the time window
    void cleanOldTicks(const std::string &symbol, long currentTime) {
        auto it = symbolMap.find(symbol);
        if (it == symbolMap.end()) return;
        auto &ticks = it->second;
        while (!ticks.empty() && (currentTime - ticks.back().timestamp) > TIME_WINDOW_MS) {
            ticks.pop_back();
        }
    }

    // Get the most recent timestamp for a symbol
    long getLatestTimestamp(const std::string &symbol) const {
        auto it = symbolMap.find(symbol);
        if (it == symbolMap.end() || it->second.empty()) {
            return 0;
        }
        return it->second.front().timestamp;
    }

  public:
    MarketDataProcessor() = default;

    // Process a tick
    void processTick(const Tick &tick) {
        symbolMap[tick.symbol].push_front(tick);
        cleanOldTicks(tick.symbol, tick.timestamp);
    }

    // Moving average over 1 minute (60000ms)
    double getMovingAverage(const std::string &symbol) const {
        auto it = symbolMap.find(symbol);
        if (it == symbolMap.end() || it->second.empty()) return 0.;

        const auto &ticks = it->second;
        double sum = 0.0;
        for (const auto &tick : ticks) {
            sum += tick.price;
        }

        return sum / static_cast<double>(ticks.size());
    }

    // Detect anomaly (price > mean + 3*standard_deviation)
    bool isAnomaly(const std::string &symbol, double price) const {
        auto it = symbolMap.find(symbol);
        if (it == symbolMap.end() || static_cast<int>(it->second.size()) < MIN_PRICES_FOR_STDDEV) {
            return false;
        }
        const auto &ticks = it->second;
        double variance = 0.;
        double mean = getMovingAverage(symbol);
        for (auto &tick : ticks) {
            variance += std::pow(tick.price - mean, 2);
        }
        variance /= static_cast<int>(it->second.size());
        double std_dev = std::sqrt(variance);

        return price > mean + 3 * std_dev ? true : false;
    }

    // Stats for debugging
    void printStats(const std::string &symbol) const {
        auto it = symbolMap.find(symbol);
        if (it == symbolMap.end()) {
            std::cout << "No data for symbol: " << symbol << std::endl;
            return;
        }

        const auto &ticks = it->second;
        std::cout << "=== Stats for " << symbol << " ===" << std::endl;
        std::cout << "Number of ticks in window: " << ticks.size() << std::endl;

        if (!ticks.empty()) {
            std::cout << "Time range: " << ticks.front().timestamp << " to "
                      << ticks.back().timestamp << " ms" << std::endl;
            std::cout << "Moving average: " << getMovingAverage(symbol) << std::endl;

            if (ticks.size() >= MIN_PRICES_FOR_STDDEV) {
                double mean = getMovingAverage(symbol);
                double variance = 0.0;
                for (const auto &tick : ticks) {
                    double diff = tick.price - mean;
                    variance += diff * diff;
                }
                variance /= ticks.size();
                double stddev = std::sqrt(variance);
                std::cout << "Standard deviation: " << stddev << std::endl;
                std::cout << "Anomaly threshold (mean + 3σ): " << (mean + 3.0 * stddev)
                          << std::endl;
            } else {
                std::cout << "Insufficient data for anomaly detection (need "
                          << MIN_PRICES_FOR_STDDEV << " prices)" << std::endl;
            }

            // Show price range
            double minPrice = ticks[0].price, maxPrice = ticks[0].price;
            for (const auto &tick : ticks) {
                minPrice = std::min(minPrice, tick.price);
                maxPrice = std::max(maxPrice, tick.price);
            }
            std::cout << "Price range: [" << minPrice << ", " << maxPrice << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    // Get number of ticks for a symbol (for testing)
    size_t getTickCount(const std::string &symbol) {
        auto it = symbolMap.find(symbol);
        return it == symbolMap.end() ? 0 : it->second.size();
    }
};

// Example usage and test
int main() {
    MarketDataProcessor processor;

    std::cout << "=== Market Data Processor Demo ===" << std::endl;

    // Simulate AAPL tick stream
    long baseTime = 1000000000000L; // Start time

    // Add initial ticks with normal prices around 150
    for (int i = 0; i < 25; i++) {
        double price = 150.0 + (rand() % 5 - 2) * 0.5; // Random price 148-152
        processor.processTick({baseTime + i * 1000, "AAPL", price, 100 + i});
    }

    processor.printStats("AAPL");

    // Test anomaly detection
    std::cout << "=== Anomaly Detection Tests ===" << std::endl;

    double testPrices[] = {151.0, 160.0, 170.0, 200.0};
    for (double testPrice : testPrices) {
        bool isAnom = processor.isAnomaly("AAPL", testPrice);
        std::cout << "Price " << testPrice << " is " << (isAnom ? "ANOMALY" : "normal")
                  << std::endl;
    }

    // Add some GOOGL data
    std::cout << "\n=== Adding GOOGL data ===" << std::endl;
    for (int i = 0; i < 30; i++) {
        double price = 2800.0 + (rand() % 10 - 5) * 2.0; // Random price around 2800
        processor.processTick({baseTime + i * 2000, "GOOGL", price, 50 + i});
    }

    processor.printStats("GOOGL");

    // Test time window by adding old data
    std::cout << "=== Time Window Test ===" << std::endl;
    std::cout << "AAPL ticks before adding old data: " << processor.getTickCount("AAPL")
              << std::endl;

    // Add a very recent tick to trigger cleanup
    processor.processTick({baseTime + 65000, "AAPL", 151.0, 500}); // 65 seconds later

    std::cout << "AAPL ticks after 65-second gap: " << processor.getTickCount("AAPL") << std::endl;
    processor.printStats("AAPL");

    return 0;
}