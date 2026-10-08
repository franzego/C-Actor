#include "actor.hpp"

void Actor::Send(Message msg) {
  mailbox_.Push(std::move(msg));
}

void Actor::Run() {
  Message msg;
  while (mailbox_.TryPop(msg)) {
    Receive(msg);
  }
}

void Actor::SetScheduleCallback(std::function<void()> cb) {
  mailbox_.SetScheduleCallback(std::move(cb));
}
