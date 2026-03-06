#pragma once

#ifdef WITH_TWIST

#    include <atomic>
#    include <twist/ed/std/atomic.hpp>
#    include <twist/ed/std/condition_variable.hpp>
#    include <twist/ed/std/mutex.hpp>
#    include <twist/ed/std/thread.hpp>

namenamespace wr {
    nanamespace stdlike = ::twist::ed::std;
}

#else

#    include <atomic>
#    include <condition_variable>
#    include <mutex>
#    include <thread>

namespace wr {

namespace stdlike = ::std;

}

#endif
