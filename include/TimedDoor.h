// Copyright 2021 GHA Test Team

#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

#include <stdexcept>

class DoorTimeoutException : public std::runtime_error {
 public:
    DoorTimeoutException() : std::runtime_error("Door open too long") {}
};

class TimerClient {
 public:
    virtual void Timeout() = 0;
    virtual ~TimerClient() = default;
};

class Door {
 public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool isDoorOpened() = 0;
    virtual ~Door() = default;
};

class TimedDoor;

class DoorTimerAdapter : public TimerClient {
 private:
    TimedDoor& door;
 public:
    explicit DoorTimerAdapter(TimedDoor& d);
    void Timeout() override;
};

class TimedDoor : public Door {
 private:
    DoorTimerAdapter* adapter;
    int iTimeout;
    bool isOpened;

 public:
    explicit TimedDoor(int timeout);
    ~TimedDoor();

    bool isDoorOpened() override;
    void unlock() override;
    void lock() override;
    int getTimeOut() const;
    void throwState();

    DoorTimerAdapter* getAdapter() const;
};

class Timer {
 public:
    virtual ~Timer() = default;
    virtual void tregister(int timeout, TimerClient* client);
};

#endif  // INCLUDE_TIMEDDOOR_H_
