#pragma once

#include "loot.hpp"
#include "queues/local/utils.hpp"
#include "shared_state.hpp"

namespace wr::queues {

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class WorkStealingQueue; /* forward-declaration */

/**
 * @brief StealHandle is an interface for stealers.
 *
 * @section LIFETIME
 *
 *  >> StealHandle must not outlive a local queue
 */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class StealHandle {
  public:  // member-functions:
    explicit StealHandle(SharedState<TaskType, Capacity>* state) : state_(state) {}

    [[nodiscard]]
    /* !! TODO !! */ queues::Loot<TaskType> steal() noexcept;

    [[nodiscard]]
    /* !! TODO !! */ Loot<TaskType> steal_batch_and_pop(WorkStealingQueue<TaskType, Capacity>& dest) noexcept;

    [[nodiscard]]
    bool is_empty() const noexcept;

    StealHandle(const StealHandle& other) = default;
    StealHandle(StealHandle&&) = default;
    StealHandle& operator=(const StealHandle&) = default;
    StealHandle& operator=(StealHandle&&) = default;

  private:  // fields:
    SharedState<TaskType, Capacity>* state_;

    friend class WorkStealingQueue<TaskType, Capacity>;
};

};  // namespace wr::queues

#include "steal_handle.tpp"
