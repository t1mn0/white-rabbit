#include <iostream>
#include <math.h>
#include "queues/local/ws_queue.hpp"
#include <gtest/gtest.h>


static constexpr size_t kCapacity = 128;

struct InnerTask : public IntrusiveListNode {
    int value;

    explicit InnerTask(int val)
        : value(val) {}

    void run() noexcept {
        std::cout << "...Hallo von der inneren aufgabe..." << "\n";
    }
};

class WsQueueTests : public ::testing::Test {
  protected:
    wr::queues::WorkStealingQueue<InnerTask, kCapacity> ws_queue;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

TEST_F(WsQueueTests, EmptyPop) {
    EXPECT_FALSE(ws_queue.try_pop().has_value());
}

TEST_F(WsQueueTests, PushAndPop) {
    auto* task = new InnerTask(22);

    ws_queue.try_push(task);
    auto popped_task = ws_queue.try_pop();

    EXPECT_EQ(task, popped_task);
}

TEST_F(WsQueueTests, LifoOrder) {
    auto* task1 = new InnerTask(11);
    auto* task2 = new InnerTask(22);


    ws_queue.try_push(task1);
    ws_queue.try_push(task2);

    auto popped_task1 = ws_queue.try_pop();
    auto popped_task2 = ws_queue.try_pop();

    EXPECT_EQ(popped_task1, task2);
    EXPECT_EQ(popped_task2, task1);
}

TEST_F(WsQueueTests, Offloading) {
    for (size_t i = 0; i < kCapacity; ++i) {
        ws_queue.try_push(new InnerTask(i));
    }

    auto* new_push = new InnerTask(22);
    if (not ws_queue.try_push(new_push)) {
        ws_queue.offload_half();
        ws_queue.try_push(new_push);
    }

    auto size = ws_queue.inner_state().load_bottom_idx() - ws_queue.inner_state().load_top_idx();
    EXPECT_EQ(size, 65);
}
