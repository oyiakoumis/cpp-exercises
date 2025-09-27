#include <cstddef>
#include <iostream>
#include <thread>

template <typename T> class ThreadSafeQueue {
  public:
    explicit ThreadSafeQueue(size_t maxSize = 1000);

    ~ThreadSafeQueue();

    // Disable copy constructor and assignment operator
    ThreadSafeQueue(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

    // Add an element (blocks if full)
    void push(const T &item);

    // Move version of push
    void push(T &&item);

    // Non-blocking version (returns false if full)
    bool tryPush(const T &item);

        // Move version of tryPush
        bool tryPush(T &&item);

    // Get an element (blocks if empty)
    T pop();

    // Non-blocking version (returns false if empty)
    bool tryPop(T &item);

    // Current size
    size_t size() const;

    // Check if queue is empty
    bool empty() const;

    // Clean shutdown (for thread termination)
    void shutdown();

    // Check if shutdown
    bool isShutdown() const;
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
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Test tryPush and tryPop
    std::cout << "\n=== Testing non-blocking operations ===" << std::endl;

    // Fill the queue
    for (int i = 100; i < 106; ++i) {
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