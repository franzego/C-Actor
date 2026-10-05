#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "mailbox.hpp"
#include <functional>
#include <memory>

// An actor is the unit of work. It holds a mailbox; other actors (or external
// code) send it messages via Send(). When the scheduler runs the actor, Run()
// drains the mailbox and dispatches each message to the subclass's Receive().
class Actor {
public:
  virtual ~Actor() = default;

  // Concrete message handling: It receives a message from another actor or even
  // a client.
  virtual void Receive(const std::shared_ptr<Message> &msg) = 0;

  // Producer side. Any thread may call this to deliver a message to another
  // actor.
  void Send(std::shared_ptr<Message> msg);

  // Consumer side. Called by a worker thread after the actor is popped off
  // the scheduler. Drains the mailbox in FIFO order.
  void Run();

  // Wire the mailbox's "schedule me" hook (set by the Schedular).
  void SetScheduleCallback(std::function<void()> cb);

private:
  Mailbox mailbox_;
};

#endif // ACTOR_HPP
