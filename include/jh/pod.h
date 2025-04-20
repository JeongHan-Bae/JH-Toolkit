/**
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file pod.h (POD system umbrella header)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Aggregated interface for defining and validating POD-like types.
 *
 * This header exposes the full `jh::pod` system:
 * - POD detection via `pod_like<T>` concept
 * - Verified POD struct macro `JH_POD_STRUCT(...)`
 * - Lightweight value types: `pair`, `array`, `optional`, `string_view`, `tuple`
 * - Bit-level POD flag sets via `bitflags<N>`
 *
 * ## Design Goals:
 * - Enforce raw-layout safety and zero-overhead memory access
 * - Enable compile-time validation for all types used in `pod_stack`, `runtime_arr`, etc.
 * - Replace STL types (`std::pair`, `std::optional`, `std::tuple`, etc.) with strict, layout-stable alternatives
 *
 * ## Included Types:
 * | Component           | Purpose                                         |
 * |---------------------|-------------------------------------------------|
 * | `pod_like<T>`       | Concept for POD compliance                      |
 * | `pair<T1, T2>`      | 2-field struct, pod version of std::pair        |
 * | `array<T, N>`       | Fixed buffer with POD enforcement               |
 * | `bitflags<N>`       | POD bitsets for fixed-width flag control        |
 * | `tuple<Ts...>`      | Transitional tuple replacement (2–8 fields)     |
 * | `bytes_view`        | Raw byte view + safe reinterpreting/cloning     |
 * | `span<T>`           | POD-only view into contiguous typed memory      |
 * | `string_view`       | POD-safe (char*, len) view for immutable text   |
 * | `optional<T>`       | POD-safe nullable value (no constructors)       |
 * | `JH_POD_STRUCT(...)`| Macro to define and assert layout safety        |
 * | `JH_ASSERT_POD_LIKE`| Manual compile-time validation                  |
 *
 * @note Prefer `#include <jh/pod>` over individual headers unless performing targeted SFINAE.
 *
 * @version 1.3.x
 * @date 2025
 */


#pragma once

#include "pods/pod_like.h"
#include "pods/pair.h"
#include "pods/array.h"
#include "pods/bits.h"
#include "pods/bytes_view.h"
#include "pods/span.h"
#include "pods/string_view.h"
#include "pods/tools.h"
#include "pods/optional.h"
