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

};  // namespace wr::queues

#include "loot.tpp"
