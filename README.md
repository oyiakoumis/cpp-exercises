# ** C++ Exercise Instructions**

## **EXERCISE 1: Order Book with Matching Engine**

### **Objective**
Implement an order book that can receive, cancel, and automatically match orders.

### **Specifications**
```cpp
class OrderBook {
public:
    enum Side { BUY, SELL };
    
    // Add an order
    void addOrder(Side side, double price, int quantity, int orderID);
    
    // Cancel an order
    bool cancelOrder(int orderID);
    
    // Get best bid/ask in O(1)
    double getBestBid() const;
    double getBestAsk() const;
    
    // Display order book (for debugging)
    void printOrderBook() const;
};
```

### **Constraints**
- `getBestBid()/getBestAsk()` must be **O(1)**
- Matching must be automatic when adding orders
- Handle partially filled orders
- Priority: price-time (FIFO at same price)

### **Usage Example**
```cpp
OrderBook book;
book.addOrder(BUY, 100.0, 10, 1);   // Bid 100 qty 10
book.addOrder(SELL, 101.0, 5, 2);   // Ask 101 qty 5
book.addOrder(SELL, 99.0, 8, 3);    // Matches with order 1
// Order 1 becomes qty=2, order 3 fully executed
```

---

## **EXERCISE 2: Generic LRU Cache**

### **Objective**
Implement a generic LRU (Least Recently Used) cache with O(1) access that works with any key-value types.

### **Specifications**
```cpp
template<typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity);
    
    // Get a value, returns std::nullopt if not found
    std::optional<Value> get(const Key& key);
    
    // Insert/update a value
    void put(const Key& key, const Value& value);
    
    // Additional utility methods
    bool contains(const Key& key) const;
    size_t size() const;
    void clear();
    
private:
    size_t capacity_;
    // Your implementation here
};
```

### **Constraints**
- `get()` and `put()` in **O(1)**
- When capacity is exceeded, remove the least recently used element
- Use `std::unordered_map` + doubly linked list
- Support any hashable key type and any value type

### **Usage Example**
```cpp
LRUCache<int, std::string> cache(2);
cache.put(1, "Alice");
cache.put(2, "Bob");
auto result = cache.get(1);     // returns std::optional{"Alice"}
cache.put(3, "Charlie");        // evicts key 2
auto missing = cache.get(2);    // returns std::nullopt
```

---

## **EXERCISE 3: VWAP Calculator**

### **Objective**
Calculate VWAP (Volume Weighted Average Price) over a sliding window of the last 100 ticks.

### **Specifications**
```cpp
class VWAPCalculator {
public:
    VWAPCalculator(int windowSize = 100);
    
    // Add a tick
    void addTick(double price, int volume);
    
    // Calculate current VWAP
    double getVWAP() const;
    
    // Number of ticks in window
    int getTickCount() const;
};
```

### **Constraints**
- Sliding window of maximum 100 ticks
- VWAP = Σ(price × volume) / Σ(volume)
- Handle case where there are fewer than 100 ticks
- Reasonable performance for frequent additions

### **Usage Example**
```cpp
VWAPCalculator vwap(3);  // window of 3 for testing
vwap.addTick(100.0, 10); // VWAP = 100.0
vwap.addTick(102.0, 20); // VWAP = (100*10 + 102*20)/(10+20) = 101.33
vwap.addTick(98.0, 30);  // VWAP = (100*10 + 102*20 + 98*30)/(10+20+30)
vwap.addTick(104.0, 40); // Evicts first tick
```

---

## **EXERCISE 4: Market Data Processor**

### **Objective**
Process a tick stream to calculate moving average and detect anomalies.

### **Specifications**
```cpp
struct Tick {
    long timestamp;  // milliseconds since epoch
    std::string symbol;
    double price;
    int volume;
};

class MarketDataProcessor {
public:
    MarketDataProcessor();
    
    // Process a tick
    void processTick(const Tick& tick);
    
    // Moving average over 1 minute (60000ms)
    double getMovingAverage(const std::string& symbol) const;
    
    // Detect anomaly (price > mean + 3*standard_deviation)
    bool isAnomaly(const std::string& symbol, double price) const;
    
    // Stats for debugging
    void printStats(const std::string& symbol) const;
};
```

