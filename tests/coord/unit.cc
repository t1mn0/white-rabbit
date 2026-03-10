#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "../../src/coordination/coordinator.hpp"

using namespace std::chrono_literals;

class CoordinatorTest : public ::testing::Test {
  protected:
    void wait_a_bit() {
        std::this_thread::sleep_for(20ms);
    }
};

TEST_F(CoordinatorTest, InitializationLimits) {
    wr::coord::Coordinator coord(4);

    auto dir1 = coord.ask_to_steal();
    auto dir2 = coord.ask_to_steal();
    auto dir3 = coord.ask_to_steal();

    EXPECT_TRUE(dir1.is_should_steal());
    EXPECT_TRUE(dir2.is_should_steal());
    EXPECT_TRUE(dir3.is_should_park());
}

TEST_F(CoordinatorTest, InitializationSingleWorker) {
    wr::coord::Coordinator coord(1);

    auto dir1 = coord.ask_to_steal();
    auto dir2 = coord.ask_to_steal();

    EXPECT_TRUE(dir1.is_should_steal());
    EXPECT_TRUE(dir2.is_should_park());
}

TEST_F(CoordinatorTest, PermitRAIIRelease) {
    wr::coord::Coordinator coord(2);

    {
        auto dir1 = coord.ask_to_steal();
        EXPECT_TRUE(dir1.is_should_steal());

        auto dir2 = coord.ask_to_steal();
        EXPECT_TRUE(dir2.is_should_park());
    }

    auto dir3 = coord.ask_to_steal();
    EXPECT_TRUE(dir3.is_should_steal());
}

TEST_F(CoordinatorTest, TwoPhaseParkingRetryLogic) {
    wr::coord::Coordinator coord(2);

    auto dir1 = coord.ask_to_steal();
    EXPECT_TRUE(dir1.is_should_steal());

    coord.notify_worker();

    auto dir2 = coord.ask_to_steal();
    EXPECT_TRUE(dir2.is_should_retry());

    auto dir3 = coord.ask_to_steal();
    EXPECT_TRUE(dir3.is_should_park());
}

TEST_F(CoordinatorTest, ThreadParkingAndWakeup) {
    wr::coord::Coordinator coord(2);

    std::atomic<bool> thread_woke_up = false;

    std::thread worker([&]() {
        coord.park_worker();
        thread_woke_up = true;
    });

    wait_a_bit();
    EXPECT_FALSE(thread_woke_up.load());

    coord.notify_worker();

    worker.join();
    EXPECT_TRUE(thread_woke_up.load());
}

TEST_F(CoordinatorTest, ShutdownLogic) {
    wr::coord::Coordinator coord(4);

    EXPECT_FALSE(coord.is_should_shutdown());

    coord.shutdown();

    EXPECT_TRUE(coord.is_should_shutdown());

    auto dir = coord.ask_to_steal();
    EXPECT_TRUE(dir.is_should_terminate());
}

TEST_F(CoordinatorTest, ShutdownWakesUpAllParkedThreads) {
    wr::coord::Coordinator coord(4);
    const int num_threads = 3;
    std::atomic<int> awoken_count = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            coord.park_worker();
            awoken_count++;
        });
    }

    wait_a_bit();
    EXPECT_EQ(awoken_count.load(), 0);

    coord.shutdown();

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(awoken_count.load(), num_threads);
}

TEST_F(CoordinatorTest, UnwrapPermitWorks) {
    wr::coord::Coordinator coord(2);

    auto dir = coord.ask_to_steal();
    EXPECT_TRUE(dir.is_should_steal());

    auto permit = std::move(dir).unwrap_permit();

    auto dir_fail = coord.ask_to_steal();
    EXPECT_TRUE(dir_fail.is_should_park());
}
