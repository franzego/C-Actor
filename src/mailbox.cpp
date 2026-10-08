#include "mailbox.hpp"

void Mailbox::SetScheduleCallback(std::function<void()> cb) {
  schedule_ = std::move(cb);
}

void Mailbox::Push(std::shared_ptr<Message> msg) {
  queue_.enqueue(std::move(msg));

  // Only the empty -> non-empty transition triggers a schedule.
  if (count_.fetch_add(1) == 0) {
    if (!scheduled_.exchange(true)) {
      if (schedule_) {
        schedule_(); // push owning actor onto the scheduler
      }
    }
  }
}

bool Mailbox::TryPop(std::shared_ptr<Message> &msg) {
  if (queue_.try_dequeue(msg)) {
    count_.fetch_sub(1);
    return true;
  }

  // Empty -> mark idle, but double-check a producer didn't slip a message in
  // concurrently (lost-wakeup guard). If one did, re-schedule and let the next
  // Run() pick it up.
  scheduled_.store(false);
  if (count_.load() > 0) {
    if (!scheduled_.exchange(true)) {
      if (schedule_) {
        schedule_();
      }
    }
  }
  return false;
}
