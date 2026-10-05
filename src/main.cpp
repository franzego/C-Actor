#include "actorsystem.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

std::mutex mu;
template <typename... Args> void log(Args &&...args) {
  std::lock_guard<std::mutex> lock(mu);
  (std::cout << ... << args);
  std::cout << "\n";
}

struct Greet : Message {
  std::string name;
  explicit Greet(std::string s) : name(std::move(s)) {}
};

class Printer : public Actor {
public:
  void Receive(const std::shared_ptr<Message> &msg) override {
    if (auto *g = dynamic_cast<Greet *>(msg.get())) {
      log("[printer] hello ", g->name, "!");
    }
  }
};

class Greeter : public Actor {
public:
  explicit Greeter(std::shared_ptr<Actor> printer)
      : printer_(std::move(printer)) {}

  void Receive(const std::shared_ptr<Message> &msg) override {
    if (auto *g = dynamic_cast<Greet *>(msg.get())) {
      log("[greeter] got Greet(", g->name, "), forwarding...");
      printer_->Send(msg);
    }
  }

private:
  std::shared_ptr<Actor> printer_;
};

int main() {
  {
    ActorSystem system;

    auto printer = system.spawn<Printer>("printer");
    system.spawn<Greeter>("greeter", printer);

    system.get("greeter")->Send(std::make_shared<Greet>("Alice"));
    system.get("greeter")->Send(std::make_shared<Greet>("Bob"));

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  log("done");
  return 0;
}
