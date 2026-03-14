#pragma once

#include <optional>

#include "throttler.hpp"

namespace wr::coord {

/* An action for the worker what to do depending on the current number of thieves, which is
 * determined by the Coordinator */
enum class Action : uint8_t {
    Steal,    /* have semaphore's Permit */
    Park,     /* command to park that worker */
    Retry,    /* two-phase parking */
    Terminate /* Gracefull shutdown for all workers */
};

/*
 * @brief CoordDirective is a component that encapsulates the result of a request to the
 * Coordinator. The Worker asks the coordinator if it can look for a victim response comes with the
 * status
 *
 */
class Directive {
  public:  // nested types:
    using Permit = Throttler::StealPermit;

  private:  // data members:
    explicit Directive(Action action, std::optional<Permit> permit = std::nullopt);

    Action action_;
    std::optional<Permit> permit_;

  public:  // member static functions:
    static Directive Steal(Permit&& permit);

    static Directive Park();

    static Directive Retry();

    static Directive Terminate();

  public:  // member functions:
    bool should_steal() const noexcept;

    bool should_retry() const noexcept;

    bool should_terminate() const noexcept;

    bool should_park() const noexcept;

    [[nodiscard]] Permit unwrap_permit() &&;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

inline Directive Directive::Steal(Permit&& permit) {
    return Directive(Action::Steal, std::move(permit));
}

inline Directive Directive::Park() {
    return Directive(Action::Park);
}

inline Directive Directive::Retry() {
    return Directive(Action::Retry);
}

inline Directive Directive::Terminate() {
    return Directive(Action::Terminate);
}

inline bool Directive::should_steal() const noexcept {
    return action_ == Action::Steal;
}

inline bool Directive::should_retry() const noexcept {
    return action_ == Action::Retry;
}

inline bool Directive::should_terminate() const noexcept {
    return action_ == Action::Terminate;
}

inline bool Directive::should_park() const noexcept {
    return action_ == Action::Park;
}

inline Directive::Permit Directive::unwrap_permit() && {
    assert(should_steal() && permit_.has_value());
    return std::move(*permit_);
}

inline Directive::Directive(Action action, std::optional<Permit> permit)
    : action_(action), permit_(std::move(permit)) {}

}  // namespace wr::coord
