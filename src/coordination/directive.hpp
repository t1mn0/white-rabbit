#pragma once

#include "throttler.hpp"
#include <optional>

namespace wr::coord {

// An action for the worker what to do
// depending on the current number of thieves, which is determined by the Coordinator.
enum class Action : uint8_t {
    Steal,    /* have semaphore's Permit */
    Park,     /* command to park that worker */
    Retry,    /* two-phase parking */
    Terminate /* Gracefull shutdown for all workers */
};

/*
 * @brief CoordDirective is a component that encapsulates the result of a request to the Coordinator.
 * The Worker asks the coordinator if it can look for a victim
 * response comes with the status
 *
 */
class CoordDirective {
  public:  // nested types:
    using Permit = Throttler::StealPermit;

  public:  // member static functions:
    static CoordDirective Steal(Permit&& permit);

    static CoordDirective Park();

    static CoordDirective Retry();

    static CoordDirective Terminate();

  public:  // member functions:
    bool should_steal() const noexcept;

    bool should_retry() const noexcept;

    bool should_terminate() const noexcept;

    bool should_park() const noexcept;

    [[nodiscard]] Permit unwrap_permit() &&;

  private:
    explicit CoordDirective(Action action, std::optional<Permit> permit = std::nullopt);

    Action action_;
    std::optional<Permit> permit_;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

CoordDirective CoordDirective::Steal(Permit&& permit) {
    ///
    return CoordDirective(Action::Steal, std::move(permit));
    ///
}

CoordDirective CoordDirective::Park() {
    ///
    return CoordDirective(Action::Park);
    ///
}

CoordDirective CoordDirective::Retry() {
    ///
    return CoordDirective(Action::Retry);
    ///
}

CoordDirective CoordDirective::Terminate() {
    ///
    return CoordDirective(Action::Terminate);
    ///
}

bool CoordDirective::should_steal() const noexcept {
    ///
    return action_ == Action::Steal;
    ///
}

bool CoordDirective::should_retry() const noexcept {
    ///
    return action_ == Action::Retry;
    ///
}

bool CoordDirective::should_terminate() const noexcept {
    ///
    return action_ == Action::Terminate;
    ///
}

bool CoordDirective::should_park() const noexcept {
    ///
    return action_ == Action::Park;
    ///
}

CoordDirective::Permit CoordDirective::unwrap_permit() && {
    ///
    assert(should_steal() && permit_.has_value());
    return std::move(*permit_);
    ///
}

CoordDirective::CoordDirective(Action action, std::optional<Permit> permit)
    : action_(action), permit_(std::move(permit)) {}

}  // namespace wr::coord
