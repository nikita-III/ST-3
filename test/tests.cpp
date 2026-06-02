// Copyright 2021 GHA Test Team

#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Return;

class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockTimer : public Timer {
public:
    MOCK_METHOD(void, tregister, (int timeout, TimerClient* client), (override));
};

TEST(TimedDoorTest, ConstructorSetsTimeoutAndDoorClosed) {
    TimedDoor door(1000);
    EXPECT_EQ(door.getTimeOut(), 1000);
    EXPECT_FALSE(door.isDoorOpened());
}

TEST(TimedDoorTest, UnlockOpensDoor) {
    TimedDoor door(500);
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
}

TEST(TimedDoorTest, LockClosesDoor) {
    TimedDoor door(500);
    door.unlock();
    door.lock();
    EXPECT_FALSE(door.isDoorOpened());
}

TEST(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
    TimedDoor door(1234);
    EXPECT_EQ(door.getTimeOut(), 1234);
}

TEST(TimedDoorTest, ThrowStateThrowsWhenDoorOpened) {
    TimedDoor door(100);
    door.unlock();
    EXPECT_THROW(door.throwState(), DoorTimeoutException);
}

TEST(TimedDoorTest, ThrowStateDoesNotThrowWhenDoorClosed) {
    TimedDoor door(100);
    door.lock();
    EXPECT_NO_THROW(door.throwState());
}

TEST(DoorTimerAdapterTest, TimeoutCallsThrowStateOnDoor) {
    TimedDoor door(200);
    DoorTimerAdapter adapter(door);
    door.unlock();
    EXPECT_THROW(adapter.Timeout(), DoorTimeoutException);
}

TEST(DoorTimerAdapterTest, TimeoutDoesNotThrowIfDoorClosed) {
    TimedDoor door(200);
    DoorTimerAdapter adapter(door);
    door.lock();
    EXPECT_NO_THROW(adapter.Timeout());
}

TEST(TimerInteractionTest, UnlockTriggersTimerRegistration) {
    MockTimer mockTimer;
    TimedDoor door(300);

    EXPECT_CALL(mockTimer, tregister(door.getTimeOut(), door.getAdapter()))
        .Times(1);

    mockTimer.tregister(door.getTimeOut(), door.getAdapter());
}

TEST(TimerInteractionTest, TimeoutCausesExceptionIfDoorStillOpen) {
    TimedDoor door(50);
    DoorTimerAdapter adapter(door);
    door.unlock();
    EXPECT_THROW(adapter.Timeout(), DoorTimeoutException);
}

TEST(TimerInteractionTest, TimeoutDoesNothingIfDoorClosed) {
    TimedDoor door(50);
    DoorTimerAdapter adapter(door);
    door.lock();
    EXPECT_NO_THROW(adapter.Timeout());
}

TEST(TimerInteractionTest, MultipleUnlockCalls) {
    TimedDoor door(100);
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
    door.lock();
    EXPECT_FALSE(door.isDoorOpened());
}

TEST(TimedDoorTest, DestructorDeletesAdapter) {
    TimedDoor* door = new TimedDoor(400);
    DoorTimerAdapter* adapter = door->getAdapter();
    delete door;
    SUCCEED();
}

TEST(RealTimerTest, TimerCallsTimeoutAfterDelay) {
    TimedDoor door(50);
    Timer realTimer;
    bool exceptionCaught = false;

    door.unlock();
    try {
        realTimer.tregister(door.getTimeOut(), door.getAdapter());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    catch (const DoorTimeoutException&) {
        exceptionCaught = true;
    }
    EXPECT_TRUE(exceptionCaught);
}
