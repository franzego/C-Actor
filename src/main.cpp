#include "actorsystem.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

std::mutex io;
template <typename... Args> void log(Args &&...args) {
  std::lock_guard<std::mutex> lock(io);
  (std::cout << ... << args);
  std::cout << "\n";
}

class Printer : public Actor {
public:
  void Receive(const Message &msg) override {
    std::visit(overloaded{
                   [](const Greet &g) { log("[printer] hello ", g.name, "!"); },
                   [](const auto &) {},
               },
               msg);
  }
};

class Greeter : public Actor {
public:
  explicit Greeter(std::shared_ptr<Actor> printer)
      : printer_(std::move(printer)) {}

  void Receive(const Message &msg) override {
    if (std::holds_alternative<Greet>(msg)) {
      log("[greeter] got Greet, forwarding...");
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

    system.get("greeter")->Send(Greet{"Alice"});
    system.get("greeter")->Send(Greet{"Bob"});

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  log("done");
  return 0;
}
