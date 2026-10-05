#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "actor.hpp"
#include "threadsafe_queue.hpp"
#include <memory>
#include <thread>
#include <vector>

// A fixed pool of worker threads. Workers consume actors from a queue that is
// OWNED BY the Schedular; the ThreadPool only holds a reference to it.
class ThreadPool {
public:
  explicit ThreadPool(ThreadSafeQueue<std::shared_ptr<Actor>> &queue);

  // Signals shutdown and joins all workers.
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

private:
  ThreadSafeQueue<std::shared_ptr<Actor>> &queue_;
  std::vector<std::thread> threads_;

  void WorkerThread();
};

#endif // THREAD_POOL_HPP
