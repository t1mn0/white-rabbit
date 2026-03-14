#pragma once

#include "../exec/config/concept.hpp"
#include "../exec/config/config.hpp"
#include "../queues/local/ws_queue.hpp"

#include <cstddef>
#include <ntrusive/ntrusive.hpp>
#include <optional>
#include <random>
#include <vector>

namespace wr {

/* forward-declaration */
template <task::Task TaskType, config::ExecutionConfig Config>
class WsExecutor;

template <task::Task TaskType, config::ExecutionConfig Config = config::DefaultConfig>
class Worker {
  public:  // nested types:
    static constexpr size_t kCapacity = Config::kLocalQueueCapacity;
    static constexpr size_t kMaxLifoStreak = Config::kMaxLifoStreak;
    static constexpr size_t kFairnessPeriod = Config::kFairnessPeriod;

    using TaskPtr = TaskType*;
    using LocalQueue = queues::WorkStealingQueue<TaskType, kCapacity>;
    using StealHandle = queues::StealHandle<TaskType, kCapacity>;
    using LootType = queues::Loot<TaskType>;

  private:  // data members:
    // We introduce a ownership relationship: the executor owns the Worker objects,
    // the Worker belongs to the executor object.
    // This attitude is caused by the fact that the Worker is inextricably linked to
    // the scheduler that owns it and cannot execute its responsibility without this
    // "knowledge". On the contrary, for example, global queue may not know its
    // owner (Worker) and at the same time ideally execute its single responsibility;
    // This relationship also allows two of its participants to "look into" (=use)
    // each other's details.
    WsExecutor<TaskType, Config>& host_;
    const size_t worker_index_;

    // tick_ field adds a bit of fairness: the worker undertakes to load a task from
    // a global queue on every constantly certain (61 by default) iteration in order to improve
    // global progress guarantees and
    //
    // why 61?? Dmitrii Viukov (who designed and implemented the Golang scheduler) said:
    // >> "It's not even a 42!"
    // >> "not too small"
    // >> "not too big"
    // >> "prime to break any patterns"
    //
    uint64_t tick_ = 0;

    std::atomic<TaskType*> lifo_slot_ = nullptr;
    size_t lifo_streak_ = 0;

    LocalQueue local_queue_;

    std::mt19937_64 rng_;
    std::vector<StealHandle> victims_;

    std::atomic<bool> stop_flag_ = false;

  public:  // member-functions:
    Worker(WsExecutor<TaskType, Config>& host, size_t worker_index);

    void start();  // auto-join;
    void stop();   // auto-join;

    void push_task(TaskType* /*, SchedHint */) noexcept;

    std::optional<IntrusiveList<TaskType>> yawn_tasks(size_t requested_size);
    WsExecutor<TaskType, Config>& host() const;

  private:  // member-functions:
    [[nodiscard]] TaskPtr pick_task() noexcept;

    std::optional<TaskPtr> try_pick_fast() noexcept;
    std::optional<TaskPtr> try_steal_any() noexcept;

    std::optional<TaskPtr> try_pop_lifo() noexcept;
    std::optional<TaskPtr> try_pop_local() noexcept;
    std::optional<TaskPtr> try_pop_global() noexcept;

    void work();  // run-loop;
};

};  // namespace wr
