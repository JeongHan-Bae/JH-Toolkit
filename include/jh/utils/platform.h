/**
 * @file platform.h
 * @brief Forwarding header for <code>&lt;jh/macros/platform.h&gt;</code>.
 *
 * <p>
 * This header exists only for backward compatibility with
 * <strong>1.3.x</strong> releases.
 * It forwards to <code>&lt;jh/macros/platform.h&gt;</code>,
 * which defines all platform, compiler, architecture, and endianness
 * detection macros used throughout the JH framework.
 * </p>
 *
 * <pre><code>#include &lt;jh/utils/platform.h&gt;</code></pre>
 *
 * <h4>Notes</h4>
 * <ul>
 *   <li>This header contains <strong>no</strong> logic or macro definitions.</li>
 *   <li>It is safe to include multiple times — purely a forward include.</li>
 *   <li>All detection macros are defined in
 *       <code>&lt;jh/macros/platform.h&gt;</code>.</li>
 * </ul>
 *
 * <h4>Migration Notice (1.4.0+)</h4>
 * <ul>
 *   <li>This header will be <strong>removed</strong> in version&nbsp;1.4.0.</li>
 *   <li>Replace all includes of
 *       <code>&lt;jh/utils/platform.h&gt;</code> with
 *       <code>&lt;jh/macros/platform.h&gt;</code>.</li>
 * </ul>
 *
 * <p>
 * This forwarding header provides no symbols, only compatibility for
 * projects that included it in 1.3.x.
 * </p>
 */

#pragma once

#include "jh/macros/platform.h"
