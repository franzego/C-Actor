#include "actorsystem.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

struct Ping : Message {
  std::chrono::steady_clock::time_point t0;
  explicit Ping(std::chrono::steady_clock::time_point t) : t0(t) {}
};

struct Start : Message {};

class Ponger : public Actor {
public:
  explicit Ponger(std::weak_ptr<Actor> pinger) : pinger_(std::move(pinger)) {}

  void Receive(const std::shared_ptr<Message> &msg) override {
    if (auto p = pinger_.lock()) {
      p->Send(msg);
    }
  }

private:
  std::weak_ptr<Actor> pinger_;
};

class Pinger : public Actor {
public:
  Pinger(std::uint64_t rounds, std::shared_ptr<std::promise<void>> done)
      : rounds_(rounds), done_(std::move(done)) {
    samples_.reserve(rounds_);
  }

  void SetPonger(std::shared_ptr<Actor> p) { ponger_ = std::move(p); }

  const std::vector<std::uint64_t> &samples() const { return samples_; }

  void Receive(const std::shared_ptr<Message> &msg) override {
    auto now = std::chrono::steady_clock::now();

    if (dynamic_cast<Start *>(msg.get())) {
      ponger_->Send(std::make_shared<Ping>(now));
      return;
    }

    if (auto *p = dynamic_cast<Ping *>(msg.get())) {
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - p->t0)
                    .count();
      samples_.push_back(static_cast<std::uint64_t>(ns));
      if (samples_.size() < rounds_) {
        ponger_->Send(std::make_shared<Ping>(now));
      } else {
        done_->set_value();
      }
    }
  }

private:
  std::shared_ptr<Actor> ponger_;
  std::uint64_t rounds_;
  std::shared_ptr<std::promise<void>> done_;
  std::vector<std::uint64_t> samples_;
};

std::vector<std::uint64_t> pingpong(std::uint64_t rounds) {
  ActorSystem system;

  auto done = std::make_shared<std::promise<void>>();
  auto done_fut = done->get_future();

  auto pinger = system.spawn<Pinger>("pinger", rounds, done);
  auto ponger = system.spawn<Ponger>("ponger", pinger);
  pinger->SetPonger(ponger);

  pinger->Send(std::make_shared<Start>());
  done_fut.wait();

  return pinger->samples();
}

double pct(const std::vector<std::uint64_t> &s, double p) {
  std::size_t idx = std::min<std::size_t>(
      s.size() - 1, static_cast<std::size_t>(s.size() * p));
  return s[idx] / 1000.0; // ns -> us
}

int main() {
  pingpong(10000); // warmup

  auto samples = pingpong(500000);
  std::sort(samples.begin(), samples.end());

  double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
  double avg = sum / samples.size() / 1000.0; // ns -> us

  auto line = [](const char *name, double us) {
    std::cout << std::left << std::setw(20) << name << " : " << std::fixed
              << std::setprecision(3) << us << " us\n";
  };

  std::cout << "round-trip latency (ns source): " << samples.size()
            << " samples\n";
  line("min", samples.front() / 1000.0);
  line("avg", avg);
  line("p50", pct(samples, 0.50));
  line("p99", pct(samples, 0.99));
  line("max", samples.back() / 1000.0);
  std::cout << "\none-way ~= round-trip / 2\n";

  return 0;
}
