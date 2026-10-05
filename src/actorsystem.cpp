#include "actorsystem.hpp"

ActorSystem::ActorSystem() = default;

std::shared_ptr<Actor> ActorSystem::get(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = registry_.find(name);
  return it == registry_.end() ? nullptr : it->second;
}
