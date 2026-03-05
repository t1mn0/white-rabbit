#pragma once

#include "wake_condition.hpp"
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <utility>

namespace wr::coord {

// Throttler is a component that stores information about the number of workers in the active steal phase
// and the number of workers which are parked. It works on the principle of TaggedSemaphore,
// limiting the number of thieves-workers in the WS Scheduler:

class Throttler /* Semaphore */ {
  public:  // nested types:
    class StealPermit /* is a linear type and RAII-wrapper for semaphore counter unit */ {
      private:  // data members:
        Throttler* host_ = nullptr;

      public:  // friendship declaration:
        friend class Throttler;

      public:  // member functions:
        ~StealPermit();

        StealPermit(StealPermit&& oth) noexcept; /* only moveable */

        StealPermit(const StealPermit&) = delete;             // non copyable
        StealPermit& operator=(const StealPermit&) = delete;  // non copyassignable
        StealPermit& operator=(StealPermit&&) = delete;       // non moveassignable

        void release() noexcept;

      private:  // member functions:
        explicit StealPermit(Throttler* host) : host_(host) {}
    };

  public:  // member functions:
    explicit Throttler(size_t max_searchers) : max_searchers_count_(max_searchers) {}

    [[nodiscard]] std::optional<StealPermit> try_acquire_permit() noexcept;

    template <WakeCondition Predicate>
    void park(Predicate&& stop_waiting) noexcept;

    void notify_work_available() noexcept;

    void notify_all_workers() noexcept;

    size_t searchers_count() const noexcept;

    size_t parked_count() const noexcept;

  private:  // member functions:
    void on_permit_released() noexcept;

  private:  // data members:
    const size_t max_searchers_count_;
    std::atomic<size_t> searchers_count_ = 0;
    std::atomic<size_t> parked_count_ = 0;

    std::mutex wait_mutex_;
    std::condition_variable work_available_;
    std::atomic<bool> work_hint_ = false;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

inline Throttler::StealPermit::~StealPermit() {
    ///
    release();
    ///
}

inline Throttler::StealPermit::StealPermit(StealPermit&& oth) noexcept /* only moveable */
    : host_(std::exchange(oth.host_, nullptr)) {}

inline void Throttler::StealPermit::release() noexcept {
    if (host_) {
        host_->on_permit_released();
        host_ = nullptr;
    }
}

inline std::optional<Throttler::StealPermit> Throttler::try_acquire_permit() noexcept {
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

inline void Throttler::notify_work_available() noexcept {
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

inline void Throttler::notify_all_workers() noexcept {
    // for scheduler shutdowning process
    std::lock_guard lock(wait_mutex_);
    work_available_.notify_all();
}

inline size_t Throttler::searchers_count() const noexcept {
    ///
    return searchers_count_.load();
    ///
}

inline size_t Throttler::parked_count() const noexcept {
    ///
    return parked_count_.load();
    ///
}

inline void Throttler::on_permit_released() noexcept {
    ///
    searchers_count_.fetch_sub(1);  // just guarantee to no leaks in the semaphore
    ///
}

}  // namespace wr::coord
