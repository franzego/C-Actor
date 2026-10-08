#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <variant>

// All concrete message types live here, in one closed set.
struct Ping {
  int n = 0;
};

struct Greet {
  std::string name;
};

struct Start {};

// The one message type: a variant of every concrete message. Stored by value,
// so delivering a message needs no heap allocation and no shared_ptr refcount.
using Message = std::variant<Ping, Greet, Start>;

// Helper to visit a variant with a set of lambdas.
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif // MESSAGE_HPP