### **Constraints**
- Handle multiple symbols simultaneously
- Moving average over 1-minute time window
- Anomaly = price > mean + 3×standard_deviation of recent prices
- Maintain at least 20 prices for standard deviation calculation

### **Usage Example**
```cpp
MarketDataProcessor processor;
processor.processTick({1000, "AAPL", 150.0, 100});
processor.processTick({2000, "AAPL", 151.0, 200});
// ... more ticks
bool anomaly = processor.isAnomaly("AAPL", 200.0);  // true if > 3σ
```

---

## **EXERCISE 5: Feature Extractor**

### **Objective**
Extract useful features for an ML model from a tick stream.

### **Specifications**
```cpp
struct MLFeatures {
    double priceReturn;      // (price - prev_price) / prev_price
    double volatility;       // standard deviation of last 10 returns
    double momentum;         // average of last 5 returns
    double volumeRatio;      // current_volume / average_volume_10ticks
    double spreadIndicator;  // bid-ask spread measure if available
};

class FeatureExtractor {
public:
    FeatureExtractor(int historySize = 20);
    
    // Process a tick and extract features
    MLFeatures extractFeatures(const Tick& tick);
    
    // Reset for new symbol
    void reset();
    
private:
    // Your price/volume history
};
```

### **Constraints**
- Calculate features in real-time (no look-ahead)
- Handle first few ticks when history is insufficient
- Normalized/sensible features for ML
- Performance for high-frequency processing

### **Usage Example**
```cpp
FeatureExtractor extractor;
MLFeatures features = extractor.extractFeatures({1000, "AAPL", 150.0, 100});
// features.priceReturn = NaN (first tick)
// ... more ticks
features = extractor.extractFeatures({2000, "AAPL", 151.0, 200});
// features.priceReturn = 0.0066 (1% increase)
```

---

## **EXERCISE 6: Producer-Consumer Queue**

### **Objective**
Implement a thread-safe queue for communication between producer and consumer.

### **Specifications**
```cpp
template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue(size_t maxSize = 1000);
    
    // Add an element (blocks if full)
    void push(const T& item);
    
    // Non-blocking version (returns false if full)
    bool tryPush(const T& item);
    
    // Get an element (blocks if empty)
    T pop();
    
    // Non-blocking version (returns false if empty)
    bool tryPop(T& item);
    
    // Current size
    size_t size() const;
    
    // Clean shutdown (for thread termination)
    void shutdown();
};
```

### **Constraints**
- **Version 1**: Use `std::mutex` + `std::condition_variable`
- **Version 2** (bonus): Implement lock-free version with `std::atomic`
- Handle clean thread shutdown
- Avoid deadlocks and race conditions

### **Usage Example**
```cpp
ThreadSafeQueue<int> queue(10);

// Producer thread
queue.push(42);

// Consumer thread
int value = queue.pop();  // value = 42
```

---

## **EXERCISE 7: Thread-safe Circular Buffer**

### **Objective**
Circular buffer optimized to avoid lock contention.

### **Specifications**
```cpp
template<typename T, size_t Size>
class CircularBuffer {
public:
    CircularBuffer();
    
    // Write (producer)
    bool write(const T& item);
    
    // Read (consumer)
    bool read(T& item);
    
    // States
    bool empty() const;
    bool full() const;
    size_t size() const;
    
private:
    // Use std::atomic for indices
    // Avoid false sharing with alignment
};
```

### **Constraints**
- **Fixed size** defined at compile time
- **Single producer, single consumer** (SPSC)
- Use only `std::atomic` (no mutex)
- Avoid false sharing (cache alignment)
- Optimal performance for high-frequency data

### **Performance Test**
```cpp
CircularBuffer<int, 1024> buffer;

// Measure throughput
auto start = std::chrono::high_resolution_clock::now();
// ... push/pop in loop
auto end = std::chrono::high_resolution_clock::now();
```

