#pragma once

#include <cassert>
#include <cstdint>
#include <optional>

#include "../../tasks/concept.hpp"

namespace wr::queues {

enum class State : uint8_t {
    Success, /* Stolen task is available */
    Empty,   /* Queue was empty => there is nothing to stole */
    Retry,   /* Contention occurred, worth retrying */
};

template <task::Task TaskType>
class Loot {
  private:
    /* *---*---*---*---*---*---* */

    State state_{State::Empty};
    TaskType* reward_{nullptr};

    /* *---*---*---*---*---*---* */

  private:  // member functions:
    Loot(State s, TaskType* task) noexcept
        : state_(s),
          reward_(task) {}

    explicit Loot(State state) noexcept
        : state_(state),
          reward_(nullptr) {}

  public:  // member functions:
    [[nodiscard]] auto static Success(TaskType* task) noexcept -> Loot {
        ///
        return Loot(State::Success, task);
        ///
    }

    [[nodiscard]] auto static Empty() noexcept -> Loot {
        ///
        return Loot(State::Empty);
        ///
    }

    [[nodiscard]] auto static Retry() noexcept -> Loot {
        ///
        return Loot(State::Retry);
        ///
    }

    bool success() const noexcept {
        ///
        return state_ == State::Success;
        ///
    }

    bool empty() const noexcept {
        ///
        return state_ == State::Empty;
        ///
    }

    bool retry() const noexcept {
        ///
        return state_ == State::Retry;
        ///
    }

    [[nodiscard]] State get_state() const noexcept {
        ///
        return state_;
        ///
    }

    /* @brief Consumes the loot [!panics if not Success!] */
    TaskType* unwrap() && {
        assert(success());
        return std::move(reward_);
    }

    /* @brief Consumes the loot and do an action on error */
    template <typename A>
    TaskType* unwrap_or(A&& some_action) && {
        if (success()) {
            return reward_;
        }
        return some_action();
    }

    /* @brief Safely extracts the task as optional */
    std::optional<TaskType*> as_optional() && noexcept {
        ///
        return success() ? std::optional{reward_} : std::nullopt;
        ///
    }
};


};  // namespace wr::queues
