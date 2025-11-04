/**
 * @file hash_fn.h (utils)
 * @brief Backward-compatibility alias for <code>jh::meta::hash</code> utilities.
 *
 * @details
 * This header preserves compatibility for version 1.3.x projects that previously
 * referenced <code>jh::utils::hash_fn::*</code> .
 * All functionality has been moved to the <code>jh::meta::hash</code> namespace.
 *
 * @see jh::meta::hash
 *
 * @note Deprecated since 1.3.x — scheduled for removal in 1.4.0.
 * @version 1.3.x (deprecated)
 * @date 2025
 */

#pragma once

#include "jh/meta/hash.h"

namespace jh::utils::hash_fn {
    using jh::meta::c_hash;
    using jh::meta::fnv1a64;
    using jh::meta::fnv1_64;
    using jh::meta::djb2;
    using jh::meta::sdbm;
    using jh::meta::hash;
}
