/**
 * @file pod_concepts.h (conceptual)
 * @brief Aggregated header for POD-related concepts under <code>jh::concepts</code>.
 *
 * <p>
 * This header serves as the conceptual aggregation layer for all POD-system
 * constraints and traits. It collects and re-exports concept definitions
 * declared in <code>jh/pods/</code> into the unified namespace
 * <code>jh::concepts</code>.
 * </p>
 *
 * <pre><code>#include &lt;jh/conceptual/pod_concepts.h&gt;</code></pre>
 *
 * <h4>Design Rationale</h4>
 * <p>
 * Provides a lightweight conceptual interface for POD-related type traits
 * without pulling in implementation details. The dependency direction
 * remains one-way: <code>jh/pods</code> defines behavior, and
 * <code>jh/conceptual</code> exposes corresponding concepts.
 * </p>
 *
 * <h4>Included Components</h4>
 * <ul>
 *   <li><code>pod_like</code>, <code>cv_free_pod_like</code></li>
 *   <li><code>max_pod_array_bytes</code>, <code>max_pod_bitflags_bytes</code></li>
 *   <li><code>trivial_bytes</code>, <code>linear_container</code>, <code>linear_status</code></li>
 *   <li><code>streamable</code>, <code>streamable_pod</code></li>
 * </ul>
 *
 * <h4>Structure Policy</h4>
 * <p>
 * The conceptual layer depends on <code>jh/pods/</code> but not the reverse.
 * This ensures <code>jh/pod</code> remains independently compilable
 * while maintaining a clear semantic boundary between implementation
 * and compile-time interfaces.
 * </p>
 */

#pragma once

#include "jh/pods/pod_like.h"
#include "jh/pods/array.h"
#include "jh/pods/bits.h"
#include "jh/pods/bytes_view.h"
#include "jh/pods/span.h"
#include "jh/pods/stringify.h"

namespace jh::concepts {
    using jh::pod::pod_like;
    using jh::pod::cv_free_pod_like;

    using jh::pod::max_pod_array_bytes;
    using jh::pod::max_pod_bitflags_bytes;

    using jh::pod::trivial_bytes;
    using jh::pod::detail::linear_container;
    using jh::pod::detail::linear_status;

    using jh::pod::streamable;
    using jh::pod::streamable_pod;
} // namespace jh::concepts
