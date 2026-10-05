#include "actor.hpp"

void Actor::Send(std::shared_ptr<Message> msg) {
  mailbox_.Push(std::move(msg));
}

void Actor::Run() {
  std::shared_ptr<Message> msg;
  while (mailbox_.TryPop(msg)) {
    Receive(msg);
  }
}

void Actor::SetScheduleCallback(std::function<void()> cb) {
  mailbox_.SetScheduleCallback(std::move(cb));
}
