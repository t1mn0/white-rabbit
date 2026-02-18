#pragma once

#include "exec/config.hpp"
#include "fwd.hpp"
#include "loot.hpp"
#include "queues/local/utils.hpp"
#include "shared_state.hpp"

namespace wr::queues {


/**
 * @brief StealHandle is an interface for stealers.
 *
 * @section LIFETIME
 *
 *  >> StealHandle must not outlive a local queue
 */

template <task::Task TaskType, size_t Capacity>
class StealHandle {
  public:  // member-functions:
    explicit StealHandle(SharedState<TaskType, Capacity>* state) : state_(state) {}

    [[nodiscard]]
    /* !! TODO !! */ Loot<TaskType> steal() noexcept;

    [[nodiscard]]
    /* !! TODO !! */ Loot<TaskType> steal_batch_and_pop(WorkStealingQueue<TaskType, Capacity>& dest) noexcept;

    [[nodiscard]]
    bool is_empty() const noexcept;

    StealHandle(const StealHandle& other) = default;
    StealHandle(StealHandle&&) = default;
    StealHandle& operator=(const StealHandle&) = default;
    StealHandle& operator=(StealHandle&&) = default;

  private:  // fields:
    /* We're holding ptr tthe other worker's shared state. It's like ticket or pass for the stealing */
    SharedState<TaskType, Capacity>* state_;

    friend class WorkStealingQueue<TaskType, Capacity>;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

/*
 * ...
 */

};  // namespace wr::queues
