#include "throttler.hpp"
#include <utility>

namespace wr::coord {

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

Throttler::StealPermit::~StealPermit() {
    release();
}

Throttler::StealPermit::StealPermit(StealPermit&& oth) noexcept /* only moveable */
    : host_(std::exchange(oth.host_, nullptr)) {}

void Throttler::StealPermit::release() noexcept {
    if (host_) {
        host_->on_permit_released();
        host_ = nullptr;
    }
}

std::optional<Throttler::StealPermit> Throttler::try_acquire_permit() noexcept {
    // trying to give `SearchPermit` via CAS:
    size_t current_searchers_count_ = searchers_count_.load();
    while (current_searchers_count_ < max_searchers_count_) {
        if (searchers_count_.compare_exchange_weak(current_searchers_count_, current_searchers_count_ + 1)) {
            return Throttler::StealPermit(this);
        }
    }
    return std::nullopt;
}

template <WakeCondition Predicate>
void Throttler::park(Predicate&& stop_waiting) noexcept {
    // `stop_waiting` should be noexcept ^
    std::unique_lock<std::mutex> lock(wait_mutex_);
    parked_count_.fetch_add(1);

    work_available_.wait(lock, [this, &stop_waiting] {
        // Condition to wake:
        // 1. external reason (stop_waiting)
        // 2. or an internal work hint (work_hint)
        return stop_waiting() || work_hint_.load();
    });

    parked_count_.fetch_sub(1);
    work_hint_.store(false);
}

void Throttler::notify_work_available() noexcept {
    // If some Worker is in `Searching` state (`searchers_count_` > 0), we not wake the sleepers.
    // Just set the `work_maybe_available_ = true` flag.
    // `Searching` worker (which currently is in steal mode) is guaranteed to see this hot task.
    if (searchers_count_.load() > 0) {
        return;
    }
    // else (if every Worker sleep or busy): Coordinator notify one sleeping worker:
    if (parked_count_.load() > 0) {
        {
            std::lock_guard<std::mutex> lock(wait_mutex_);
            work_hint_.store(true);
        }
        work_available_.notify_one();
    }
}

void Throttler::notify_all_workers() noexcept {
    // for scheduler shutdowning process
    std::lock_guard lock(wait_mutex_);
    work_available_.notify_all();
}

size_t Throttler::searchers_count() const noexcept {
    ///
    return searchers_count_.load();
    ///
}

size_t Throttler::parked_count() const noexcept {
    ///
    return parked_count_.load();
    ///
}

void Throttler::on_permit_released() noexcept {
    ///
    searchers_count_.fetch_sub(1);  // just guarantee to no leaks in the semaphore
    ///
}

}  // namespace wr::coord
