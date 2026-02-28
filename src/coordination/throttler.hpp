#pragma once

#include "wake_condition.hpp"
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <optional>

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

}  // namespace wr::coord
