#include "/actorsystem.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

struct Ping : Message {};

// Counts messages; on the `target`-th one, fires the completion promise.
class Sink : public Actor {
public:
  Sink(std::shared_ptr<std::atomic<std::uint64_t>> counter,
       std::uint64_t target, std::shared_ptr<std::promise<void>> done)
      : counter_(std::move(counter)), target_(target), done_(std::move(done)) {}

  void Receive(const std::shared_ptr<Message> &) override {
    if (counter_->fetch_add(1, std::memory_order_relaxed) + 1 == target_) {
      done_->set_value();
    }
  }

private:
  std::shared_ptr<std::atomic<std::uint64_t>> counter_;
  std::uint64_t target_;
  std::shared_ptr<std::promise<void>> done_;
};

double measure(std::size_t producers, std::size_t sinks,
               std::uint64_t per_producer) {
  ActorSystem system;

  const std::uint64_t total = producers * per_producer;

  auto counter = std::make_shared<std::atomic<std::uint64_t>>(0);
  auto done = std::make_shared<std::promise<void>>();
  auto done_fut = done->get_future();

  std::vector<std::shared_ptr<Actor>> sink_actors;
  for (std::size_t s = 0; s < sinks; ++s) {
    sink_actors.push_back(
        system.spawn<Sink>("sink" + std::to_string(s), counter, total, done));
  }

  std::promise<void> start;
  std::shared_future<void> go = start.get_future().share();

  std::vector<std::thread> threads;
  for (std::size_t p = 0; p < producers; ++p) {
    threads.emplace_back([p, per_producer, sinks, &sink_actors, go] {
      auto sink = sink_actors[p % sinks];
      go.wait();
      for (std::uint64_t i = 0; i < per_producer; ++i) {
        sink->Send(std::make_shared<Ping>());
      }
    });
  }

  auto t0 = std::chrono::steady_clock::now();
  start.set_value();
  done_fut.wait();
  auto t1 = std::chrono::steady_clock::now();

  for (auto &t : threads) {
    t.join();
  }

  double seconds = std::chrono::duration<double>(t1 - t0).count();
  return static_cast<double>(total) / seconds;
}

int main() {
  const auto hw = std::thread::hardware_concurrency();
  const std::uint64_t N = 2000000;

  measure(1, 1, 200000); // warmup: allocations, thread wake, CPU frequency

  auto report = [](const char *name, double rate) {
    std::cout << std::left << std::setw(24) << name << " : " << std::fixed
              << std::setprecision(0) << rate << " msgs/sec\n";
  };

  report("1 producer -> 1 actor", measure(1, 1, N));
  report("P producers -> 1 actor", measure(hw, 1, N));
  report("P producers -> P actors", measure(hw, hw, N));

  return 0;
}
