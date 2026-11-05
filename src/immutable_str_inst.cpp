/**
 * @file immutable_str_inst.cpp
 * @brief Explicit instantiations of both <code>jh::immutable_str</code> policies.
 *
 * <p>
 * This file instantiates the <code>true</code> and <code>false</code> policies
 * so their members are available in the static build
 * (<code>jh-toolkit-static</code>). Including the dual-mode header also emits
 * the two shared, non-template initialization helpers once.
 * </p>
 *
 * @see jh::immutable_str
 */

#define JH_HEADER_IMPL_BUILD

#include "jh/core/immutable_str.h"

template class jh::immutable_str<true>;
template class jh::immutable_str<false>;
