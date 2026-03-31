#pragma once

#include <optional>

#include "throttler.hpp"

namespace wr::coord {

/* *---*---*---*---*---*---*---*---*---*---*---* */

/* An action for the worker what to do
 * depending on the current number of thieves,
 *  which is determined by the Coordinator
 */
enum class Action : uint8_t {
    Steal,    /* have semaphore's Permit */
    Park,     /* command to park that worker */
    Retry,    /* two-phase parking */
    Terminate /* Gracefull shutdown for all workers */
};

/* *---*---*---*---*---*---*---*---*---*---*---* */

/*
 * @brief CoordDirective is a component that encapsulates the result of a request to the
 * Coordinator. The Worker asks the coordinator if it can look for a victim response comes with
 * the status
 *
 */
class Directive {
  public:  // nested types:
    using Permit = Throttler::StealPermit;

  private:
    /* *---*---*---*---*---*---*---* */

    Action action_;
    std::optional<Permit> permit_;

    /* *---*---*---*---*---*---*---* */

  public:
    explicit Directive(Action action, std::optional<Permit> permit = std::nullopt)
        : action_(action),
          permit_(std::move(permit)) {}

  public:  // member static functions:
    static Directive Steal(Permit&& permit) {
        ///
        return Directive(Action::Steal, std::move(permit));
        ///
    }

    static Directive Park() {
        ///
        return Directive(Action::Park);
        ///
    }

    static Directive Retry() {
        ///
        return Directive(Action::Retry);
        ///
    }

    static Directive Terminate() {
        ///
        return Directive(Action::Terminate);
        ///
    }

  public:  // member functions:
    bool should_steal() const noexcept {
        ///
        return action_ == Action::Steal;
        ///
    }

    bool should_retry() const noexcept {
        ///
        return action_ == Action::Retry;
        ///
    }

    bool should_terminate() const noexcept {
        ///
        return action_ == Action::Terminate;
        ///
    }

    bool should_park() const noexcept {
        ///
        return action_ == Action::Park;
        ///
    }

    [[nodiscard]] Permit unwrap_permit() && {
        assert(should_steal() && permit_.has_value());
        return std::move(*permit_);
    }
};


}  // namespace wr::coord
