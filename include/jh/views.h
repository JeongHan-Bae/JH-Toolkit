/**
 * @file views.h
 * @brief Aggregator header for <code>jh::views</code> (1.3.x only).
 *
 * <p>
 * This header aggregates all standard view adaptors under
 * the <code>jh::views</code> namespace.
 * </p>
 *
 * <pre><code>#include &lt;jh/views.h&gt;</code></pre>
 *
 * <p>
 * Internally, it includes the underlying adaptor implementations from
 * <code>jh/ranges/views/</code> and exposes them collectively through
 * <code>jh::views</code>.
 * </p>
 *
 * <h4>Overview</h4>
 * <ul>
 *   <li>Defines <code>jh::views</code> as the user-facing namespace.</li>
 *   <li>Aggregates all adaptors (e.g. <code>zip</code>, <code>enumerate</code>).</li>
 *   <li>Provides a stable include path for all <strong>1.3.x</strong> releases.</li>
 * </ul>
 *
 * <h4>Removal Notice (1.4.0+)</h4>
 * <ul>
 *   <li>This header will be <strong>removed</strong> starting from version&nbsp;1.4.0.</li>
 *   <li>Use <code>#include &lt;jh/views&gt;</code> instead — it is the
 *       <strong>canonical and permanent</strong> include form.</li>
 *   <li>All adaptor implementations will remain located in
 *       <code>jh/ranges/views/&#42;.h</code>.</li>
 * </ul>
 *
 * <p>
 * This file exists solely to maintain compatibility with
 * <strong>1.3.x</strong> code and will not appear in future releases.
 * </p>
 */

#pragma once

#include "jh/ranges/views/common.h"
#include "jh/ranges/views/enumerate.h"
#include "jh/ranges/views/flatten.h"
#include "jh/ranges/views/transform.h"
#include "jh/ranges/views/vis_transform.h"
#include "jh/ranges/views/zip.h"

namespace jh::views {
    using namespace jh::ranges::views;
}