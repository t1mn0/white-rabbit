#include <gtest/gtest.h>

#include "../../../src/queues/global/global_queue.hpp"

// -------------------- Test prerequisites --------------------

struct TestTask : vvv::IntrusiveListNode<TestTask> {
    int value;

    explicit TestTask(int v) : value(v) {}

    void run() noexcept { /* do nothing */ }
};

class GlobalQueueTest : public ::testing::Test {
  protected:
    wr::queues::GlobalQueue<TestTask> queue;
};

// -------------------- Tests --------------------

TEST_F(GlobalQueueTest, FIFO) {
    TestTask t1(1), t2(2);

    queue.push(&t1);
    queue.push(&t2);

    auto res1 = queue.try_pop();
    ASSERT_TRUE(res1.has_value());
    EXPECT_EQ((*res1)->value, 1);

    auto res2 = queue.try_pop();
    ASSERT_TRUE(res2.has_value());
    EXPECT_EQ((*res2)->value, 2);

    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST_F(GlobalQueueTest, PushEmptyBatch) {
    vvv::IntrusiveList<TestTask> empty_batch;
    queue.push_batch(std::move(empty_batch));
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST_F(GlobalQueueTest, PopBatchMoreThanAvailable) {
    TestTask t1(1), t2(2);
    queue.push(&t1);
    queue.push(&t2);

    auto res = queue.try_pop_batch(100);
    ASSERT_TRUE(res.has_value());

    int size = 0;
    while (res->TryPopFront()) {
        size++;
    }

    EXPECT_EQ(size, 2);
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST_F(GlobalQueueTest, PopBatchZero) {
    TestTask t1(1);
    queue.push(&t1);

    auto res = queue.try_pop_batch(0);
    EXPECT_FALSE(res.has_value());

    auto t = queue.try_pop();
    EXPECT_TRUE(t.has_value());
    EXPECT_EQ((*t)->value, 1);
}

TEST_F(GlobalQueueTest, BatchOperations) {
    vvv::IntrusiveList<TestTask> batch;
    TestTask tasks[5] = {TestTask(1), TestTask(2), TestTask(3), TestTask(4), TestTask(5)};

    for (auto& t : tasks) {
        batch.PushBack(&t);
    }

    queue.push_batch(std::move(batch));
    // now batch is empty, size(queue) = 5;

    auto popped_batch = queue.try_pop_batch(3);
    // now size(popped_batch) = 3, size(queue) = 2;
    ASSERT_TRUE(popped_batch.has_value());

    int count = 0;
    while (popped_batch->NonEmpty()) {
        auto* t = popped_batch->PopFrontNonEmpty();
        ++count;
        EXPECT_EQ(t->value, count);
    }
    EXPECT_EQ(count, 3);
    // now size(popped_batch) = 0, size(queue) = 2;

    int remains = 0;
    while (queue.try_pop()) {
        ++remains;
    }
    EXPECT_EQ(remains, 2);
    // now size(popped_batch) = 0, size(queue) = 0;
}
