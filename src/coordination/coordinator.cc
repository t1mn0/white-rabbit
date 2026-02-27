#include "coordinator.hpp"
#include "directive.hpp"

namespace wr::coord {

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

Coordinator::Coordinator(size_t total_workers)
    : semaphore_(total_workers > 1 ? total_workers / 2 : 1) {}

CoordDirective Coordinator::try_steal() noexcept {
    if (shutdown_requested_.load()) {
        return CoordDirective::Terminate();
    }

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

void Coordinator::park_worker() noexcept {
    ///
    semaphore_.park([this] { return shutdown_requested_.load(); });
    ///
}

void Coordinator::notify_worker() noexcept {
    if (semaphore_.searchers_count() > 0) {
        work_maybe_available_.store(true);
        return;
    }

    if (semaphore_.parked_count() > 0) {
        semaphore_.notify_work_available();
    }
}

void Coordinator::shutdown() noexcept {
    shutdown_requested_.store(true);
    semaphore_.notify_all_workers();
}

bool Coordinator::should_shutdown() const noexcept {
    ///
    return shutdown_requested_.load();
    ///
}

}  // namespace wr::coord
