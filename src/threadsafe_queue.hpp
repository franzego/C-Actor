#ifndef THREADSAFE_QUEUE_HPP
#define THREADSAFE_QUEUE_HPP
#include <atomic>
#include <condition_variable>
#include <immintrin.h>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

inline void spin_pause() {
#if defined(__x86_64__) || defined(_M_X64)
  _mm_pause();
#else
  std::this_thread::yield();
#endif
}

// Thread-safe MPMC FIFO queue. Pop blocks until an item is available (or the
// queue is shut down), but spins briefly first to catch fast arrivals without
// a context switch.
template <typename T> class ThreadSafeQueue {
public:
  ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

  void Push(const T &item);
  void Push(T &&item);

  bool Pop(T &item);
  bool TryPop(T &item);

  void Shutdown();

private:
  static constexpr int kSpinTight = 1000;
  static constexpr int kSpinYield = 16;

  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::atomic<bool> done_{false};
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

template <typename T> bool ThreadSafeQueue<T>::TryPop(T &item) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    return false;
  }
  item = std::move(queue_.front());
  queue_.pop();
  return true;
}

template <typename T> bool ThreadSafeQueue<T>::Pop(T &item) {
  for (int i = 0; i < kSpinTight && !done_; ++i) {
    if (TryPop(item)) {
      return true;
    }
    spin_pause();
  }
  for (int i = 0; i < kSpinYield && !done_; ++i) {
    if (TryPop(item)) {
      return true;
    }
    std::this_thread::yield();
  }
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty() || done_; });
  if (queue_.empty()) {
    return false;
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
