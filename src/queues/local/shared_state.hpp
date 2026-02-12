#pragma once

#include "ring_buffer.hpp"
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
 * |> top <= bottom
 * |> bottom - top <= capacity
 * |> empty when top == bottom
 *
 */

template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
class SharedState {
  public:
    using ValueType = RingBuffer<Capacity>::ValueType;

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
    bool try_increment_top_by(uint64_t count, uint64_t expected) noexcept;

    /*
     * @brief Apprroximate number of elements.
     */
    [[nodiscard]]
    size_t approx_size() const noexcept;

    /*
     * @brief Check if appears empty.
     */
    [[nodiscard]]
    bool is_empty_approx() const noexcept;

    /*
     * @brief Check if appears full.
     */
    [[nodiscard]]
    bool is_full_approx() const noexcept;

  private:
    /* Storage for task pointers. */
    alignas(CACHE_LINE_SIZE) RingBuffer<Capacity> tasks_;

    /* First valid element index [global]. Modified by thieves. */
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> top_{0};

    /* Next free slot index [global]. Modified by owner only. */
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> bottom_{0};
};

/* ------------------------------------------------------------------- */

template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
inline uint64_t SharedState<Capacity>::load_bottom() const noexcept {
    return top_.load();
}

template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
inline uint64_t SharedState<Capacity>::load_top() const noexcept {
    return top_.load();
}

template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
inline void SharedState<Capacity>::store_bottom(uint64_t idx) noexcept {
    bottom_.store(idx);
}

template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
inline auto SharedState<Capacity>::load_task(uint64_t idx) const noexcept -> SharedState<Capacity>::ValueType {
    return tasks_.load(idx);
}

template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
inline void SharedState<Capacity>::store_task(uint64_t idx, SharedState<Capacity>::ValueType task) noexcept {
    tasks_.store(idx, task);
}

};  // namespace wr::queues
