#include <deque>
#include <iomanip>
#include <iostream>

struct Tick {
    double price;
    int volume;

    Tick(double p, int v) : price(p), volume(v) {}
};

class VWAPCalculator {
  private:
    std::deque<Tick> ticks;
    int windowSize;
    double totalPriceVolume;
    long long totalVolume;

  public:
    VWAPCalculator(int windowSize = 100)
        : windowSize(windowSize), totalPriceVolume(0.0), totalVolume(0) {
        if (windowSize <= 0) {
            throw std::invalid_argument("Window size must be positive");
        }
    }

    // Add a tick
    void addTick(double price, int volume) {
        if (volume <= 0) {
            throw std::invalid_argument("Volume must be positive");
        }

        if (windowSize <= getTickCount()) {
            const Tick &victim = ticks.front();
            totalPriceVolume -= victim.price * victim.volume;
            totalVolume -= victim.volume;
            ticks.pop_front();
        }

        ticks.emplace_back(price, volume);
        totalPriceVolume += price * volume;
        totalVolume += volume;
    }

    // Calculate current VWAP
    double getVWAP() const {
        if (totalVolume == 0) return 0.;
        return totalPriceVolume / totalVolume;
    }

    // Number of ticks in window
    int getTickCount() const {
        return static_cast<int>(ticks.size());
    }

    long long getTotalVolume() const {
        return totalVolume;
    }

    double getTotalPriceVolume() const {
        return totalPriceVolume;
    }

    void clear() {
        ticks.clear();
        totalPriceVolume = 0.;
        totalVolume = 0;
    }
};

int main() {
    std::cout << "=== VWAP Calculator Demo ===" << std::endl;
    std::cout << std::fixed << std::setprecision(4);

    // Test with window size of 3 as per example
    VWAPCalculator vwap(3);

    std::cout << "\nAdding tick: price=100.0, volume=10" << std::endl;
    vwap.addTick(100.0, 10);
    std::cout << "VWAP: " << vwap.getVWAP() << " (Expected: 100.0)" << std::endl;
    std::cout << "Tick count: " << vwap.getTickCount() << std::endl;

    std::cout << "\nAdding tick: price=102.0, volume=20" << std::endl;
    vwap.addTick(102.0, 20);
    std::cout << "VWAP: " << vwap.getVWAP() << " (Expected: 101.3333)" << std::endl;
    std::cout << "Tick count: " << vwap.getTickCount() << std::endl;

    std::cout << "\nAdding tick: price=98.0, volume=30" << std::endl;
    vwap.addTick(98.0, 30);
    std::cout << "VWAP: " << vwap.getVWAP() << " (Expected: 99.6667)" << std::endl;
    std::cout << "Tick count: " << vwap.getTickCount() << std::endl;

    std::cout << "\nAdding tick: price=104.0, volume=40 (should evict first tick)" << std::endl;
    vwap.addTick(104.0, 40);
    std::cout << "VWAP: " << vwap.getVWAP() << " (Expected: 101.5556)" << std::endl;
    std::cout << "Tick count: " << vwap.getTickCount() << std::endl;

    // Test with larger window
    std::cout << "\n=== Testing with default window size (100) ===" << std::endl;
    VWAPCalculator vwap100;

    // Add 150 ticks to test window overflow
    for (int i = 1; i <= 150; ++i) {
        double price = 100.0 + (i % 20) - 10; // Price oscillates between 90-110
        int volume = 10 + (i % 5);            // Volume between 10-14
        vwap100.addTick(price, volume);

        if (i % 25 == 0) {
            std::cout << "After " << i << " ticks - VWAP: " << vwap100.getVWAP()
                      << ", Window size: " << vwap100.getTickCount() << std::endl;
        }
    }

    return 0;
}