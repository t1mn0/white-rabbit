#include "loot.hpp"

namespace wr::queues {

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskType>
Loot<TaskType>::Loot(State s, TaskType* task) noexcept : state_(s), reward_(task) {}

template <task::Task TaskType>
Loot<TaskType>::Loot(State state) noexcept : state_(state), reward_(nullptr) {}

template <task::Task TaskType>
auto Loot<TaskType>::Success(TaskType* task) noexcept -> Loot {
    return Loot(State::Success, task);
}

template <task::Task TaskType>
auto Loot<TaskType>::Empty() noexcept -> Loot {
    return Loot(State::Empty);
}

template <task::Task TaskType>
auto Loot<TaskType>::Retry() noexcept -> Loot {
    return Loot(State::Retry);
}

template <task::Task TaskType>
bool Loot<TaskType>::is_success() const noexcept {
    return state_ == State::Success;
}

template <task::Task TaskType>
bool Loot<TaskType>::is_empty() const noexcept {
    return state_ == State::Empty;
}

template <task::Task TaskType>
bool Loot<TaskType>::is_retry() const noexcept {
    return state_ == State::Retry;
}

template <task::Task TaskType>
auto Loot<TaskType>::get_state() const noexcept -> State {
    return state_;
}

template <task::Task TaskType>
TaskType* Loot<TaskType>::unwrap() && {
    assert(is_success());
    return std::move(reward_);
}

template <task::Task TaskType>
template <typename A>
TaskType* Loot<TaskType>::unwrap_or(A&& some_action) && {
    if (is_success()) {
        return reward_;
    } else {
        some_action();
    }
}

template <task::Task TaskType>
std::optional<TaskType*> Loot<TaskType>::as_optional() && noexcept {
    return is_success() ? std::optional{reward_} : std::nullopt;
}

}  // namespace wr::queues
