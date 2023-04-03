#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "trigger_timer.h"

#include <mutex>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

using namespace trigger_timer;



TEST(TimerTest, ResetTest) {

    spdlog::set_level(spdlog::level::debug);

    auto callback = [](){
        spdlog::debug("timeout!");
    };

    TriggerTimer trig_timer(5000, callback);
    trig_timer.Start();  // 开始倒计时

    for (int i = 1; i <= 5; i ++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(i*1000-100));
        trig_timer.Reset();
    }
}

TEST(TimerTest, ObjectCallbackTest) {

    spdlog::set_level(spdlog::level::debug);

    class B {
    public:
        int value_ = 3;
        void callback() {
            value_ *= 2;
            spdlog::debug("B::timeout!");
            spdlog::debug("new value = {}", value_);
        }
    };

    B b;
    auto func = [&b] { b.callback(); };
    TriggerTimer trig_timer(5000, func);
    trig_timer.Start();  // 开始倒计时
}


class Node {
public:
    Node(): t_ptr(new TriggerTimer) {  };
    ~Node() { spdlog::debug("Dec"); t_ptr.reset(); }
    void callback() { spdlog::debug("Node::timeout. "); }

    std::shared_ptr<TriggerTimer> t_ptr;
};


TEST(TimerTest, ReStartTest) {

    spdlog::set_level(spdlog::level::debug);

    std::shared_ptr<Node> node(new Node);
    node->t_ptr->SetUpTimeout(5000);
    auto w_node = std::weak_ptr<Node>(node);
    auto func = [w_node] { w_node.lock()->callback(); };
    node->t_ptr->SetUpCallback(func);
    node->t_ptr->Start();
}

TEST(TimerTest, CycleTimerTest) {
    spdlog::set_level(spdlog::level::debug);
    auto callback = []{ spdlog::debug("Time Out. "); };
    CycleTimer timer(3000, callback);
    timer.Start();
    std::this_thread::sleep_for(std::chrono::seconds(13));
    timer.Stop();
}
