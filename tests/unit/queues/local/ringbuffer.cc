#include "queues/local/ring_buffer.hpp"
#include "queues/local/shared_state.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <ntrusive/intrusive.hpp>
#include <numbers>

static constexpr size_t kCapacity = 128;

struct InnerTask : public IntrusiveListNode {
    int value;

    explicit InnerTask(int val)
        : value(val) {}

    void run() noexcept {
        std::cout << "...Hallo von der inneren aufgabe..." << "\n";
    }
};

/* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */

class RingBufferTests : public ::testing::Test {
  protected:
    wr::queues::RingBuffer<InnerTask, kCapacity> rbuff;
    IntrusiveList<InnerTask> storage;

    void allocate_tasks() {
        for (size_t i = 0; i < kCapacity; ++i) {
            storage.push_back(*(new InnerTask(kCapacity * std::numbers::pi)));
        }
    }
};

/* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */

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

/* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */

class SharedStateTests : public ::testing::Test {
  protected:
    wr::queues::SharedState<InnerTask, kCapacity> state;
};

TEST_F(SharedStateTests, EmptyStateInvariant) {
    auto top = state.load_top_idx();
    auto bottom = state.load_bottom_idx();

    EXPECT_EQ(top, 0);
    EXPECT_EQ(bottom, 0);
}

TEST_F(SharedStateTests, CasManipulation) {
    auto incr = state.try_increment_top(0);
    EXPECT_TRUE(incr);

    EXPECT_EQ(state.load_top_idx(), 1);

    /* +---+---+---+---+---+---+---+---+ */

    auto incr2 = state.try_increment_top(0);
    EXPECT_FALSE(incr2);

    EXPECT_EQ(state.load_top_idx(), 1);
}

TEST_F(SharedStateTests, CasManipulationII) {
    state.store_bottom_idx(5);
    auto incr = state.try_increment_top_by(0, 5);

    EXPECT_TRUE(incr);
    EXPECT_EQ(state.load_top_idx(), 5);
}

TEST_F(SharedStateTests, StoreAndLoad) {
    state.store_task(22, new InnerTask(22));

    auto twtw = state.load_task(22);

    EXPECT_EQ(twtw->value, 22);
}

TEST_F(SharedStateTests, LoadFromEmptyState) {
    EXPECT_EQ(state.load_task(0), nullptr);
}

TEST_F(SharedStateTests, MappingCheck) {
    state.store_task(kCapacity, new InnerTask(kCapacity));

    EXPECT_EQ(state.load_task(0), state.load_task(kCapacity));
}

TEST_F(SharedStateTests, HugeIndices) {
    state.store_task(1'000'000, new InnerTask(1'000'000));

    EXPECT_EQ(state.load_task(1'000'000), state.load_task(1'000'000 % kCapacity));
}

TEST_F(SharedStateTests, BadIncrement) {
    state.store_bottom_idx(2);

    EXPECT_FALSE(state.try_increment_top_by(2, 5));
}
