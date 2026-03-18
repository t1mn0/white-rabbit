#include "queues/local/ring_buffer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <ntrusive/intrusive.hpp>

struct InnerTask : public IntrusiveListNode {
    int value;

    void run() noexcept {
        std::cout << "...Hallo von der inneren aufgabe..." << std::endl;
    }
};

class RingBufferTests : public ::testing::Test {
  protected:
    static constexpr size_t kCapacity = 128;

    wr::queues::RingBuffer<InnerTask, kCapacity> rbuff;
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
