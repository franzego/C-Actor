#include "mailbox.hpp"

void Mailbox::SetScheduleCallback(std::function<void()> cb) {
  schedule_ = std::move(cb);
}

// Actors push messages into the Mailbox which eventually schedules the actor on
// the schedular queue
void Mailbox::Push(std::shared_ptr<Message> msg) {
  bool should_schedule = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    bool was_empty = queue_.empty();
    queue_.push_back(std::move(msg));

    // Only the empty -> non-empty transition while NOT already scheduled
    // triggers a schedule. Further messages just pile up in the queue.
    should_schedule = was_empty && !scheduled_;
    if (should_schedule) {
      scheduled_ = true;
    }
  }
  if (should_schedule && schedule_) {
    schedule_(); // push owning actor onto the scheduler, outside the lock
  }
}

// The threadpool assigns a thread worker to pop up a message from the mailbox
// to be acted on
bool Mailbox::TryPop(std::shared_ptr<Message> &msg) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) {
    scheduled_ = false; // fully drained -> actor is idle again
    return false;
  }
  msg = std::move(queue_.front());
  queue_.pop_front();
  return true;
}
