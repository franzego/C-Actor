#ifndef MESSAGE_HPP
#define MESSAGE_HPP

// Base type for all messages in the system. Concrete message types derive
// from this; actors receive a shared_ptr<Message> and down-cast to the
// specific type they understand.
struct Message {
  virtual ~Message() = default;
};

#endif // MESSAGE_HPP
