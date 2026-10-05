#include "thread_pool.hpp"

// ThreadPool is the underlying datastructre of the threads
ThreadPool::ThreadPool(ThreadSafeQueue<std::shared_ptr<Actor>> &queue)
    : queue_(queue) {
  unsigned const count = std::thread::hardware_concurrency();
  try {
    for (unsigned i = 0; i < count; ++i) {
      threads_.emplace_back(&ThreadPool::WorkerThread, this);
    }
  } catch (...) {
    queue_.Shutdown();
    for (auto &t : threads_) {
      if (t.joinable()) {
        t.join();
      }
    }
    throw;
  }
}

ThreadPool::~ThreadPool() {
  queue_.Shutdown();
  for (auto &t : threads_) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void ThreadPool::WorkerThread() {
  std::shared_ptr<Actor> actor;
  while (queue_.Pop(actor)) {
    actor->Run();
  }
}
