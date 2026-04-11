#pragma once

#include <atomic>
#include <memory>

#include "../coordination/coordinator.hpp"
#include "../queues/global/global_queue.hpp"
#include "../tasks/concept.hpp"
#include "../worker/worker.hpp"
#include "config/concept.hpp"
#include "config/config.hpp"

namespace wr {

template <task::Task TaskT, config::ExecutionConfig Config = config::DefaultConfig>
class WsExecutor {
  public:  // nested types:
    using WorkerType = Worker<TaskT, Config>;
    using TaskPtr = TaskT*;

  private:  // data members:
    size_t num_workers_;
    std::vector<std::unique_ptr<WorkerType>> workers_;
    std::vector<std::thread> threads_;

    coord::Coordinator coordinator_;
    queues::GlobalQueue<TaskT> global_queue_;

    std::atomic<bool> started_ = false;

  public:  // member functions:
    explicit WsExecutor(size_t workers_count);
    ~WsExecutor();

    WsExecutor(const WsExecutor&) = delete;
    WsExecutor(WsExecutor&&) = delete;
    WsExecutor& operator=(const WsExecutor&) = delete;
    WsExecutor& operator=(WsExecutor&&) = delete;

    void submit(TaskPtr task) noexcept;

    void stop();

    // getters (for worker):
    queues::GlobalQueue<TaskT>& global_queue() noexcept {
        return global_queue_;
    }

    coord::Coordinator& coordinator() noexcept {
        return coordinator_;
    }

  private:  // member functions:
    void setup_victims();
    void spawn_workers();
};

/* -------------------------------------------------------------------- */

template <task::Task TaskType, config::ExecutionConfig Config>
WsExecutor<TaskType, Config>::WsExecutor(size_t workers_count)
    : num_workers_(workers_count),
      coordinator_(workers_count) {
    for (size_t i = 0; i < num_workers_; ++i) {
        workers_.push_back(std::make_unique<WorkerType>(*this, i));
    }
    setup_victims();
    spawn_workers();
}

template <task::Task TaskType, config::ExecutionConfig Config>
WsExecutor<TaskType, Config>::~WsExecutor() {
    stop();
}

template <task::Task TaskType, config::ExecutionConfig Config>
void WsExecutor<TaskType, Config>::setup_victims() {
    for (size_t i = 0; i < num_workers_; ++i) {
        std::vector<typename WorkerType::StealHandle> victims;
        for (size_t j = 0; j < num_workers_; ++j) {
            if (i == j)
                continue;
            victims.push_back(workers_[j]->stealer());
        }
        workers_[i]->set_victims(std::move(victims));
    }
}

template <task::Task TaskType, config::ExecutionConfig Config>
void WsExecutor<TaskType, Config>::spawn_workers() {
    for (size_t i = 0; i < num_workers_; ++i) {
        threads_.emplace_back([this, i]() { workers_[i]->run(); });
    }
    started_.store(true);
}

template <task::Task TaskType, config::ExecutionConfig Config>
void WsExecutor<TaskType, Config>::submit(TaskPtr task) noexcept {
    auto* local = current_worker<TaskType, Config>;

    if (local != nullptr) {
        local->push(task);
    } else {
        global_queue_.push(task);
        coordinator_.notify_worker();
    }
}

template <task::Task TaskType, config::ExecutionConfig Config>
void WsExecutor<TaskType, Config>::stop() {
    bool expected = true;
    if (started_.compare_exchange_strong(expected, false)) {
        coordinator_.shutdown();

        for (auto& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
}

}  // namespace wr
