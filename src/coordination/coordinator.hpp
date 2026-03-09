#pragma once

#include "directive.hpp"
#include "throttler.hpp"

namespace wr::coord {

/* Coordinator is a component that encapsulates the decision logic for stealer:
 * 1. whether the Worker should continue searching for tasks
 * 2. whether it needs to fall asleep immediately
 * 3. whether it is worth trying to retrieve tasks again (two-phase parking) */
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
    [[nodiscard]] Directive ask_to_steal() noexcept;

    void park_worker() noexcept;

    void notify_worker() noexcept;

    void shutdown() noexcept;

    bool is_should_shutdown() const noexcept;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

inline Coordinator::Coordinator(size_t total_workers)
    : semaphore_(total_workers > 1 ? total_workers / 2 : 1) {}

inline auto Coordinator::ask_to_steal() noexcept -> Directive {
    if (shutdown_requested_.load()) {
        return Directive::Terminate();
    }

    /* permission to steal */
    auto permittion = semaphore_.try_acquire_permit();

    if (permittion.has_value()) {
        return Directive::Steal(std::move(permittion.value()));
    }

    /* two-phased parking... */
    if (work_maybe_available_.exchange(false)) {
        return Directive::Retry();
    }

    return Directive::Park();
}

inline void Coordinator::park_worker() noexcept {
    semaphore_.park([this] {
        return shutdown_requested_.load();
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

inline bool Coordinator::is_should_shutdown() const noexcept {
    return shutdown_requested_.load();
}

}  // namespace wr::coord
