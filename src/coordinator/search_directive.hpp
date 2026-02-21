#pragma once

#include "throttler.hpp"
#include <optional>

namespace wr::coord {

// An action for the worker what to do depending on the current
// number of thieves, which is determined by the Coordinator.
enum class Action : uint8_t {
    Search,    // have semaphore's Permit
    Wait,      // command to park that worker
    Retry,     // !maybe use it for two-phase parking or delete
    Terminate  // !maybe if sched shutdown, we can use notification via Coordinator to all Workers.
               // But it seems like not the responsibility of Coordinator.
};

// SearchDirective encapsulates the result of the request to the Coordinator.
// Worker asks the coordinator if he can look for a victim in the response (the `SearchDirective` object)
// with the status `SearchDirective::Search` gets `SearchPermit` or another `SearchDirective` value.
class SearchDirective {
  public:  // nested types:
    using Permit = Throttler::SearchPermit;

  public:  // member static functions:
    static SearchDirective Search(Permit&& permit) {
        return SearchDirective(Action::Search, std::move(permit));
    }

    static SearchDirective Wait() {
        return SearchDirective(Action::Wait);
    }

    static SearchDirective Retry() {
        return SearchDirective(Action::Retry);
    }

    static SearchDirective Terminate() {
        return SearchDirective(Action::Terminate);
    }

  public:  // member functions:
    bool is_search_permitted() const noexcept { return action_ == Action::Search; }

    bool is_retry_requested() const noexcept { return action_ == Action::Retry; }

    bool is_terminate_requested() const noexcept { return action_ == Action::Terminate; }

    bool is_should_park() const noexcept { return action_ == Action::Wait; }

    [[nodiscard]] Permit take_permit() && {
        assert(is_search_permitted() && permit_.has_value());
        return std::move(*permit_);
    }

  private:
    explicit SearchDirective(Action action, std::optional<Permit> permit = std::nullopt)
        : action_(action), permit_(std::move(permit)) {}

    Action action_;
    std::optional<Permit> permit_;
};

}  // namespace wr::coord
