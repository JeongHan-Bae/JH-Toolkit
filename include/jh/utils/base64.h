/**
 * @file base64.h (utils)
 * @brief Backward-compatibility alias for <code>jh::serio::base64</code> utilities.
 *
 * @details
 * This header preserves compatibility for version 1.3.x projects that previously
 * referenced <code>jh::utils::base64::*</code> and <code>jh::utils::base64url::*</code>.
 *
 * As of version 1.3.5, all Base64-related functionality has been refactored and
 * relocated to the <code>jh::serio</code> module, under the <code>jh::serio::base64</code>
 * and <code>jh::serio::base64url</code> namespaces.
 *
 * This forwarding header is provided only for transitional support and will be removed
 * in version 1.4.0.
 *
 * @see jh::serio::base64
 * @see jh::serio::base64url
 *
 * @note Deprecated since 1.3.5 — scheduled for removal in 1.4.0.
 * @version 1.3.5 (deprecated)
 * @date 2025
 */

#pragma once

#include "jh/serialize_io/base64.h"

namespace jh::utils::base64 {
    using namespace jh::serio::base64;
} // namespace jh::utils::base64

namespace jh::utils::base64url {
    using namespace jh::serio::base64url;
} // namespace jh::utils::base64url
