#pragma once

#include "../../tasks/concept.hpp"
#include <cassert>
#include <cstdint>
#include <optional>

namespace wr::queues {

enum class State : uint8_t {
    Success, /* Stolen task is available */
    Empty,   /* Queue was empty => there is nothing to stole */
    Retry,   /* Contention occurred, worth retrying */
};

template <task::Task TaskType>
class Loot {
  private:  // member-functions:
    Loot(State, TaskType*) noexcept;
    explicit Loot(State state) noexcept;

  public:  // member-functions:
    [[nodiscard]] auto static Success(TaskType* task) noexcept -> Loot;
    [[nodiscard]] auto static Empty() noexcept -> Loot;
    [[nodiscard]] auto static Retry() noexcept -> Loot;

    bool is_success() const noexcept;
    bool is_empty() const noexcept;
    bool is_retry() const noexcept;

    [[nodiscard]] State get_state() const noexcept;

    /* @brief Consumes the loot [!panics if not Success!] */
    TaskType* unwrap() &&;

    /* @brief Consumes the loot and do an action on error */
    template <typename A>
    TaskType* unwrap_or(A&& some_action) &&;

    /* @brief Safely extracts the task as optional */
    std::optional<TaskType*> as_optional() && noexcept;

  private:  // fields:
    State state_{State::Empty};
    TaskType* reward_{nullptr};
};

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


};  // namespace wr::queues
