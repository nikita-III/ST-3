// Copyright 2021 GHA Test Team

#include "TimedDoor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override)); //
};

class TimedDoorFixture : public ::testing::Test {
 protected:
  std::unique_ptr<TimedDoor> door;

  void SetUp() override {
    door = std::make_unique<TimedDoor>(10);
    door->lock();
  }

  void TearDown() override { door.reset(); }
};

TEST_F(TimedDoorFixture, test_closed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorFixture, test_lock_closed) {
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorFixture, not_throw_when_closed) {
  EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorFixture, throws_when_open) {
  try {
    door->unlock();
  } catch (const std::runtime_error &) {
  }
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST(TimedDoor, timeout_stored) {
  TimedDoor door(123);
  EXPECT_EQ(door.getTimeOut(), 123);
}

TEST(TimedDoor, throw_open_after_timeout) {
  TimedDoor door(0);
  door.lock();
  EXPECT_THROW(door.unlock(), std::runtime_error);
}

TEST(TimedDoor, not_throw_closed_before_timeout) {
  TimedDoor door(80);
  door.lock();

  auto worker = std::async(std::launch::async, [&door]() -> bool {
    try {
      door.unlock();
      return false;
    } catch (const std::runtime_error &) {
      return true;
    }
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  door.lock();

  EXPECT_FALSE(worker.get());
  EXPECT_FALSE(door.isDoorOpened());
}

TEST(TimedDoor, open_after_unlock_exp) {
  TimedDoor door(0);
  door.lock();
  try {
    door.unlock();
  } catch (const std::runtime_error &) {
  }
  EXPECT_TRUE(door.isDoorOpened());
}

TEST(Timer, reg_call_timeout) {
  Timer timer;
  MockTimerClient client;
  EXPECT_CALL(client, Timeout()).Times(1);
  timer.tregister(0, &client);
}

TEST(Timer, reg_null_client_not_throw) {
  Timer timer;
  EXPECT_NO_THROW(timer.tregister(0, nullptr));
}

TEST(DoorTimerAdapter, timoeout_throw_when_open) {
  TimedDoor door(0);
  door.lock();
  try {
    door.unlock();
  } catch (const std::runtime_error &) {
  }

  DoorTimerAdapter adapter(door);
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(Timer, reg_with_delay) {
  Timer timer;
  MockTimerClient client;
  EXPECT_CALL(client, Timeout()).Times(1);
  timer.tregister(2, &client);
}

TEST(DoorTimerAdapter, timeout_when_closed) {
  TimedDoor door(50);
  door.lock();
  DoorTimerAdapter adapter(door);

  try {
    door.unlock();
  } catch (const std::runtime_error &) {}

  door.lock();

  EXPECT_NO_THROW(adapter.Timeout());
}

TEST(DoorTimerAdapter, timoeout_not_throw_when_closed) {
  TimedDoor door(0);
  door.lock();
  DoorTimerAdapter adapter(door);
  EXPECT_NO_THROW(adapter.Timeout());
}
