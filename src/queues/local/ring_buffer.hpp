#pragma once

#include "../../tasks/task.hpp"
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
 *  |> Has efficient push/pop operations without any memory allocations,
 *      in order to achieve lock-free design.
 *
 *  |> Since we want to model/achieve the deque behavior and support work-stealing,
 *      this 'desired' data structure should have LIFO behavior for worker and FIFO behavior for stealers.
 *
 *  |> Has quite predictable memory usage.
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
 *  |> Global index represents logical position in infinite stream :
 *  ... | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | ...
 *
 *  Top and Bottom are global indeces and NEVER decrease : they grow monotonically from 0 to infty
 *
 *  => Current deque size := [Bottom - Top]
 *
 *  => No ABA problem with indices [they never repeat].
 *
 *  |> Local index represents physical slot in array.
 *
 *  We don't have infinity memory, so we 'fold' our infinite tape into a ring of fixed size.
 *
 *  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | ...
 *
 *  | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | ...
 *    |---------------|
 *          CAPACITY
 *
 *  @section MAPPING
 *
 *  |> That's actually simple : [Local] = [Global] % [Capacity]
 *  But since capacity := 2^N => It resuces to : [Global] & [mask], where mask := [capacity - 1]
 *
 * @tparam Capacity must be a power of two.
 */
template <size_t Capacity>
    requires IsPowerOfTwo<Capacity>
class RingBuffer {
  public:
    using ValueType = TaskBase*;

    RingBuffer() = default;

    /* Non-copiable */
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    /* Non-movable */
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    /**
     * @brief Load task ptr from slot.
     * @param index Global index
     */
    ValueType load(uint64_t index, std::memory_order = std::memory_order::seq_cst) const;

    /**
     * @brief Store task pointer into slot.
     */
    void store(uint64_t index, ValueType value, std::memory_order = std::memory_order::seq_cst);

  private:
    static constexpr size_t to_local_index(uint64_t global);

    static constexpr size_t kCapacity = Capacity;
    static constexpr size_t kMask = Capacity - 1;

    std::array<std::atomic<ValueType>, Capacity> slots_;
};

/* ------------------------------------------------------------------- */


};  // namespace wr::queues
