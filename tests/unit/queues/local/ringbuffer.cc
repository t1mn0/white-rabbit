#include "queues/local/ring_buffer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <ntrusive/intrusive.hpp>
#include <numbers>

struct InnerTask : public IntrusiveListNode {
    int value;

    explicit InnerTask(int val)
        : value(val) {}

    void run() noexcept {
        std::cout << "...Hallo von der inneren aufgabe..." << std::endl;
    }
};

class RingBufferTests : public ::testing::Test {
  protected:
    static constexpr size_t kCapacity = 128;

    wr::queues::RingBuffer<InnerTask, kCapacity> rbuff;
    IntrusiveList<InnerTask> storage;

    void allocate_tasks() {
        for (size_t i = 0; i < kCapacity; ++i) {
            storage.push_back(*(new InnerTask(kCapacity * std::numbers::pi)));
        }
    }
};

TEST_F(RingBufferTests, CapacityCheck) {
    EXPECT_EQ(rbuff.capacity(), kCapacity);
}

TEST_F(RingBufferTests, ToLocalIndexMapping) {
    EXPECT_EQ(rbuff.to_local_index(0), 0);
    EXPECT_EQ(rbuff.to_local_index(1), 1);
    EXPECT_EQ(rbuff.to_local_index(127), 127);
    EXPECT_EQ(rbuff.to_local_index(128), 0);
    EXPECT_EQ(rbuff.to_local_index(257), 1);
}

TEST_F(RingBufferTests, StoreAndLoad) {
    allocate_tasks();

    rbuff.store(127, &storage.front());
    rbuff.store(129, &storage.back());

    EXPECT_EQ(rbuff.load(127), &storage.front());
    EXPECT_EQ(rbuff.load(1), &storage.back());
}

TEST_F(RingBufferTests, OverwriteElements) {
    allocate_tasks();

    auto* task1 = &storage.front();
    auto* task2 = &storage.back();

    rbuff.store(0, task1);
    EXPECT_EQ(rbuff.load(0), task1);

    rbuff.store(0, task2);
    EXPECT_EQ(rbuff.load(0), task2);
}
