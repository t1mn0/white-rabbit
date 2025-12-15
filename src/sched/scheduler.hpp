#pragma once

#include "../queues/global/global_queue.hpp"
#include "../tasks/task.hpp"
#include "../tasks/task_container.hpp"
#include "worker.hpp"
#include <memory>
#include <random>
#include <thread>
#include <vector>

class Worker;

namespace wr {

class Scheduler {
  public:
    static const uint32_t DEFAULT_WORKERS;
    static constexpr size_t POOL_CAPACITY = 8192;

    Scheduler(uint32_t worker_count /* = DEFAULT_WORKERS*/);
    Scheduler(const Scheduler&) = delete;             // non copyable;
    Scheduler& operator=(const Scheduler&) = delete;  // non copyassignable;
    Scheduler(Scheduler&&) = delete;                  // non movable;
    Scheduler& operator=(Scheduler&&) = delete;       // non moveassignable
    ~Scheduler();

    void submit(TaskBase*);
    void submit_task_container(ITaskContainer&&);

  private:
    uint32_t worker_count_;
    std::vector<std::unique_ptr<Worker>> workers_;
    std::vector<std::thread> threads_;

    std::random_device rd_;
    std::mt19937 rng_;
};

}  // namespace wr
