#include <cstddef>
#include <iostream>
#include <thread>

template <typename T> class ThreadSafeQueue {
  private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    size_t max_size_;
    bool shutdown_;

  public:
    explicit ThreadSafeQueue(size_t maxSize = 1000) : max_size_(maxSize), shutdown_(false) {}

    ~ThreadSafeQueue() {
        shutdown();
    }

    // Disable copy constructor and assignment operator
    ThreadSafeQueue(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

    // Add an element (blocks if full)
    void push(const T &item) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this]() { return queue_.size() < max_size_ || shutdown_; });
        if (shutdown_) {
            throw std::runtime_error("Queue is shutting down.");
        }
        queue_.push(item);
        notEmpty_.notify_one();
    }

    // Move version of push
    void push(T &&item) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this]() { return queue_.size() < max_size_ || shutdown_; });
        if (shutdown_) {
            throw std::runtime_error("Queue is shutting down.");
        }
        queue_.push(std::move(item));
        notEmpty_.notify_one();
    }

    // Non-blocking version (returns false if full)
    bool tryPush(const T &item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= max_size_ || shutdown_) {
            return false;
        }

        queue_.push(item);
        notEmpty_.notify_one();
        return true;
    }

    // Move version of tryPush
    bool tryPush(T &&item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= max_size_ || shutdown_) {
            return false;
        }

        queue_.push(std::move(item));
        notEmpty_.notify_one();
        return true;
    }

    // Get an element (blocks if empty)
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [this]() { return !queue_.empty() || shutdown_; });

        if (shutdown_) {
            throw std::runtime_error("Queue is shutting down.");
        }

        T item = std::move(queue_.front());
        queue_.pop();
        notFull_.notify_one();
        return item;
    }

    // Non-blocking version (returns false if empty)
    bool tryPop(T &item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty() || shutdown_) return false;
        item = std::move(queue_.front());
        queue_.pop();
        notFull_.notify_one();
        return true;
    }

    // Current size
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // Clean shutdown (for thread termination)
    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    // Check if shutdown
    bool isShutdown() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return shutdown_;
    }
};

// Example usage and testing
int main() {
    ThreadSafeQueue<int> queue(5); // Small queue for demonstration

    std::cout << "=== Thread-Safe Queue Demo ===" << std::endl;

    // Producer thread
    std::thread producer([&queue]() {
        try {
            for (int i = 1; i <= 10; ++i) {
                queue.push(i);
                std::cout << "Producer: Added " << i << " (queue size: " << queue.size() << ")"
                          << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::cout << "Producer: Finished" << std::endl;
        } catch (const std::exception &e) {
            std::cout << "Producer exception: " << e.what() << std::endl;
        }
    });

    // Consumer thread
    std::thread consumer([&queue]() {
        try {
            while (true) {
                int value = queue.pop();
                std::cout << "Consumer: Got " << value << " (queue size: " << queue.size() << ")"
                          << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
        } catch (const std::exception &e) {
            std::cout << "Consumer exception: " << e.what() << std::endl;
        }
    });

    // Let them run for a while
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Test tryPush and tryPop
    std::cout << "\n=== Testing non-blocking operations ===" << std::endl;

    // Fill the queue
    for (int i = 1; i < 7; ++i) {
        if (queue.tryPush(i)) {
            std::cout << "Successfully pushed " << i << std::endl;
        } else {
            std::cout << "Failed to push " << i << " (queue full or shutdown)" << std::endl;
        }
    }

    // Try to pop some items
    int item;
    for (int i = 0; i < 3; ++i) {
        if (queue.tryPop(item)) {
            std::cout << "Successfully popped " << item << std::endl;
        } else {
            std::cout << "Failed to pop (queue empty)" << std::endl;
        }
    }
 
    // Shutdown the queue
    std::cout << "\n=== Shutting down ===" << std::endl;
    queue.shutdown();

    // Wait for threads to finish
    producer.join();
    consumer.join();

    std::cout << "Demo completed!" << std::endl;

    return 0;
}