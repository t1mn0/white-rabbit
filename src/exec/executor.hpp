#pragma once

#include "../tasks/concept.hpp"
#include "../worker/worker.hpp"
#include "config/concept.hpp"
#include "config/config.hpp"

#include <atomic>
#include <memory>

namespace wr {

template <task::Task TaskType, config::ExecutionConfig Config = config::DefaultConfig>
class WsExecutor {
  public:
    using WorkerType = worker::Worker<TaskType, Config>;

  private:
    std::vector<std::unique_ptr<WorkerType>> workers_;
    std::atomic<int> active_stealers_{0};
    size_t max_stealers_;
    size_t num_workers_;

  public:
    WsExecutor(size_t workers_count);
    ~WsExecutor();

    WsExecutor(const WsExecutor&) = delete;
    WsExecutor(WsExecutor&&) = delete;
    WsExecutor& operator=(const WsExecutor&) = delete;
    WsExecutor& operator=(WsExecutor&&) = delete;

    void submit(TaskType* task) noexcept;
};


}  // namespace wr
