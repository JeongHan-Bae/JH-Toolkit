/**
 * @file pod_concepts.h (conceptual)
 * @brief Aggregated header for POD-related concepts under <code>jh::concepts</code>.
 *
 * <p>
 * This header collects all POD-system concepts into the <code>jh::concepts</code> namespace.
 * </p>
 *
 * <pre><code>#include &lt;jh/conceptual/pod_concepts.h&gt;</code></pre>
 *
 * <p>
 * It includes and re-exports all concept definitions from <code>jh/pods/</code>,
 * ensuring that all POD-related type constraints can be accessed uniformly
 * through <code>jh::concepts</code>.
 * </p>
 *
 * <h4>Included Components</h4>
 * <ul>
 *   <li><code>pod_like</code>, <code>cv_free_pod_like</code></li>
 *   <li><code>max_pod_array_bytes</code>, <code>max_pod_bitflags_bytes</code></li>
 *   <li><code>trivial_bytes</code>, <code>linear_container</code></li>
 *   <li><code>streamable</code>, <code>streamable_pod</code></li>
 * </ul>
 *
 * <h4>Structure Policy</h4>
 * <p>
 * The <code>jh/pods/</code> directory defines the implementation-level primitives,
 * while <code>jh/conceptual/</code> provides their conceptual aggregation.
 * This header is included by both <code>&lt;jh/pod&gt;</code> and
 * <code>&lt;jh/concepts&gt;</code> to ensure consistency across layers.
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

    using jh::pod::streamable;
    using jh::pod::streamable_pod;
} // namespace jh::concepts
