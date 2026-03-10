#include <twist/ed/std/atomic.hpp>
#include <vvv/list.hpp>

#include "../src/coordination/coordinator.hpp"
#include "../src/coordination/directive.hpp"
#include "../src/coordination/throttler.hpp"
#include "../src/exec/config/concept.hpp"
#include "../src/exec/config/config.hpp"
#include "../src/exec/executor.hpp"
#include "../src/queues/global/global_queue.hpp"
#include "../src/queues/local/loot.hpp"
#include "../src/queues/local/ring_buffer.hpp"
#include "../src/queues/local/shared_state.hpp"
#include "../src/queues/local/steal_handle.hpp"
#include "../src/queues/local/ws_queue.hpp"
#include "../src/tasks/concept.hpp"
#include "../src/utils/constants.hpp"
#include "../src/worker/worker.hpp"

using namespace wr;

void lsp_indexing_helper() {
    struct MockTask : vvv::IntrusiveListNode<MockTask> {
        void run() noexcept {}
    };

    queues::GlobalQueue<MockTask> gq;
    coord::Coordinator coordinator(4);
    // WsExecutor<MockTask> executor(4);
}

int main() {
    return 0;
}
