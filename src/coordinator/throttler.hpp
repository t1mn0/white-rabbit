#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <utility>

#include "wake_condition.hpp"

namespace wr::coord {

// Throttler stores information about the number of workers in the active steal phase - `searchers_count_`
// and the number of workers which are parked - `parked_count_`. It works on the principle of TaggedSemaphore,
// limiting the number of thieves-workers in the WS Scheduler:

class Throttler /* Semaphore */ {
  public:  // nested types:
    class SearchPermit /* is linear type and RAII-wrapper for semaphore counter unit */ {
      private:  // data members:
        Throttler* host_ = nullptr;

      public:  // friendship declaration:
        friend class Throttler;

      public:  // member functions:
        ~SearchPermit() {
            release();
        }

        SearchPermit(SearchPermit&& oth) noexcept /* only moveable */
            : host_(std::exchange(oth.host_, nullptr)) {}

        SearchPermit(const SearchPermit&) = delete;             // non copyable
        SearchPermit& operator=(const SearchPermit&) = delete;  // non copyassignable
        SearchPermit& operator=(SearchPermit&&) = delete;       // non moveassignable

        void release() noexcept {
            if (host_) {
                host_->on_permit_released();
                host_ = nullptr;
            }
        }

      private:  // member functions:
        explicit SearchPermit(Throttler* host) : host_(host) {}
    };

  private:  // data members:
    const size_t max_searchers_count_;
    std::atomic<size_t> searchers_count_ = 0;
    std::atomic<size_t> parked_count_ = 0;

    std::mutex wait_mutex_;
    std::condition_variable work_available_;
    std::atomic<bool> work_hint_ = false;

  public:  // member functions:
    explicit Throttler(size_t max_searchers) : max_searchers_count_(max_searchers) {}

    [[nodiscard]] std::optional<SearchPermit> try_acquire_permit() noexcept {
        // trying to give `SearchPermit` via CAS:
        size_t current_searchers_count_ = searchers_count_.load();
        while (current_searchers_count_ < max_searchers_count_) {
            if (searchers_count_.compare_exchange_weak(current_searchers_count_, current_searchers_count_ + 1)) {
                return SearchPermit(this);
            }
        }
        return std::nullopt;
    }

    template <WakeCondition Predicate>
    void park(Predicate&& stop_waiting) noexcept {
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

    void notify_work_available() noexcept {
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

    void notify_all_workers() noexcept {
        // for scheduler shutdowning process
        std::lock_guard lock(wait_mutex_);
        work_available_.notify_all();
    }

    size_t searchers_count() const noexcept { return searchers_count_.load(); }

    size_t parked_count() const noexcept { return parked_count_.load(); }

  private:  // member functions:
    void on_permit_released() noexcept {
        searchers_count_.fetch_sub(1);  // just guarantee to no leaks in the semaphore
    }
};

}  // namespace wr::coord
