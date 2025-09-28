#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

#include "../include/ThreadSafeQueue.hpp"

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(size_t maxSize) : max_size_(maxSize), shutdown_(false) {}

template <typename T> ThreadSafeQueue<T>::~ThreadSafeQueue() {
    shutdown();
}

// Add an element (blocks if full)
template <typename T> void ThreadSafeQueue<T>::push(const T &item) {
    std::unique_lock<std::mutex> lock(mutex_);
    notFull_.wait(lock, [this]() { return queue_.size() < max_size_ || shutdown_; });
    if (shutdown_) {
        throw std::runtime_error("Queue is shutting down.");
    }
    queue_.push(item);
    notEmpty_.notify_one();
}

// Move version of push
template <typename T> void ThreadSafeQueue<T>::push(T &&item) {
    std::unique_lock<std::mutex> lock(mutex_);
    notFull_.wait(lock, [this]() { return queue_.size() < max_size_ || shutdown_; });
    if (shutdown_) {
        throw std::runtime_error("Queue is shutting down.");
    }
    queue_.push(std::move(item));
    notEmpty_.notify_one();
}

// Non-blocking version (returns false if full)
template <typename T> bool ThreadSafeQueue<T>::tryPush(const T &item) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.size() >= max_size_ || shutdown_) {
        return false;
    }

    queue_.push(item);
    notEmpty_.notify_one();
    return true;
}

// Move version of tryPush
template <typename T> bool ThreadSafeQueue<T>::tryPush(T &&item) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.size() >= max_size_ || shutdown_) {
        return false;
    }

    queue_.push(std::move(item));
    notEmpty_.notify_one();
    return true;
}

// Get an element (blocks if empty)
template <typename T> T ThreadSafeQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    notEmpty_.wait(lock, [this]() { return !queue_.empty() || shutdown_; });

    if (shutdown_ && queue_.empty()) {
        throw std::runtime_error("Queue is shutting down.");
    }

    T item = std::move(queue_.front());
    queue_.pop();
    notFull_.notify_one();
    return item;
}

// Non-blocking version (returns false if empty)
template <typename T> bool ThreadSafeQueue<T>::tryPop(T &item) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return false;

    item = std::move(queue_.front());
    queue_.pop();
    notFull_.notify_one();
    return true;
}

// Current size
template <typename T> size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

// Check if queue is empty
template <typename T> bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

// Clean shutdown (for thread termination)
template <typename T> void ThreadSafeQueue<T>::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
    notEmpty_.notify_all();
    notFull_.notify_all();
}

// Check if shutdown
template <typename T> bool ThreadSafeQueue<T>::isShutdown() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return shutdown_;
}

