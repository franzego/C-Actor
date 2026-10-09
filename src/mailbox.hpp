#ifndef MAILBOX_HPP
#define MAILBOX_HPP

#include "concurrentqueue.h"
#include "message.hpp"
#include <atomic>
#include <cstdint>
#include <functional>

// A mailbox is the FIFO queue of messages owned by a single actor.
//
// Producers: any thread sending a message to the actor calls Push().
// Consumer:  the single worker (from the threadpool) that has popped the actor
//            off the scheduler calls TryPop() until it returns false.
//
// The queue is lock-free (moodycamel ConcurrentQueue), messages are stored by
// value (std::variant). Scheduling is decided by two atomics:
//   - count_:     number of queued messages (drives the empty->non-empty check)
//   - scheduled_: whether the actor is already in the scheduler queue
// This tolerates a rare double-schedule (harmless) while guaranteeing no
// message is ever stranded.
class Mailbox {
public:
  Mailbox() = default;

  // Wire the actor's "push me onto the scheduler" hook.
  void SetScheduleCallback(std::function<void()> cb);

  // Producer side. Any thread may call this.
  void Push(Message msg);

  // Consumer side. Only the worker currently owning the actor calls this.
  bool TryPop(Message &msg);

private:
  moodycamel::ConcurrentQueue<Message> queue_;
  std::atomic<std::uint64_t> offset_{0};
  std::atomic<bool> scheduled_{false};
  std::function<void()> schedule_;
};

#endif // MAILBOX_HPP