---

## **EXERCISE 8: Multi-threaded Tick Processor**

### **Objective**
Complete system with separate threads for different tasks.

### **Specifications**
```cpp
class MultiThreadedTickProcessor {
public:
    MultiThreadedTickProcessor();
    ~MultiThreadedTickProcessor();
    
    // Start the system
    void start();
    
    // Clean stop
    void stop();
    
    // Inject ticks (main thread)
    void feedTick(const Tick& tick);
    
    // Get results
    double getVWAP(const std::string& symbol) const;
    MLFeatures getLastFeatures(const std::string& symbol) const;
    
private:
    // Thread 1: Market data ingestion
    void marketDataThread();
    
    // Thread 2: VWAP calculations
    void vwapThread();
    
    // Thread 3: Feature extraction
    void featureThread();
    
    // Thread 4: Anomaly detection
    void anomalyThread();
};
```

### **Constraints**
- **4 threads** with separate responsibilities
- Communication via thread-safe queues
- Clean shutdown of all threads
- Error and exception handling
- Performance monitoring (optional)

---

## **EXERCISE 9: Linear Regression**

### **Objective**
Simple linear regression with gradient descent, no external library.

### **Specifications**
```cpp
class LinearRegression {
public:
    LinearRegression(double learningRate = 0.01, int maxIterations = 1000);
    
    // Train the model
    void fit(const std::vector<std::vector<double>>& X, 
             const std::vector<double>& y);
    
    // Predictions
    std::vector<double> predict(const std::vector<std::vector<double>>& X) const;
    
    // Get coefficients
    std::vector<double> getCoefficients() const;
    double getIntercept() const;
    
    // Metrics
    double getMeanSquaredError(const std::vector<std::vector<double>>& X,
                               const std::vector<double>& y) const;
};
```

### **Constraints**
- Gradient descent algorithm from scratch
- Handle multiple features (X can have multiple columns)
- Stopping criteria: convergence or max iterations
- Optional feature normalization

### **Usage Example**
```cpp
// Data: price = a*volume + b*volatility + c
std::vector<std::vector<double>> X = {{100, 0.1}, {200, 0.2}, {150, 0.15}};
std::vector<double> y = {50.0, 55.0, 52.0};

LinearRegression lr;
lr.fit(X, y);
auto predictions = lr.predict(X);
```

---

## **EXERCISE 10: Mini Perceptron**

### **Objective**
Simple perceptron with forward and backward pass.

### **Specifications**
```cpp
class Perceptron {
public:
    Perceptron(int inputSize, double learningRate = 0.1);
    
    // Forward pass
    double forward(const std::vector<double>& input);
    
    // Backward pass and weight update
    void backward(double target);
    
    // Train on a dataset
    void train(const std::vector<std::vector<double>>& X,
               const std::vector<double>& y,
               int epochs = 100);
    
    // Prediction
    double predict(const std::vector<double>& input);
    
private:
    std::vector<double> weights_;
    double bias_;
    double learning_rate_;
    double last_output_;  // for backward pass
    std::vector<double> last_input_;  // for backward pass
};
```

### **Constraints**
- Activation function: sigmoid
- Algorithm: gradient descent
- Weight update after each example
- Handle binary classification (output 0 or 1)

---

## **EXERCISE 11: Backtest Engine**

### **Objective**
Simple backtest engine to test strategies on historical data.

### **Specifications**
```cpp
struct Trade {
    long timestamp;
    std::string symbol;
    double price;
    int quantity;  // positive = buy, negative = sell
};

class BacktestEngine {
public:
    BacktestEngine(double initialCash = 100000.0);
    
    // Load historical data
    void loadData(const std::string& filename);
    
    // Add a simple strategy
    void setStrategy(std::function<int(const Tick&, double)> strategy);
    
    // Run the backtest
    void run();
    
    // Performance metrics
    double getTotalReturn() const;
    double getSharpeRatio() const;
    double getMaxDrawdown() const;
    
    // Trade history
    std::vector<Trade> getTrades() const;
    
private:
    double cash_;
    std::map<std::string, int> positions_;  // symbol -> quantity
    std::vector<Tick> historical_data_;
    std::vector<Trade> trades_;
};
```

