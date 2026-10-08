#include <gtest/gtest.h>

#include "actorsystem.hpp"
#include "mailbox.hpp"
#include "threadsafe_queue.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>

TEST(ThreadSafeQueueTest, PopReturnsItemsInFifoOrder) {
  ThreadSafeQueue<int> q;
  q.Push(1);
  q.Push(2);
  q.Push(3);

  int v = 0;
  ASSERT_TRUE(q.Pop(v));
  EXPECT_EQ(v, 1);
  ASSERT_TRUE(q.Pop(v));
  EXPECT_EQ(v, 2);
  ASSERT_TRUE(q.Pop(v));
  EXPECT_EQ(v, 3);
}

TEST(ThreadSafeQueueTest, PopReturnsFalseAfterShutdown) {
  ThreadSafeQueue<int> q;
  q.Shutdown();
  int v = 0;
  EXPECT_FALSE(q.Pop(v));
}

TEST(MailboxTest, TryPopReturnsMessagesInFifoOrder) {
  Mailbox mb;
  mb.Push(Ping{1});
  mb.Push(Ping{2});

  Message m;
  ASSERT_TRUE(mb.TryPop(m));
  EXPECT_EQ(std::get<Ping>(m).n, 1);
  ASSERT_TRUE(mb.TryPop(m));
  EXPECT_EQ(std::get<Ping>(m).n, 2);
  EXPECT_FALSE(mb.TryPop(m));
}

TEST(MailboxTest, SchedulesOnlyOnEmptyToNonEmptyTransition) {
  Mailbox mb;
  int calls = 0;
  mb.SetScheduleCallback([&calls] { ++calls; });

  mb.Push(Ping{1});
  EXPECT_EQ(calls, 1);

  mb.Push(Ping{2});
  EXPECT_EQ(calls, 1);

  Message m;
  mb.TryPop(m);
  mb.TryPop(m);
  mb.TryPop(m);

  mb.Push(Ping{3});
  EXPECT_EQ(calls, 2);
}

class Counter : public Actor {
public:
  void Receive(const Message &msg) override {
    if (std::holds_alternative<Ping>(msg)) {
      count_ += std::get<Ping>(msg).n;
    }
  }
  int total() const { return count_.load(); }

private:
  std::atomic<int> count_{0};
};

TEST(ActorSystemTest, SpawnedActorReceivesMessages) {
  ActorSystem system;
  auto counter = system.spawn<Counter>("counter");

  constexpr int kMsgs = 100;
  for (int i = 0; i < kMsgs; ++i) {
    counter->Send(Ping{1});
  }

  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (counter->total() < kMsgs && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  EXPECT_EQ(counter->total(), kMsgs);
}

TEST(ActorSystemTest, DuplicateNameThrows) {
  ActorSystem system;
  system.spawn<Counter>("dup");
  EXPECT_THROW(system.spawn<Counter>("dup"), std::invalid_argument);
}
