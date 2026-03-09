#include <atomic>
#include <chrono>
#include <iostream>
#include <vector>

#include "../src/exec/executor.hpp"

struct ComputeTask : public vvv::IntrusiveListNode<ComputeTask> {
    std::atomic<size_t>& counter;

    explicit ComputeTask(std::atomic<size_t>& external_counter)
        : counter(external_counter) {}

    void run() noexcept {
        counter.fetch_add(1);
    }
};

int main() {
    const size_t kNumWorkers = 4;
    const size_t kNumTasks = 100'000;

    std::atomic<size_t> completed_tasks{0};

    std::cout << "[WhiteRabbit] Starting executor with " << kNumWorkers << " workers..." << std::endl;

    {
        wr::WsExecutor<ComputeTask, wr::config::TinyConfig> executor{kNumWorkers};
        std::vector<ComputeTask> tasks;
        tasks.reserve(kNumTasks);
        for (size_t i = 0; i < kNumTasks; ++i) {
            tasks.emplace_back(completed_tasks);
        }

        std::cout << "[WhiteRabbit] Submitting " << kNumTasks << " tasks..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < kNumTasks; ++i) {
            executor.submit(&tasks[i]);
        }
        std::cout << "[WhiteRabbit] Waiting for completion..." << std::endl;
        executor.stop();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "[WhiteRabbit] Done!" << std::endl;
        std::cout << "Time: " << duration.count() << " ms" << std::endl;
    }

    std::cout << "Tasks expected:  " << kNumTasks << std::endl;
    std::cout << "Tasks completed: " << completed_tasks.load() << std::endl;

    if (completed_tasks.load() == kNumTasks) {
        std::cout << "SUCCESS: All tasks executed correctly." << std::endl;
        return 0;
    } else {
        std::cerr << "FAILURE: Task count mismatch!" << std::endl;
        return 1;
    }
}
