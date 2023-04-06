
#include "trigger_timer.h"
#include <functional>
#include <utility>
#include <thread>
#include "spdlog/spdlog.h"

namespace trigger_timer {


/** TimerBase **/

TimerBase::TimerBase(int timeout, std::function<void()> callback)
    : is_running_(false), timeout_(timeout), callback_(std::move(callback)) {

}

TimerBase::TimerBase()
    : is_running_(false), timeout_(0), callback_(nullptr) {

}

void TimerBase::Reset(int timeout) {
    std::unique_lock<std::mutex> lock(mu_);
    if (!is_running_) { // 超时callback已经触发了, 再来reset已经晚了
        return;
    }
    if (timeout > 0) { // == -1 表示超时时间不变
        timeout_ = timeout;
    }
    start_ = Clock::now();
    cv_.notify_one();
//        spdlog::debug("Timer Reset. ");
}

void TimerBase::SetUpCallback(std::function<void()> callback) {
    std::unique_lock<std::mutex> lock(mu_);
    callback_ = std::move(callback);
}
void TimerBase::SetUpTimeout(int timeout) {
    std::unique_lock<std::mutex> lock(mu_);
    timeout_ = timeout;
}

void TimerBase::Stop() {
    std::unique_lock<std::mutex> lock(mu_);
    is_running_ = false;
    cv_.notify_one();
}


bool TimerBase::HasCallback() {
    std::unique_lock<std::mutex> lock(mu_);
    return callback_ == nullptr;
}


/** TriggerTimer **/


TriggerTimer::TriggerTimer(): TimerBase() { }

TriggerTimer::TriggerTimer(int timeout, std::function<void()> callback)
    : TimerBase(timeout, callback) {

}

TriggerTimer::~TriggerTimer() {
    if (running_thread_.joinable()) {
        running_thread_.join();
    }
}


void TriggerTimer::Start() {
    assert ( timeout_ > 0 );
    assert ( callback_ != nullptr );

    Stop(); // 如果正在倒计时别的任务, 就先停止

    std::unique_lock<std::mutex> lock(mu_);
    is_running_ = true;
    start_ = Clock::now();
    if (running_thread_.joinable()) {
        running_thread_.detach();
    }
    running_thread_ = std::thread(&TriggerTimer::_run, this);
}


void TriggerTimer::_run() {
    std::unique_lock<std::mutex> lock(mu_);
    while (is_running_) {
        cv_.wait_for(lock, std::chrono::milliseconds(timeout_));
        if (!is_running_) { // 如果被外部调用Stop就直接退出
//            spdlog::debug("Outer call `Stop`");
            break;
        }
        if (Clock::now() - start_ >= std::chrono::milliseconds(timeout_)) {
            if (callback_) {
                is_running_ = false;
                lock.unlock();
                callback_();
                return;
            }
        } // else: 外部调用reset则继续重新计时
    }
}


/** CycleTimer **/


CycleTimer::CycleTimer(): TimerBase() { }

CycleTimer::CycleTimer(int timeout, std::function<void()> callback)
    : TimerBase(timeout, callback) {

}


CycleTimer::~CycleTimer() {
//    spdlog::debug("here");
    if (running_thread_.joinable()) {
//        spdlog::debug("join");
        running_thread_.join();
    }
}


void CycleTimer::Start() {
    assert ( timeout_ > 0 );
    assert ( callback_ != nullptr );

    Stop(); // 如果正在倒计时别的任务, 就先停止

    std::unique_lock<std::mutex> lock(mu_);
    is_running_ = true;
    start_ = Clock::now();
    if (running_thread_.joinable()) {
        running_thread_.detach();
    }
    running_thread_ = std::thread(&CycleTimer::_run, this);
}


void CycleTimer::_run() {
    std::unique_lock<std::mutex> lock(mu_);
    while (is_running_) {
        cv_.wait_for(lock, std::chrono::milliseconds(timeout_));
        if (!is_running_) {
            break;
        }
        if (Clock::now() - start_ >= std::chrono::milliseconds(timeout_)) {
            if (callback_) {
                lock.unlock();
                callback_();
                lock.lock();
            }
            continue;  // 循环计时
        }
    }
}


}