### **Constraints**
- Handle cash + stock positions
- Calculate mark-to-market PnL
- Strategy = function returning quantity to buy/sell
- Standard financial metrics

### **Strategy Example**
```cpp
// Simple momentum strategy
auto momentum_strategy = [](const Tick& tick, double current_price) -> int {
    static double prev_price = 0;
    if (prev_price > 0 && tick.price > prev_price * 1.01) {
        return 100;  // buy 100 shares
    }
    prev_price = tick.price;
    return 0;
};
```

---

# **EXERCISE 12: Market Data Feed Simulator**

## **Objective**
Implement an asynchronous market data feed simulator with backpressure handling and performance monitoring.

## **Specifications**
```cpp
class MarketDataFeed {
public:
    using TickCallback = std::function<void(const Tick&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    MarketDataFeed();
    ~MarketDataFeed();
    
    // Start/stop the feed
    void start();
    void stop();
    
    // Subscribe to symbols
    void subscribe(const std::string& symbol, TickCallback onTick, ErrorCallback onError);
    void unsubscribe(const std::string& symbol);
    
    // Control tick frequency
    void setTickRate(const std::string& symbol, int ticksPerSecond);
    
    // Performance statistics
    uint64_t getTicksReceived(const std::string& symbol) const;
    uint64_t getTicksDropped(const std::string& symbol) const;
    double getAverageLatency() const;
    
private:
    struct SymbolData {
        double lastPrice = 100.0;
        std::atomic<uint64_t> ticksReceived{0};
        std::atomic<uint64_t> ticksDropped{0};
        int tickRate = 1000; // ticks per second
        TickCallback callback;
        ErrorCallback errorCallback;
    };
    
    std::unordered_map<std::string, SymbolData> symbols_;
    std::atomic<bool> running_{false};
    std::vector<std::thread> workers_;
    mutable std::shared_mutex symbolsMutex_;
    
    // Simulate realistic price movements
    double generateNextPrice(const std::string& symbol, double currentPrice);
    void feedWorker(const std::string& symbol);
};
```

## **Constraints**
- Use **std::async** or custom thread pool for simulation
- Handle **backpressure**: drop ticks if consumer is too slow
- Implement **realistic price simulation** (Brownian motion + volatility)
- **Thread-safe** subscribe/unsubscribe operations
- **Clean shutdown** of all threads
- Use **C++17/20 features**: std::shared_mutex, std::atomic, etc.

## **Usage Example**
```cpp
MarketDataFeed feed;
std::atomic<int> receivedCount{0};

// Subscribe with lambda callback
feed.subscribe("AAPL", [&](const Tick& tick) {
    receivedCount++;
    // Process tick (simulate slow consumer occasionally)
    if (receivedCount % 1000 == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}, [](const std::string& error) {
    std::cerr << "Error: " << error << std::endl;
});

feed.setTickRate("AAPL", 10000); // 10k ticks/sec
feed.start();

// Monitor performance
std::this_thread::sleep_for(std::chrono::seconds(5));
std::cout << "Received: " << feed.getTicksReceived("AAPL") << std::endl;
std::cout << "Dropped: " << feed.getTicksDropped("AAPL") << std::endl;
std::cout << "Avg Latency: " << feed.getAverageLatency() << "ms" << std::endl;

feed.stop();
```

---

# **EXERCISE 13: Async Order Router**

## **Objective**
Implement an asynchronous order router with retry logic, timeout handling, and comprehensive monitoring.

