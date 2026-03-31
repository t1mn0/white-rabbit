#pragma once

#include <cstddef>
#include <random>
#include <vector>

#include <ntrusive/list.hpp>

#include "../exec/config/concept.hpp"
#include "../exec/config/config.hpp"
#include "../queues/local/ws_queue.hpp"

namespace wr {

/* forward-declaration */
template <task::Task TaskT, config::ExecutionConfig Config>
class WsExecutor;

template <task::Task TaskT, config::ExecutionConfig Config = config::DefaultConfig>
class Worker {
  public:
    /* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

    static constexpr size_t kCapacity = Config::kLocalQueueCapacity;
    static constexpr size_t kMaxLifoStreak = Config::kMaxLifoStreak;
    static constexpr size_t kFairnessPeriod = Config::kFairnessPeriod;

    using TaskPtr = TaskT*;
    using LocalQueue = queues::WorkStealingQueue<TaskT, kCapacity>;
    using StealHandle = queues::StealHandle<TaskT, kCapacity>;

  private:
    /* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

    /* Ownership relationship: Executor owns Workers.
     * Worker needs reference to the host to access global queue and coordinator */
    WsExecutor<TaskT, Config>& host_;
    const size_t index_;

    LocalQueue local_queue_;

    // tick_ adds a bit of fairness: periodically checks global queue to prevent starvation.
    // Why 61?? Dmitrii Viukov (Golang scheduler designer) said:
    // >> "It's not even a 42!"
    // >> "not too small"
    // >> "not too big"
    // >> "prime to break any patterns"
    uint64_t tick_ = 0;

    // LIFO slot is hottest slot for TaskPtr [thread-local]
    TaskPtr lifo_slot_ = nullptr;
    size_t lifo_streak_ = 0;

    std::mt19937_64 rng_;
    std::vector<StealHandle> victims_;

    std::atomic<bool> stop_flag_ = false;

    /* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

  public:  // member-functions:
    Worker(WsExecutor<TaskT, Config>& host, size_t worker_index);

    void run() noexcept;  // run-loop;
    void push(TaskT* /*, TODO: SchedHint param */) noexcept;

    WsExecutor<TaskT, Config>& host() const noexcept;

    [[nodiscard]] StealHandle stealer() noexcept;
    void set_victims(std::vector<StealHandle> victims) noexcept;

  private:  // member-functions:
    [[nodiscard]] TaskPtr pick_task() noexcept;

    [[nodiscard]] TaskPtr try_steal_any() noexcept;

    [[nodiscard]] TaskPtr try_pop_lifo() noexcept;
    [[nodiscard]] TaskPtr try_pop_local() noexcept;
    [[nodiscard]] TaskPtr try_pop_global() noexcept;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

template <task::Task T, config::ExecutionConfig C>
thread_local Worker<T, C>* current_worker = nullptr;

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

template <task::Task TaskT, config::ExecutionConfig Config>
Worker<TaskT, Config>::Worker(WsExecutor<TaskT, Config>& host, size_t index)
    : host_(host),
      index_(index),
      rng_(index) {}

template <task::Task TaskT, config::ExecutionConfig Config>
WsExecutor<TaskT, Config>& Worker<TaskT, Config>::host() const noexcept {
    return host_;
}

template <task::Task TaskType, config::ExecutionConfig Config>
auto Worker<TaskType, Config>::stealer() noexcept -> StealHandle {
    return local_queue_.create_stealer();
}

template <task::Task TaskType, config::ExecutionConfig Config>
void Worker<TaskType, Config>::set_victims(std::vector<StealHandle> victims) noexcept {
    victims_ = std::move(victims);
}

template <task::Task TaskT, config::ExecutionConfig Config>
void Worker<TaskT, Config>::push(TaskPtr task) noexcept {
    tick_++;

    // trying push to lifo slot (most warm cache):
    if (lifo_streak_ < kMaxLifoStreak) {
        if (lifo_slot_ == nullptr) {
            lifo_slot_ = task;
            lifo_streak_++;
            host_.coordinator().notify_worker();
            return;
        }

        // slot occupied: swap and push the old task to local queue:
        TaskPtr old = lifo_slot_;
        lifo_slot_ = task;
        task = old;
        lifo_streak_++;
    } else {
        lifo_streak_ = 0;
    }

    // if couldnt push to lifo, trying push to local queue:
    if (!local_queue_.try_push(task)) /* pushing to local queue failed => */ {
        auto batch = local_queue_.offload_half();
        if (batch) {
            host_.global_queue().push_batch(std::move(*batch));
        }
        local_queue_.try_push(task);
    }

    host_.coordinator().notify_worker();
}

// RUN LOOP:
template <task::Task TaskT, config::ExecutionConfig Config>
void Worker<TaskT, Config>::run() noexcept {
    current_worker<TaskT, Config> = this;

    while (true) {
        TaskPtr task = pick_task();
        if (task) {
            task->run();
        } else {
            // pick_task return nullptr <=> coordinator signal Termination => stop accepting tasks;
            break;
        }
    }
}

template <task::Task TaskT, config::ExecutionConfig Config>
auto Worker<TaskT, Config>::pick_task() noexcept -> TaskPtr {
    while (true) {
        if (tick_ % kFairnessPeriod == 0) {
            if (TaskPtr task = try_pop_global())
                return task;
        }

        if (TaskPtr task = try_pop_lifo()) {
            return task;
        } else if (TaskPtr task = try_pop_local()) {
            return task;
        } else if (TaskPtr task = try_pop_global()) {
            return task;
        }

        auto directive = host_.coordinator().ask_to_steal();

        if (directive.should_terminate()) {
            return nullptr;
        } else if (directive.should_steal()) {
            if (TaskPtr stolen_task = try_steal_any()) {
                return stolen_task;
            }
            continue;
        } else if (directive.should_retry()) {
            continue;
        }

        host_.coordinator().park_worker();
    }
}

template <task::Task TaskT, config::ExecutionConfig Config>
auto Worker<TaskT, Config>::try_pop_lifo() noexcept -> TaskPtr {
    TaskPtr task = lifo_slot_;
    lifo_slot_ = nullptr;
    return task;
}

template <task::Task TaskT, config::ExecutionConfig Config>
auto Worker<TaskT, Config>::try_pop_local() noexcept -> TaskPtr {
    return local_queue_.try_pop().value_or(nullptr);
}

template <task::Task TaskT, config::ExecutionConfig Config>
auto Worker<TaskT, Config>::try_pop_global() noexcept -> TaskPtr {
    return host_.global_queue().try_pop().value_or(nullptr);
}

template <task::Task TaskT, config::ExecutionConfig Config>
auto Worker<TaskT, Config>::try_steal_any() noexcept -> TaskPtr {
    if (victims_.empty()) {
        return nullptr;
    }

    std::uniform_int_distribution<size_t> dist(0, victims_.size() - 1);
    size_t start = dist(rng_);

    for (size_t i = 0; i < victims_.size(); ++i) {
        size_t target = (start + i) % victims_.size();
        auto loot = victims_[target].steal();

        if (loot.success()) {
            return std::move(loot).unwrap();
        }
    }
    return nullptr;
}

}  // namespace wr
