
#include "trigger_timer.h"
#include <functional>
#include <utility>
#include <thread>
#include "spdlog/spdlog.h"

namespace trigger_timer {

TriggerTimer::TriggerTimer(int timeout, std::function<void()> callback)
    : is_running_(false), timeout_(timeout), callback_(std::move(callback)) {

}

TriggerTimer::~TriggerTimer() {
    Stop();
}

void TriggerTimer::SetUpTimeout(int timeout) {
    timeout_ = timeout;
}

void TriggerTimer::SetUpCallback(std::function<void()> callback) {
    callback_ = std::move(callback);
}

void TriggerTimer::Start() {
    assert ( timeout_ > 0 );
    assert ( callback_ != nullptr );

    Stop(); // 如果正在倒计时别的任务, 就先停止

    std::unique_lock<std::mutex> lock(mu_);
    is_running_ = true;
    start_ = Clock::now();
//    pre_start_ = start_;
    std::thread count_down(&TriggerTimer::_run, this);
    count_down.detach();
}


void TriggerTimer::Stop() {
    std::unique_lock<std::mutex> lock(mu_);
    is_running_ = false;
    cv_.notify_one();
}


void TriggerTimer::Reset(int timeout) {
    std::unique_lock<std::mutex> lock(mu_);
    if (!is_running_) { // 超时callback已经触发了, 再来reset已经晚了
        return;
    }
    if (timeout > 0) { // == -1 表示超时时间不变
        timeout_ = timeout;
    }
//    pre_start_ = start_;
    start_ = Clock::now();

    cv_.notify_one();
    spdlog::debug("TriggerTimer Reset!");
}

void TriggerTimer::_run() {
    std::unique_lock<std::mutex> lock(mu_);
    while (is_running_) {
        cv_.wait_for(lock, std::chrono::milliseconds(timeout_));
        if (!is_running_) { // 如果被外部调用Stop就直接退出
            spdlog::debug("Outer call `Stop`");
            break;
        }
//        auto elapsed = Clock::now() - pre_start_;
//        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
//        spdlog::debug("current interval: {} ms",elapsed_ms);
        if (Clock::now() - start_ >= std::chrono::milliseconds(timeout_)) {
            callback_();
            break; // 超时之后就不用继续计时了
        } // else: 外部调用reset则继续重新计时
    }
}


}