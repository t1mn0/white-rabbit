#pragma once

#include "exec/config.hpp"
#include "ring_buffer.hpp"
#include "tasks/concept.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>

namespace wr::queues {

/**
 * @brief SharedState : The core data structure shared between Owner and Thieves.
 * It provides ergonomic access to shared RingBuffer
 * and handles HOW to access data without any coordination logic.
 *
 * We divide our shared resources by aligning them to different cache-lines in order to prevent false-sharing.
 * => Stealers modifying top_ don't invalidate owner's cache-line
 * => Owner modifying bottom_ doesn't invalidate stealer's cache-line
 *
 *
 * @section INVARIANTS
 *
 * >> (top <= bottom)
 * >> (bottom - top <= capacity)
 * >> (empty when top == bottom)
 *
 */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class SharedState {
  public:
    using ValueType = typename RingBuffer<TaskType, Capacity>::ValueType;

    SharedState() = default;

    SharedState(const SharedState&) = delete;
    SharedState& operator=(const SharedState&) = delete;

    SharedState(SharedState&&) = delete;
    SharedState& operator=(SharedState&&) = delete;


    /*
     * @brief Load bottom index.
     */
    [[nodiscard]]
    uint64_t load_bottom() const noexcept;

    /*
     * @brief Load top index.
     */
    [[nodiscard]]
    uint64_t load_top() const noexcept;

    /*
     * @brief Store bottom index.
     * @param idx New bottom value.
     */
    void store_bottom(uint64_t idx) noexcept;

    /*
     * @brief Load task from buffer by given index.
     */
    [[nodiscard]]
    auto load_task(uint64_t idx) const noexcept -> ValueType;

    /*
     * @brief Store task into buffer at given index.
     */
    void store_task(uint64_t idx, ValueType task) noexcept;

    /*
     * @brief Atomically increment current top if it equals expected value.
     * @param expected Current expected value of top.
     * @return true if CAS succeeded => we claimed the slot, false otherwise.
     */
    [[nodiscard]]
    bool try_increment_top(uint64_t expected) noexcept;

    /*
     * @brief Atomically increment current top by count if it equals expected value.
     * @param count How many slots to claim.
     * @param expected Current expected value of top.
     *
     * [Used for batch stealing].
     */
    [[nodiscard]]
    bool try_increment_top_by(uint64_t expected, uint64_t count) noexcept;


  private:
    /* Storage for task pointers. */
    alignas(utils::constants::CACHE_LINE_SIZE) RingBuffer<TaskType, Capacity> tasks_;

    /* First valid element index [global]. Modify by stealers. */
    alignas(utils::constants::CACHE_LINE_SIZE) std::atomic<uint64_t> top_{0};

    /* Next free slot index [global]. Modify by owner only. */
    alignas(utils::constants::CACHE_LINE_SIZE) std::atomic<uint64_t> bottom_{0};
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
uint64_t SharedState<TaskType, Capacity>::load_bottom() const noexcept {
    return top_.load();
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
uint64_t SharedState<TaskType, Capacity>::load_top() const noexcept {
    return top_.load();
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
void SharedState<TaskType, Capacity>::store_bottom(uint64_t idx) noexcept {
    bottom_.store(idx);
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto SharedState<TaskType, Capacity>::load_task(uint64_t idx) const noexcept -> ValueType {
    return tasks_.load(idx);
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
void SharedState<TaskType, Capacity>::store_task(uint64_t idx, ValueType task) noexcept {
    tasks_.store(idx, task);
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
bool SharedState<TaskType, Capacity>::try_increment_top(uint64_t expected) noexcept {
    return top_.compare_exchange_strong(expected, expected + 1); /* if top := expected => top = expected + 1 */
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
bool SharedState<TaskType, Capacity>::try_increment_top_by(uint64_t expected, uint64_t count) noexcept {
    return top_.compare_exchange_strong(expected, expected + count);
}


};  // namespace wr::queues
