/**
 * @file example_pod_writer.cpp
 * @brief Worker process that performs randomized but deterministic updates on a shared POD object.
 */

#include "jh/synchronous/ipc/shared_process_memory.h"
#include "jh/pod"
#include <cmath>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>

// -----------------------------------------------------------------------------
// Define shared POD type
// -----------------------------------------------------------------------------
JH_POD_STRUCT(DemoPod,
    std::uint64_t xor_field;
    std::uint64_t add_field;
    double        mul_field;
);

// -----------------------------------------------------------------------------
// Shared memory binding
// -----------------------------------------------------------------------------
using shm_t = jh::sync::ipc::shared_process_memory<"demo_shared_pod", DemoPod>;

// -----------------------------------------------------------------------------
// Main worker logic
// -----------------------------------------------------------------------------
int main() {
    auto &shm = shm_t::instance();

    constexpr std::uint64_t xor_mask   = 0xA5A5A5A5A5A5A5A5ULL;
    constexpr std::uint64_t add_inc    = 10;
    constexpr double mul_factor        = 1.0001;
    constexpr int iterations           = 20'000;

    // Define operation types
    enum class OpType { Xor, Add, Mul };

    // Build operation list: 2N XORs, N ADDs, N MULs
    std::vector<OpType> ops;
    ops.reserve(4 * iterations);
    for (int i = 0; i < iterations; ++i) {
        ops.push_back(OpType::Add);
        ops.push_back(OpType::Mul);
        ops.push_back(OpType::Xor);
        ops.push_back(OpType::Xor); // even times → invariant
    }

    // Randomize operation order
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(ops.begin(), ops.end(), rng);

    // Execute randomized sequence
    for (OpType op : ops) {
        std::lock_guard guard(shm.lock());
        shm.flush_acquire();
        auto &ref = shm.ref();

        switch (op) {
            case OpType::Xor:
                ref.xor_field ^= xor_mask;
                break;
            case OpType::Add:
                ref.add_field += add_inc;
                break;
            case OpType::Mul:
                ref.mul_field *= mul_factor;
                break;
        }

        // Full seq fence ensures visibility across all processes
        shm.flush_seq();
    }

    return 0;
}
