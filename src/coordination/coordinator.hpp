#pragma once

#include "directive.hpp"
#include "throttler.hpp"

namespace wr::coord {

/* Coordinator is a component that encapsulates the decision logic for theft: whether the Worker
 * should continue searching for tasks, whether it needs to fall asleep immediately, or whether it
 * is worth trying to retrieve tasks again (two-phase parking) */

class Coordinator {
  public:  // nested types:
    using SearchPermit = Throttler::StealPermit;

  private:  // data members:
    Throttler semaphore_;
    std::atomic<bool> shutdown_requested_ = false;
    std::atomic<bool> work_maybe_available_ = false;

  public:  // member functions:
    explicit Coordinator(size_t total_workers);

    // main method for requesting instructions by Worker
    [[nodiscard]] CoordDirective try_steal() noexcept;

    void park_worker() noexcept;

    void notify_worker() noexcept;

    void shutdown() noexcept;

    bool should_steal() noexcept;

    bool should_shutdown() const noexcept;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

inline Coordinator::Coordinator(size_t total_workers)
    : semaphore_(total_workers > 1 ? total_workers / 2 : 1) {}

inline auto Coordinator::try_steal() noexcept -> CoordDirective {
    if (shutdown_requested_.load()) {
        return CoordDirective::Terminate();
    }

    /* permission to steal */
    auto permittion = semaphore_.try_acquire_permit();

    if (permittion.has_value()) {
        return CoordDirective::Steal(std::move(permittion.value()));
    }

    /* two-phased parking... */
    if (work_maybe_available_.load()) {
        return CoordDirective::Retry();
    }

    return CoordDirective::Park();
}

inline void Coordinator::park_worker() noexcept {
    semaphore_.park([this] {
        ///
        return shutdown_requested_.load();
        ///
    });
}

inline void Coordinator::notify_worker() noexcept {
    if (semaphore_.searchers_count() > 0) {
        work_maybe_available_.store(true);
        return;
    }

    if (semaphore_.parked_count() > 0) {
        semaphore_.notify_work_available();
    }
}

inline void Coordinator::shutdown() noexcept {
    shutdown_requested_.store(true);
    semaphore_.notify_all_workers();
}

inline bool Coordinator::should_shutdown() const noexcept {
    ///
    return shutdown_requested_.load();
    ///
}

}  // namespace wr::coord
