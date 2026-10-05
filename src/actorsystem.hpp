#ifndef ACTOR_SYSTEM_HPP
#define ACTOR_SYSTEM_HPP

#include "schedular.hpp"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

// The ActorSystem is the composition root: it owns the Schedular (and thus the
// thread pool) and a registry of every spawned actor. It is the only thing a
// user talks to -- create it, spawn actors, send messages, let it die.
// The registry keeps actors
// alive; destroying the ActorSystem tears everything down (the Schedular's
// member ThreadPool shuts down and joins its threads on destruction).
class ActorSystem {
public:
  ActorSystem();

  // Factory: construct a T, keep it alive in the registry, and register its
  // mailbox with the scheduler. Returns the typed handle. Throws if `name` is
  // already taken.
  template <typename T, typename... Args>
  std::shared_ptr<T> spawn(const std::string &name, Args &&...args) {
    static_assert(std::is_base_of_v<Actor, T>,
                  "spawn<T>: T must derive from Actor");
    auto actor = std::make_shared<T>(std::forward<Args>(args)...);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (registry_.count(name)) {
        throw std::invalid_argument("duplicate actor name '" + name + "'");
      }
      registry_.emplace(name, actor);
    }
    schedular_.Register(actor);
    return actor;
  }

  // Look up an actor by name; nullptr if not found.
  std::shared_ptr<Actor> get(const std::string &name);

private:
  Schedular schedular_;
  std::mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<Actor>> registry_;
};

#endif // ACTOR_SYSTEM_HPP
