#pragma once

#include <chrono>
#include <mutex>
#include "spdlog/spdlog.h"

namespace trigger_timer {

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::steady_clock::time_point;

class TriggerTimer {
public:

    TriggerTimer(int timeout, std::function<void()> callback);

    template <typename T>
    TriggerTimer(int timeout, void (T::*callback)(), T* object)
        : is_running_(false), timeout_(timeout) {
        callback_ = std::bind(callback, object);
    }

    ~TriggerTimer();

    void SetUpTimeout(int timeout);
    void SetUpCallback(std::function<void()> callback);

    template <typename T>
    void SetUpCallback(void (T::*callback)(), T* object) {
        callback_ = std::bind(callback, object);
    }

    void Start();
    void Reset(int timeout=-1);      // Reset不会停止, 重置倒计时之后接着倒计时
    void Stop();                  // Stop会停止



private:

    std::thread running_thread_;
    bool is_running_;
    TimePoint start_;
//    TimePoint pre_start_;  // 用来调试
    int timeout_;
    std::function<void()> callback_{};
    std::mutex mu_{};
    std::condition_variable cv_{};

    void _run();


};


}