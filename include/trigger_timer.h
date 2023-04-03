#pragma once

#include <chrono>
#include <mutex>
#include <utility>
#include "spdlog/spdlog.h"

namespace trigger_timer {

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::steady_clock::time_point;



class TimerBase {
protected:
    virtual void _run() = 0;
public:
    virtual ~TimerBase() = default;;
    virtual void Start() = 0;

    TimerBase(int timeout, std::function<void()> callback);
    TimerBase();

    void Reset(int timeout=-1);
    void Stop();
    void SetUpCallback(std::function<void()> callback);
    void SetUpTimeout(int timeout);


protected:
    std::thread running_thread_;
    bool is_running_;
    TimePoint start_;
    int timeout_;
    std::function<void()> callback_{};
    std::mutex mu_{};
    std::condition_variable cv_{};
};



class TriggerTimer: public TimerBase {
public:

    TriggerTimer();
    TriggerTimer(int timeout, std::function<void()> callback);
    ~TriggerTimer() override;

    void Start() override;

protected:
    void _run() override;
};


class CycleTimer: public TimerBase {
public:
    CycleTimer();
    CycleTimer(int timeout, std::function<void()> callback);
    ~CycleTimer() override;

    void Start() override;
    void Reset(int timeout=-1) { }  // 禁止CycleTimer从外面重置计时器

protected:
    void _run() override;
};


}