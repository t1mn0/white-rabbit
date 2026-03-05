#include "queues/local/ring_buffer.hpp"
#include "vvv/list.hpp"
#include <gtest/gtest.h>

struct InnerTask : public vvv::IntrusiveListNode<InnerTask> {
    int value;

    void run() noexcept {
        std::cout << "Hello from task..." << std::endl;
    }
};

class RingBufferTests : public ::testing::Test {
  protected:
    wr::queues::RingBuffer<InnerTask, 128> rbuff;
};

TEST_F(RingBufferTests, CapacityCheck) {
    EXPECT_EQ(rbuff.capacity(), 128);
}