// Example usage and testing
int main() {
    std::mutex cout_mutex; // Mutex for synchronized console output

    std::cout << "=== Thread-Safe Queue Demo ===" << std::endl;

    // Test 1: Basic Producer-Consumer
    {
        std::cout << "\n--- Test 1: Basic Producer-Consumer ---" << std::endl;
        ThreadSafeQueue<int> queue(5); // Small queue for demonstration
        std::atomic<bool> stop_consumer{false};

        // Producer thread
        std::thread producer([&queue, &cout_mutex]() {
            try {
                for (int i = 1; i <= 10; ++i) {
                    queue.push(i);
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "Producer: Added " << i << " (queue size: " << queue.size()
                                  << ")" << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Producer: Finished" << std::endl;
                }
            } catch (const std::exception &e) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Producer exception: " << e.what() << std::endl;
            }
        });

        // Consumer thread
        std::thread consumer([&queue, &cout_mutex, &stop_consumer]() {
            try {
                while (!stop_consumer) {
                    // Use tryPop with timeout to allow checking stop flag
                    int value;
                    if (queue.tryPop(value)) {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "Consumer: Got " << value << " (queue size: " << queue.size()
                                  << ")" << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                }

                // Drain remaining items after stop signal
                int value;
                while (queue.tryPop(value)) {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Consumer: Draining " << value << " (queue size: " << queue.size()
                              << ")" << std::endl;
                }

                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Consumer: Stopped gracefully" << std::endl;
                }
            } catch (const std::exception &e) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Consumer exception: " << e.what() << std::endl;
            }
        });

        // Wait for producer to finish
        producer.join();

        // Let consumer process remaining items
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Signal consumer to stop
        stop_consumer = true;
        consumer.join();
    }

    // Test 2: Non-blocking operations on a fresh queue
    {
        std::cout << "\n--- Test 2: Non-blocking operations (tryPush/tryPop) ---" << std::endl;
        ThreadSafeQueue<int> queue(5);

        // Try to fill the queue beyond capacity
        std::cout << "Attempting to push 7 items into queue with capacity 5:" << std::endl;
        for (int i = 1; i <= 7; ++i) {
            if (queue.tryPush(i)) {
                std::cout << "  ✓ Successfully pushed " << i << " (queue size: " << queue.size()
                          << ")" << std::endl;
            } else {
                std::cout << "  ✗ Failed to push " << i << " (queue full, size: " << queue.size()
                          << ")" << std::endl;
            }
        }

        // Try to pop some items
        std::cout << "\nPopping 3 items:" << std::endl;
        int item;
        for (int i = 0; i < 3; ++i) {
            if (queue.tryPop(item)) {
                std::cout << "  ✓ Successfully popped " << item << " (queue size: " << queue.size()
                          << ")" << std::endl;
            } else {
                std::cout << "  ✗ Failed to pop (queue empty)" << std::endl;
            }
        }

        // Try to push again after making space
        std::cout << "\nTrying to push again after making space:" << std::endl;
        if (queue.tryPush(100)) {
            std::cout << "  ✓ Successfully pushed 100 (queue size: " << queue.size() << ")"
                      << std::endl;
        }
    }

    // Test 3: Shutdown behavior
    {
        std::cout << "\n--- Test 3: Shutdown behavior ---" << std::endl;
        ThreadSafeQueue<int> queue(10);

        // Add some items
        std::cout << "Adding items before shutdown:" << std::endl;
        for (int i = 1; i <= 5; ++i) {
            queue.push(i);
            std::cout << "  Added " << i << std::endl;
        }

        std::cout << "Queue size before shutdown: " << queue.size() << std::endl;

        // Create consumer that will try to pop after shutdown
        std::thread consumer([&queue, &cout_mutex]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            try {
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Consumer: Attempting to pop after shutdown..." << std::endl;
                }
                int value = queue.pop(); // This should throw
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Consumer: Got " << value << std::endl;
                }
            } catch (const std::exception &e) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Consumer: Caught exception: " << e.what() << std::endl;
            }
        });

        // Shutdown the queue
        std::cout << "Shutting down queue..." << std::endl;
        queue.shutdown();
        std::cout << "Queue is shutdown: " << (queue.isShutdown() ? "true" : "false") << std::endl;

        // Try operations after shutdown
        std::cout << "Trying tryPush after shutdown: ";
        if (!queue.tryPush(99)) {
            std::cout << "Failed (as expected)" << std::endl;
        }

        consumer.join();
    }

    // Test 4: Multiple producers and consumers
    {
        std::cout << "\n--- Test 4: Multiple producers and consumers ---" << std::endl;
        ThreadSafeQueue<int> queue(10);
        std::atomic<int> produced{0};
        std::atomic<int> consumed{0};
        const int items_per_producer = 5;
        const int num_producers = 3;
        const int num_consumers = 2;

        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        std::atomic<bool> stop_consumers{false};

        // Start producers
        for (int p = 0; p < num_producers; ++p) {
            producers.emplace_back([&queue, &cout_mutex, &produced, p]() {
                for (int i = 0; i < items_per_producer; ++i) {
                    int value = p * 100 + i;
                    queue.push(value);
                    produced++;
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "Producer " << p << ": Added " << value << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            });
        }

        // Start consumers
        for (int c = 0; c < num_consumers; ++c) {
            consumers.emplace_back([&queue, &cout_mutex, &consumed, &stop_consumers, c]() {
                while (!stop_consumers || !queue.empty()) {
                    int value;
                    if (queue.tryPop(value)) {
                        consumed++;
                        {
                            std::lock_guard<std::mutex> lock(cout_mutex);
                            std::cout << "Consumer " << c << ": Got " << value << std::endl;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(70));
                }
            });
        }

        // Wait for all producers
        for (auto &t : producers) {
            t.join();
        }

        std::cout << "All producers finished. Waiting for consumers to drain queue..." << std::endl;

        // Wait a bit then stop consumers
        while (!queue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        stop_consumers = true;

        // Wait for all consumers
        for (auto &t : consumers) {
            t.join();
        }

        std::cout << "Statistics: Produced=" << produced << ", Consumed=" << consumed << std::endl;
    }

    std::cout << "\n=== All tests completed! ===" << std::endl;
    return 0;
}