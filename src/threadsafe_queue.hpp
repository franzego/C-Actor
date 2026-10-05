#ifndef THREADSAFE_QUEUE_HPP
#define THREADSAFE_QUEUE_HPP
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

// Thread-safe MPMC FIFO queue: multiple producers Push, multiple consumers Pop.
// Pop blocks until an item is available or Shutdown() is called.
//
// NOTE: this is a class template, so the definitions live here in the header.
// It is the underlying structure of the Schedular.
template <typename T> class ThreadSafeQueue {
private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::atomic<bool> done_{false};

public:
  ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

  void Push(const T &item);
  void Push(T &&item);

  // Blocks until an item is available. Returns false once the queue has been
  // shut down and drained; true otherwise (with the item moved into `item`).
  bool Pop(T &item);

  // Wakes all blocked consumers; subsequent Pops return false once empty.
  void Shutdown();
};

template <typename T> void ThreadSafeQueue<T>::Push(const T &item) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);
  }
  cond_.notify_one();
}

template <typename T> void ThreadSafeQueue<T>::Push(T &&item) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(item));
  }
  cond_.notify_one();
}

template <typename T> bool ThreadSafeQueue<T>::Pop(T &item) {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty() || done_; });
  if (queue_.empty()) {
    return false; // shut down and drained
  }
  item = std::move(queue_.front());
  queue_.pop();
  return true;
}

template <typename T> void ThreadSafeQueue<T>::Shutdown() {
  done_ = true;
  cond_.notify_all();
}

#endif
