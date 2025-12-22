#pragma once

#include "../../tasks/task.hpp"
#include <cassert>
#include <cstdint>
#include <optional>

namespace wr::queues {

/**
 * @brief Result of a steal operation.
 *
 * |> Success(TaskBase*) | Empty | Retry
 *
 */
class Loot {
  public:
    enum class State : uint8_t {
        Success, /* Stolen task is available */
        Empty,   /* Queue was empty => there is nothing to stole */
        Retry,   /* Contention occurred, worth retrying */
    };

  public:
    [[nodiscard]] auto static success(TaskBase* task) noexcept -> Loot;

    [[nodiscard]] auto static empty() noexcept -> Loot;

    [[nodiscard]] auto static retry() noexcept -> Loot;


    bool is_success() const noexcept;

    bool is_empty() const noexcept;

    bool is_retry() const noexcept;

    [[nodiscard]] State get_state() const noexcept;

    /* @brief Consumes the loot [!panics if not Success!] */
    TaskBase* unwrap() &&;

    /* @brief Safely extracts the task as optional */
    std::optional<TaskBase*> as_optional() && noexcept;


  private:
    Loot(State, TaskBase*) noexcept;
    explicit Loot(State state) noexcept;

  private:
    State state_{Loot::State::Empty};
    TaskBase* reward_{nullptr};
};

/* ------------------------------------------------------------------- */


inline Loot::Loot(State s, TaskBase* task) noexcept : state_(s), reward_(task) {}

inline Loot::Loot(State state) noexcept : state_(state), reward_(nullptr) {}

inline auto Loot::success(TaskBase* task) noexcept -> Loot {
    assert(task != nullptr);
    return Loot(State::Success, task);
}

inline auto Loot::empty() noexcept -> Loot {
    return Loot(State::Empty);
}

inline auto Loot::retry() noexcept -> Loot {
    return Loot(State::Retry);
}

inline bool Loot::is_success() const noexcept {
    return state_ == State::Success;
}

inline bool Loot::is_empty() const noexcept {
    return state_ == State::Empty;
}

inline bool Loot::is_retry() const noexcept {
    return state_ == State::Retry;
}

inline auto Loot::get_state() const noexcept -> State {
    return state_;
}

inline TaskBase* Loot::unwrap() && {
    assert(is_success());
    return std::move(reward_);
}

inline std::optional<TaskBase*> Loot::as_optional() && noexcept {
    if (is_success()) {
        return reward_;
    } else {
        return std::nullopt;
    }
}

};  // namespace wr::queues
