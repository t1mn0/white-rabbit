#pragma once

#include "../tasks/concept.hpp"
#include "../worker/worker.hpp"
#include "config.hpp"
#include <atomic>
#include <deque>
#include <memory>

namespace wr {

template <task::Task TaskType, config::ExecutionConfig Config = config::DefaultConfig>
class WsExecutor {
  public:
    WsExecutor(size_t workers_count);
    ~WsExecutor();

    WsExecutor(const WsExecutor&) = delete;
    WsExecutor(WsExecutor&&) = delete;
    WsExecutor& operator=(const WsExecutor&) = delete;
    WsExecutor& operator=(WsExecutor&&) = delete;

    using WorkerType = Worker<TaskType, Config>;


    void submit(TaskType* task) noexcept;

  private:
    std::vector<std::unique_ptr<WorkerType>> workers_;
    std::atomic<int> active_stealers_{0};
    size_t max_stealers_;
    size_t num_workers_;
};


}  // namespace wr
