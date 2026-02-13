#pragma once

#include "../../tasks/concept.hpp"
#include "utils.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace wr::queues {

/**
 * @brief Fixed-size circular buffer for task pointers.
 *
 * @section PHILOSOPHY
 *
 * ??? Why circular buffer ???
 *
 * For a local deque in ws scheduler we need a data structure that:
 *  >> Has efficient push/pop operations without any memory allocations,
 *      in order to achieve lock-free design.
 *
 *  >> Since we want to model/achieve the deque behavior and support work-stealing,
 *      this 'desired' data structure should have an ability to act with LIFO behavior for worker and with FIFO behavior for stealers.
 *
 *  >> Has quite predictable memory usage.
 *
 *  => Circular buffer is a perfect variant.
 *  It has O(1) complexity on push/pop operations.
 *  It has bounded capacity.
 *  It has zero allocation in hot path and reuses the same memory.
 *  And it is lock-free friendly.
 *
 *
 * @section INDEXING
 *  We use two index concepts : [global] and [local].
 *
 *  >> Global index represents logical position in infinite stream :
 *  ... | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | ...
 *
 *  Top and Bottom are global indeces and NEVER decrease : they grow monotonically from 0 to infty
 *
 *  => Current deque size := [Bottom - Top]
 *
 *  => No ABA problem with indices [they never repeat].
 *
 *
 *  >> Local index represents physical slot in array.
 *
 *  We don't have infinite memory, so we 'fold' our infinite tape into a ring of fixed size.
 *
 *  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | ...
 *
 *  | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | ...
 *    |---------------|
 *          CAPACITY
 *
 *  @section MAPPING / SLICING
 *
 *  >> That's actually simple : [Local] = [Global] % [Capacity]
 *  But since capacity := 2^N => It reduces to : [Global] & [mask], where mask := [capacity - 1]
 *
 * @tparam Capacity must be a power of two.
 */
template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class RingBuffer {
  public:
    using ValueType = TaskType*;

    static constexpr size_t kCapacity = Capacity;
    static constexpr size_t kMask = Capacity - 1;

    RingBuffer() = default;

    /* Non-copiable : slots contains atomics, copying would break inveriants of atomicity */
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    /* Non-movable : same */
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    /**
     * @brief Load task ptr from slot.
     * @param index Global index
     * @return Task pointer stored at that logical position.
     */
    auto load(uint64_t index) const noexcept -> ValueType;

    /**
     * @brief Store task pointer into slot.
     * @param index Global index.
     * @param value Task ptr to store.
     */
    void store(uint64_t index, ValueType value) noexcept;

    /**
     * @brief Convert global index to local slot index.
     * @param global Any positive int
     * @return Index in range [0, Capacity)
     */
    [[nodiscard]]
    static constexpr size_t to_local_index(uint64_t global) noexcept;

    constexpr size_t capacity() const noexcept;

  private:
    std::array<std::atomic<ValueType>, Capacity> slots_;
};

};  // namespace wr::queues

#include "ring_buffer.tpp"
