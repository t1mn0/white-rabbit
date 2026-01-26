#pragma once

#include "../../tasks/concept.hpp"
#include <cassert>
#include <cstdint>
#include <optional>

namespace wr::queues {

template <task::Task TaskT>
class Loot {
  public:
    enum class State : uint8_t {
        Success, /* Stolen task is available */
        Empty,   /* Queue was empty => there is nothing to stole */
        Retry,   /* Contention occurred, worth retrying */
    };

  public:
    [[nodiscard]] auto static success(TaskT* task) noexcept -> Loot;

    [[nodiscard]] auto static empty() noexcept -> Loot;

    [[nodiscard]] auto static retry() noexcept -> Loot;


    bool is_success() const noexcept;

    bool is_empty() const noexcept;

    bool is_retry() const noexcept;

    [[nodiscard]] State get_state() const noexcept;

    /* @brief Consumes the loot [!panics if not Success!] */
    TaskT* unwrap() &&;

    /* @brief Safely extracts the task as optional */
    std::optional<TaskT*> as_optional() && noexcept;


  private:
    Loot(State, TaskT*) noexcept;
    explicit Loot(State state) noexcept;

  private:
    State state_{Loot<TaskT>::State::Empty};
    TaskT* reward_{nullptr};
};

/* ------------------------------------------------------------------- */


template <task::Task TaskT>
inline Loot<TaskT>::Loot(State s, TaskT* task) noexcept : state_(s), reward_(task) {}

template <task::Task TaskT>
inline Loot<TaskT>::Loot(State state) noexcept : state_(state), reward_(nullptr) {}

template <task::Task TaskT>
inline auto Loot<TaskT>::success(TaskT* task) noexcept -> Loot {
    return Loot(State::Success, task);
}

template <task::Task TaskT>
inline auto Loot<TaskT>::empty() noexcept -> Loot {
    return Loot(State::Empty);
}

template <task::Task TaskT>
inline auto Loot<TaskT>::retry() noexcept -> Loot {
    return Loot(State::Retry);
}

template <task::Task TaskT>
inline bool Loot<TaskT>::is_success() const noexcept {
    return state_ == State::Success;
}

template <task::Task TaskT>
inline bool Loot<TaskT>::is_empty() const noexcept {
    return state_ == State::Empty;
}

template <task::Task TaskT>
inline bool Loot<TaskT>::is_retry() const noexcept {
    return state_ == State::Retry;
}

template <task::Task TaskT>
inline auto Loot<TaskT>::get_state() const noexcept -> State {
    return state_;
}

template <task::Task TaskT>
inline TaskT* Loot<TaskT>::unwrap() && {
    assert(is_success());
    return std::move(reward_);
}

template <task::Task TaskT>
inline std::optional<TaskT*> Loot<TaskT>::as_optional() && noexcept {
    if (is_success()) {
        return reward_;
    } else {
        return std::nullopt;
    }
}

};  // namespace wr::queues
