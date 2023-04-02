#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "trigger_timer.h"

#include <mutex>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

using namespace trigger_timer;



TEST(TimerTest, TEST1) {

    spdlog::set_level(spdlog::level::debug);

    auto callback = [](){
        spdlog::debug("timeout!");
    };

    TriggerTimer trig_timer(5000, callback);
    trig_timer.Start();  // 开始倒计时

    std::string cmd;

    std::cout << "Input: ";
    while (std::cin >> cmd) {
        if (cmd == "reset") {
            trig_timer.Reset(); // 事件触发, 重置计时器, 并且支持修改超时时间
        } else if (cmd == "stop") {
            trig_timer.Stop();
        }
        std::cout << "Input: ";
    }


}
