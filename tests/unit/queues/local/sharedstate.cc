#include "queues/local/shared_state.hpp"
#include <gtest/gtest.h>
#include <ntrusive/intrusive.hpp>

struct InnerTask : public IntrusiveListNode {
    int value;

    explicit InnerTask(int val)
        : value(val) {}

    void run() noexcept {
        std::cout << "...Hallo von der inneren aufgabe..." << std::endl;
    }
};

class SharedStateTest : public ::testing::Test {
  protected:
    static constexpr size_t kCapacity = 128;
    wr::queues::SharedState<InnerTask, kCapacity> shared_state;
};
