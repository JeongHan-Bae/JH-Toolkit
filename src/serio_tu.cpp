/**
 * @file serio_tu.cpp
 * @brief Compilation unit for <code>jh::serio</code>.
 *
 * <p>
 * This file ensures that internal non-inline functions of
 * <code>jh::serio</code> are compiled for the static build
 * (<code>jh-toolkit-static</code>).
 * </p>
 *
 * @see jh::serio
 */

#define JH_HEADER_IMPL_BUILD

#include "jh/serialize_io/base64.h"
#include "jh/serialize_io/uri.h"