## **Specifications**
```cpp
enum class OrderStatus { PENDING, FILLED, REJECTED, CANCELLED, PARTIAL };

struct OrderResult {
    int orderId;
    OrderStatus status;
    double filledPrice;
    int filledQuantity;
    std::string message;
    std::chrono::milliseconds latency;
    std::string exchangeName;
};

class AsyncOrderRouter {
public:
    using OrderCallback = std::function<void(const OrderResult&)>;
    
    AsyncOrderRouter();
    ~AsyncOrderRouter();
    
    // Send order asynchronously
    std::future<OrderResult> sendOrderAsync(
        const std::string& symbol,
        Side side,
        double price,
        int quantity,
        OrderCallback callback = nullptr
    );
    
    // Cancel order
    std::future<bool> cancelOrderAsync(int orderId);
    
    // Configuration
    void setMaxRetries(int retries);
    void setTimeoutMs(int timeoutMs);
    void addExchange(const std::string& exchangeName, int avgLatencyMs, double successRate);
    
    // Monitoring and statistics
    double getAverageLatency() const;
    double getSuccessRate() const;
    uint64_t getTotalOrders() const;
    
    // Health check
    bool isExchangeHealthy(const std::string& exchangeName) const;
    
private:
    struct ExchangeConfig {
        std::string name;
        int avgLatencyMs;
        double successRate;
        std::atomic<uint64_t> totalOrders{0};
        std::atomic<uint64_t> successfulOrders{0};
        std::atomic<uint64_t> totalLatencyMs{0};
    };
    
    struct PendingOrder {
        int orderId;
        std::string symbol;
        Side side;
        double price;
        int quantity;
        std::promise<OrderResult> promise;
        OrderCallback callback;
        std::chrono::steady_clock::time_point startTime;
        int attemptCount = 0;
    };
    
    std::unordered_map<std::string, ExchangeConfig> exchanges_;
    std::unordered_map<int, std::unique_ptr<PendingOrder>> pendingOrders_;
    
    std::atomic<int> nextOrderId_{1};
    std::atomic<int> maxRetries_{3};
    std::atomic<int> timeoutMs_{1000};
    
    mutable std::shared_mutex exchangesMutex_;
    mutable std::mutex pendingOrdersMutex_;
    
    // Thread pool for order processing
    std::vector<std::thread> workers_;
    ThreadSafeQueue<std::function<void()>> taskQueue_;
    std::atomic<bool> running_{false};
    
    // Core functionality
    std::future<OrderResult> sendToExchange(
        const std::string& exchangeName, 
        const PendingOrder& order
    );
    
    void retryOrder(std::shared_ptr<PendingOrder> order);
    void workerThread();
    std::string selectBestExchange(const std::string& symbol) const;
    
    // Simulation helpers
    OrderResult simulateExchangeResponse(
        const ExchangeConfig& exchange, 
        const PendingOrder& order
    ) const;
};
```

## **Constraints**
- Use **std::future** and **std::promise** for async operations
- Implement **automatic retry** with exponential backoff on failure
- **Configurable timeout** per order
- **Simulate network latency** and variable success rates
- **Thread-safe** operations with minimal locking
- Implement **circuit breaker pattern** (bonus)
- Use **C++17/20 features**: std::optional, structured bindings, etc.

## **Usage Example**
```cpp
AsyncOrderRouter router;

// Configure exchanges with different characteristics
router.addExchange("NYSE", 50, 0.98);    // 50ms avg latency, 98% success
router.addExchange("NASDAQ", 30, 0.95);  // 30ms avg latency, 95% success
router.addExchange("BATS", 20, 0.99);    // 20ms avg latency, 99% success

router.setMaxRetries(3);
router.setTimeoutMs(500);

// Send order with callback
auto future = router.sendOrderAsync("AAPL", BUY, 150.0, 100, 
    [](const OrderResult& result) {
        if (result.status == OrderStatus::FILLED) {
            std::cout << "Order " << result.orderId << " filled at " 
                      << result.filledPrice << std::endl;
        }
    });

// Wait for result or continue processing
try {
    auto result = future.get();
    std::cout << "Final status: " << static_cast<int>(result.status) << std::endl;
    std::cout << "Latency: " << result.latency.count() << "ms" << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Order failed: " << e.what() << std::endl;
}

// Monitor performance
std::cout << "Success Rate: " << router.getSuccessRate() * 100 << "%" << std::endl;
std::cout << "Average Latency: " << router.getAverageLatency() << "ms" << std::endl;
```

---
