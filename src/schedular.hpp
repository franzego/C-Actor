#ifndef SCHEDULAR_HPP
#define SCHEDULAR_HPP

#include "actor.hpp"
#include "thread_pool.hpp"
#include "threadsafe_queue.hpp"
#include <memory>

// The Schedular owns the queue of runnable actors AND the thread pool that
// consumes it. Actors are the producers: when a message lands in an actor's
// mailbox, the mailbox fires the callback set by Register(), pushing the actor
// onto the queue.
class Schedular {
public:
  Schedular();

  // Producer side: enqueue an actor to be run by the pool.
  void Push(std::shared_ptr<Actor> actor);

  // Wire `actor` so that its mailbox schedules it here when it gets a message.
  void Register(std::shared_ptr<Actor> actor);

private:
  ThreadSafeQueue<std::shared_ptr<Actor>> queue_;
  ThreadPool threadpool_; // constructed with &queue_
};

#endif // SCHEDULAR_HPP
