#ifndef THREADSAFE_HPP
#define THREADSAFE_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

template <typename T> class ThreadSafeQueue {
  private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    size_t max_size_;
    bool shutdown_;


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

#endif