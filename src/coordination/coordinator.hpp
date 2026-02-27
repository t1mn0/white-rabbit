#pragma once

#include "directive.hpp"
#include "throttler.hpp"

namespace wr::coord {

// Coordinator is a component that encapsulates the decision logic for theft:
// whether the Worker should continue searching for tasks,
// whether it needs to fall asleep immediately, or
// whether it is worth trying to retrieve tasks again (two-phase parking)

class Coordinator {
  public:  // nested types:
    using SearchPermit = Throttler::StealPermit;

  public:  // member functions:
    explicit Coordinator(size_t total_workers);

    // main method for requesting instructions by Worker
    [[nodiscard]] CoordDirective try_steal() noexcept;

    void park_worker() noexcept;

    void notify_worker() noexcept;

    void shutdown() noexcept;

    bool should_steal() noexcept;

    bool should_shutdown() const noexcept;


  private:  // data members:
    Throttler semaphore_;
    std::atomic<bool> shutdown_requested_ = false;
    std::atomic<bool> work_maybe_available_ = false;
};

}  // namespace wr::coord
