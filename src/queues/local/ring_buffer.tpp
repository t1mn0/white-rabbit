#include "ring_buffer.hpp"

namespace wr::queues {

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto RingBuffer<TaskType, Capacity>::load(uint64_t index) const noexcept -> RingBuffer<TaskType, Capacity>::ValueType {

    return slots_[to_local_index(index)].load();
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
void RingBuffer<TaskType, Capacity>::store(uint64_t index, ValueType val) noexcept {

    slots_[to_local_index(index)].store(val);
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
constexpr size_t RingBuffer<TaskType, Capacity>::to_local_index(uint64_t global) noexcept {

    /* since size_t has system-dependent size, we have to guarantee bit safety */
    return static_cast<size_t>(global) & kMask;
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
constexpr size_t RingBuffer<TaskType, Capacity>::capacity() const noexcept {
    return Capacity;
}

}  // namespace wr::queues
