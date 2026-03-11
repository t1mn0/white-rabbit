#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/thread.hpp>

#include <cassert>
#include <iostream>

void FaultInjectionSmoke() {
    twist::ed::std::atomic<int> value{0};

    twist::ed::std::thread t([&] {
        value.store(42, std::memory_order_release);
    });

    t.join();

    assert(value.load(std::memory_order_acquire) == 42);
}

int main() {
    std::cout << "[fault_injection] smoke test..." << std::endl;

    for (int i = 0; i < 100; ++i) {
        FaultInjectionSmoke();
    }

    std::cout << "[fault_injection] PASSED!" << std::endl;
    return 0;
}
