#include "schedular.hpp"

Schedular::Schedular() : threadpool_(queue_) {}

void Schedular::Push(std::shared_ptr<Actor> actor) {
  queue_.Push(std::move(actor));
}

void Schedular::Register(std::shared_ptr<Actor> actor) {
  // Capture a weak_ptr, NOT the shared_ptr: the mailbox holds this callback for
  // the actor's whole lifetime, and a strong capture would be a self-cycle
  // (actor -> mailbox -> callback -> actor), leaking the actor forever.
  std::weak_ptr<Actor> weak = actor;
  actor->SetScheduleCallback([this, weak] {
    if (auto sp = weak.lock()) {
      queue_.Push(std::move(sp));
    }
  });
}
