#pragma once

#include "../../tasks/task.hpp"
#include <cstdint>
#include <optional>

namespace wr::queues {

/**
 * @brief Loot is an algebraic data type that represents result of stealing process.
 */
template <typename T>
class Loot {
  public:
    enum class State : uint8_t {
        Success,
        Empty,
        Retry,
    };

  public:
    [[nodiscard]] auto static success() -> Loot;

    [[nodiscard]] auto static empty() -> Loot;

    [[nodiscard]] auto static retry() -> Loot;


    bool is_success() const noexcept;

    bool is_empty() const noexcept;

    bool is_retry() const noexcept;

    /* takes the ownership and unwrap the optional reward */
    TaskBase* unwrap() &&;

    TaskBase* operator*() const noexcept;

  private:
    State state_{Loot::State::Empty};
    std::optional<TaskBase*> reward_;

  private:
    Loot(State s, TaskBase* task);
};

};  // namespace wr::queues
