#ifndef MAILBOX_HPP
#define MAILBOX_HPP

#include "message.hpp"
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

// A mailbox is the FIFO queue of messages owned by a single actor.
//
// Producers: any thread sending a message to the actor calls Push().
// Consumer:  the single worker (from the threadpool) that has popped the actor
//            off the scheduler calls TryPop() until it returns false.
//
// scheduled_ guarantees the actor sits in the scheduler queue at most once.
// A message arriving while the actor is already scheduled just waits in the
// queue and is picked up on the next Run().
class Mailbox {
public:
  Mailbox() = default;

  // Wire the actor's "push me onto the scheduler" hook.
  void SetScheduleCallback(std::function<void()> cb);

  // Producer side. Any thread may call this. An actor on receiving a message,
  // calls the push
  void Push(std::shared_ptr<Message> msg);

  // Consumer side. Only the worker thread currently owning the actor calls
  // this. Pops the next pending message (FIFO) into `msg` and returns true;
  // returns false when empty, marking the actor idle again.
  bool TryPop(std::shared_ptr<Message> &msg);

private:
  std::deque<std::shared_ptr<Message>> queue_;
  mutable std::mutex mutex_;
  std::function<void()> schedule_;
  bool scheduled_ = false; // true => queued or being drained
};

#endif // MAILBOX_HPP
