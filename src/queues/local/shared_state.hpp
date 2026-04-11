#pragma once

#include <cstddef>
#include <cstdint>

#include "ring_buffer.hpp"
#include "tasks/concept.hpp"
#include "utils/constants.hpp"

using wr::utils::constants::CACHE_LINE_SIZE;

namespace wr::queues {

/**
 * @brief SharedState : The core data structure shared between owner and thieves.
 * It provides ergonomic access to shared RingBuffer
 * and handles HOW to access data without any coordination logic.
 *
 * We divide our shared resources by aligning them to different cache-lines
 * in order to prevent false-sharing.
 *
 * => Stealers modifying top_ don't invalidate owner's cache-line
 * => Owner modifying bottom_ doesn't invalidate stealer's cache-line
 *
 *
 * @section Invariants
 *
 * >> (top <= bottom)
 * >> (bottom - top <= capacity)
 * >> (empty when top == bottom)
 *
 */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class SharedState {
  public:  // nested types:
    using ValueType = typename RingBuffer<TaskType, Capacity>::ValueType;

  private:
    /* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

    /* Storage for task pointers. */
    alignas(CACHE_LINE_SIZE) RingBuffer<TaskType, Capacity> tasks_;

    /* First valid element index [global]. Modify by stealers. */
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> top_{0};

    /* Next free slot index [global]. Modify by owner only. */
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> bottom_{0};

    /* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

  public:  // member functions:
    SharedState() = default;

    SharedState(const SharedState&) = delete;
    SharedState& operator=(const SharedState&) = delete;

    SharedState(SharedState&&) = delete;
    SharedState& operator=(SharedState&&) = delete;

    /*
     * @brief Load bottom index.
     */
    [[nodiscard]]
    uint64_t load_bottom_idx(std::memory_order mo = std::memory_order_seq_cst) const noexcept {
        ///
        return bottom_.load(mo);
        ///
    }

    /*
     * @brief Load top index.
     */
    [[nodiscard]]
    uint64_t load_top_idx(std::memory_order mo = std::memory_order_seq_cst) const noexcept {
        ///
        return top_.load(mo);
        ///
    }

    /*
     * @brief Store bottom index.
     * @param idx New bottom value.
     */
    void store_bottom_idx(uint64_t idx, std::memory_order mo = std::memory_order_seq_cst) noexcept {
        ///
        bottom_.store(idx, mo);
        ///
    }

    /*
     * @brief Load task from buffer by given index.
     */
    [[nodiscard]]
    ValueType load_task(uint64_t idx, std::memory_order mo = std::memory_order_seq_cst) const noexcept {
        ///
        return tasks_.load(idx, mo);
        ///
    }

    /*
     * @brief Store task into buffer at given index.
     */
    void store_task(uint64_t idx, ValueType task, std::memory_order mo = std::memory_order_seq_cst) noexcept {
        ///
        tasks_.store(idx, task, mo);
        ///
    }

    /*
     * @brief Atomically increment current top if it equals expected value.
     * @param expected Current expected value of top.
     * @return true if CAS succeeded => we claimed the slot, false otherwise.
     */
    [[nodiscard]]
    bool try_increment_top(uint64_t expected, std::memory_order mo = std::memory_order_seq_cst) noexcept {
        /*
         * if top := expected => now top = expected + 1
         */
        return top_.compare_exchange_strong(expected, expected + 1, mo);
    }

    /*
     * @brief Atomically increment current top by count if it equals expected value.
     * @param count How many slots to claim.
     * @param expected Current expected value of top.
     *
     * [Used for batch stealing].
     */
    [[nodiscard]]
    bool try_increment_top_by(
        uint64_t expected,
        uint64_t count,
        std::memory_order mo = std::memory_order_seq_cst
    ) noexcept {
        ///
        if (count > bottom_.load()) {
            return false;
        }

        return top_.compare_exchange_strong(expected, expected + count, mo);
        ///
    }
};

};  // namespace wr::queues
