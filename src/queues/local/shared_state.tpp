#include "shared_state.hpp"

namespace wr::queues {


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

}  // namespace wr::queues
