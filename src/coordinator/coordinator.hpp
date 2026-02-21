#pragma once

#include "search_directive.hpp"
#include "throttler.hpp"

namespace wr::coord {

// Coordinator encapsulates the logic of decision-making for theft:
// whether the Worker should continue searching for tasks,
// whether it needs to fall asleep immediately, or
// whether it is worth trying to retrieve tasks again (two-phase parking)

class Coordinator {
  private:  // data members:
    Throttler semaphore_;
    std::atomic<bool> shutdown_requested_ = false;
    std::atomic<bool> work_maybe_available_ = false;

  public:  // nested types:
    using SearchPermit = Throttler::SearchPermit;

  public:  // member functions:
    explicit Coordinator(size_t total_workers)
        : semaphore_(total_workers > 1 ? total_workers / 2 : 1) {}

    // main method for requesting instructions by the Worker
    [[nodiscard]] SearchDirective try_acquire_search_permit() noexcept {
        if (shutdown_requested_.load()) {
            return SearchDirective::Terminate();
        }

        auto permit = semaphore_.try_acquire_permit();
        if (permit.has_value()) {
            return SearchDirective::Search(std::move(*permit));
        }

        if (work_maybe_available_.load()) {
            return SearchDirective::Retry();
        }

        return SearchDirective::Wait();
    }

    void park_worker() noexcept {
        semaphore_.park([this] { return shutdown_requested_.load(); });
    }

    void notify_work_available() noexcept {
        if (semaphore_.searchers_count() > 0) {
            work_maybe_available_.store(true);
            return;
        }

        if (semaphore_.parked_count() > 0) {
            semaphore_.notify_work_available();
        }
    }

    void shutdown() noexcept {
        shutdown_requested_.store(true);
        semaphore_.notify_all_workers();
    }

    bool is_shutdown_requested() const noexcept {
        return shutdown_requested_.load();
    }
};

}  // namespace wr::coord
