/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
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
 * \endverbatim
 */
/**
 * @file span.h (pods)
 * @brief POD-safe minimal span for contiguous memory.
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Fully POD (<code>T* + uint64_t</code>)</li>
 *   <li>No dynamic allocation, no STL dependencies</li>
 *   <li>Iteration, slicing, and indexing support</li>
 *   <li>Suitable for arena allocators, mmap, and raw containers</li>
 * </ul>
 *
 * @note Unlike <code>std::span</code>, this type is limited to POD-compatible
 *       contiguous memory. It does not support iterator ranges, polymorphic
 *       containers, or allocator-aware semantics.
 * @note Lifetime of underlying memory must be managed externally.
 * @note Functions rely on <code>reinterpret_cast</code> and therefore cannot
 *       be used in <code>consteval</code> contexts.
 */

#pragma once

#include <cstdint>   // for uint64_t
#include <cstddef>
#include <concepts>  // NOLINT for std::convertible_to<>
#include <type_traits>
#include <utility>

#include "jh/pods/pod_like.h"
#include "jh/pods/pair.h"

namespace jh::pod {

    namespace detail {

        enum class data_status : uint8_t {
            none = 0,
            adl,
            field,
            method
        };

        enum class len_status : uint8_t {
            none = 0,
            adl,
            field,
            method
        };

        template<typename C>
        consteval data_status compute_data_status() noexcept {

            if constexpr (requires([[maybe_unused]] const C &c) {
                { get_data(c) } -> std::convertible_to<const void *>;
            }) {
                // --- ADL form ---
                using R = decltype(get_data(std::declval<const C &>()));
                if constexpr (std::is_pointer_v<std::decay_t<R>>)
                    return data_status::adl;
            } else if constexpr (requires([[maybe_unused]] const C &c) {
                // data exists as a member (not a function)
                requires (!std::is_member_function_pointer_v<decltype(&C::data)>);
                c.data;
            }) {
                // --- field form ---
                using R = decltype(std::declval<const C *>()->data);
                if constexpr (std::is_pointer_v<std::decay_t<R>>)
                    return data_status::field;
            } else if constexpr (requires([[maybe_unused]] const C &c) {
                { c.data() } -> std::convertible_to<const void *>;
            }) {
                // --- method form ---
                using R = decltype(std::declval<const C *>()->data());
                if constexpr (std::is_pointer_v<std::decay_t<R>>)
                    return data_status::method;
            }
            return data_status::none;
        }

        template<typename C>
        consteval len_status compute_len_status() noexcept {

            if constexpr (requires([[maybe_unused]] const C &c) {
                { get_size(c) } -> std::convertible_to<std::uint64_t>;
            }) {
                // ADL
                return len_status::adl;
            } else if constexpr (requires([[maybe_unused]] const C &c) {
                requires (!std::is_member_function_pointer_v<decltype(&C::len)>);
                { c.len } -> std::convertible_to<std::uint64_t>;
            }) {
                // field
                return len_status::field;
            } else if constexpr (requires([[maybe_unused]] const C &c) {
                { c.size() } -> std::convertible_to<std::uint64_t>;
            }) {
                // method
                return len_status::method;
            }
            return len_status::none;
        }

        template<typename C>
        consteval jh::pod::pair<data_status, len_status> compute_view_status() noexcept {
            return {compute_data_status<C>(), compute_len_status<C>()};
        }

        template<auto S, typename T>
        struct ref_type_helper {
            static constexpr auto get() {
                if constexpr (S.first == data_status::field) {
                    using ptr_t = decltype(&T::data);
                    static_assert(!std::is_member_function_pointer_v<ptr_t>);
                    return std::type_identity<decltype(std::declval<T *>()->data)>{};
                } else if constexpr (S.first == data_status::method) {
                    return std::type_identity<decltype(std::declval<T *>()->data())>{};
                } else {
                    return std::type_identity<decltype(get_data(*std::declval<T *>()))>{};
                }
            }
        };

        /**
         * @brief Precomputed linear-container classification result.
         *
         * This variable template evaluates the compile-time data/length access pattern
         * of a given container type <code>C</code> via <code>compute_view_status&lt;C&gt;()</code>,
         * and caches its pair of <code>data_status</code> / <code>len_status</code> for reuse.
         *
         * <p><b>ADL Priority:</b><br/>
         * Detection prioritizes <b>ADL (Argument-Dependent Lookup)</b> forms such as
         * <code>get_data()</code> or <code>get_size()</code> before checking direct fields
         * or member functions. This allows users to explicitly override default detection
         * when the internal representation is ambiguous or does not meet expectations.</p>
         *
         * <p><b>Exposure:</b><br/>
         * The result is exposed in <b>jh::concepts</b> as
         * <code>jh::concepts::linear_status&lt;C&gt;</code>, enabling higher-level modules
         * to query whether a type provides pointer-based and length-based accessors
         * without repeating introspection.</p>
         *
         * <p><b>Note:</b><br/>
         * The <b>precomputed</b> status is exposed instead of the function
         * <code>compute_view_status&lt;C&gt;()</code> to minimize compile-time overhead.
         * The constexpr evaluation runs once per type, and subsequent concept checks
         * reuse the cached value.</p>
         */
        template<typename C>
        constexpr jh::pod::pair<data_status, len_status> linear_status = compute_view_status<C>();

        /**
         * @brief Concept defining a <b>linear container</b>.
         *
         * A type satisfies this concept if its corresponding
         * <code>linear_status</code> indicates valid data and length access,
         * i.e. both <code>data_status</code> and <code>len_status</code> are non-<code>none</code>.
         *
         * <p><b>Exposure:</b><br/>
         * This concept is available through <b>jh::concepts</b>,
         * representing the canonical definition of a <b>linear container</b> in JH-Toolkit.</p>
         *
         * <p><b>Design Rationale:</b><br/>
         * The implementation resides in <code>jh/pods/</code> to keep the semantic core
         * lightweight and self-contained, avoiding complex mutual dependencies between
         * <code>jh/pods</code> (the base POD utilities) and higher-level <code>jh/concepts</code>.
         * <br/>This also allows <code>jh/pod</code> to remain a minimal independently
         * compilable module.</p>
         */
        template<typename C>
        concept linear_container = [] {
            constexpr auto status = linear_status<C>;
            return status.first != data_status::none && status.second != len_status::none;
        }();
    }

    /**
     * @brief Non-owning typed view over a contiguous memory block.
     *
     * Behaves like a stripped-down <code>std::span</code>, but remains fully POD
     * (<code>T* + uint64_t</code>).
     *
     * <h4>Differences from std::span:</h4>
     * <ul>
     *   <li>No bounds-checked <code>.at()</code></li>
     *   <li>No dynamic extent; always runtime-sized</li>
     *   <li>No interop with iterator-only containers</li>
     * </ul>
     *
     * <h4>Usage Model:</h4>
     * <ul>
     *   <li>Provides indexing (<code>operator[]</code>)</li>
     *   <li>Range iteration via <code>begin()</code>/<code>end()</code></li>
     *   <li>Slicing via <code>sub()</code>, <code>first()</code>, <code>last()</code></li>
     * </ul>
     *
     * @tparam T Element type. Must satisfy <code>pod_like</code>.
     */
    template<pod_like T>
    struct span final {
        T *data;           ///< @brief Pointer to the first element.
        std::uint64_t len; ///< @brief Number of elements.

        using element_type = T;                                   ///< @brief Element type (alias of <code>T</code>).
        using value_type = std::remove_cv_t<T>;                   ///< @brief Value type without const/volatile.
        using size_type = std::uint64_t;                          ///< @brief Size type (64-bit).
        using difference_type [[maybe_unused]] = std::ptrdiff_t;  ///< @brief Signed difference type.
        using reference = value_type &;                           ///< @brief Reference to element.
        using const_reference = const value_type &;               ///< @brief Const reference to element.
        using pointer = value_type *;                             ///< @brief Pointer to element.
        using const_pointer = const value_type *;                 ///< @brief Const pointer to element.

        /// @brief Access an element by index (no bounds check).
        constexpr const_reference operator[](std::uint64_t index) const noexcept {
            return data[index];
        }

        /// @brief Pointer to the first element.
        [[nodiscard]] constexpr const_pointer begin() const noexcept { return data; }

        /// @brief Pointer to one-past-the-end.
        [[nodiscard]] constexpr const_pointer end() const noexcept { return data + len; }

        /// @brief Number of elements in view.
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /// @brief Whether the view is empty.
        [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }

        /**
         * @brief Creates a sub-span from <code>offset</code>, with optional <code>count</code> elements.
         *
         * If <code>count == 0</code> (default), the view extends to end.
         * If <code>offset &gt; len</code>, returns empty span.
         */
        [[nodiscard]] constexpr span sub(const std::uint64_t offset,
                                         const std::uint64_t count = 0) const noexcept {
            if (offset > len) return {nullptr, 0};
            const std::uint64_t remaining = len - offset;
            const std::uint64_t real_len = (count == 0 || count > remaining) ? remaining : count;
            return {data + offset, real_len};
        }

        /// @brief Returns the first <code>count</code> elements as a new span.
        [[nodiscard]] constexpr span first(const std::uint64_t count) const noexcept {
            if (!count) return {nullptr, 0};
            return {data, (len > count ? count : len)};
        }

        /// @brief Returns the last <code>count</code> elements as a new span.
        [[nodiscard]] constexpr span last(const std::uint64_t count) const noexcept {
            if (!count) return {nullptr, 0};
            if (count >= len) return *this;
            return {data + len - count, count};
        }

        /**
         * @brief Equality comparison between two spans.
         *
         * Two spans are considered equal if they reference the <b>same sequence object</b>
         * (i.e. identical pointer and identical length).
         * This does <b>not</b> compare the element values.
         *
         * @note Implemented with <code>= default</code>, which checks <code>data</code> and <code>len</code>.
         * If you need value-wise comparison, use algorithms such as
         * <code>std::equal(lhs.begin(), lhs.end(), rhs.begin())</code>.
         */
        constexpr bool operator==(const span &rhs) const = default;
    };

    /// @brief Create span from a raw array (<code>T[N]</code>).
    template<typename T, std::uint64_t N>
    [[nodiscard]] constexpr span<T> to_span(T (&arr)[N]) noexcept {
        return {arr, static_cast<std::uint64_t>(N)};
    }

    /// @brief Create span from a const raw array (<code>const T[N]</code>).
    template<typename T, std::uint64_t N>
    [[nodiscard]] constexpr span<const T> to_span(const T (&arr)[N]) noexcept {
        return {arr, static_cast<std::uint64_t>(N)};
    }

    /**
     * @brief Creates a POD-compatible <code>span</code> from a linear container.
     *
     * <p>
     * This function constructs a <code>span&lt;T&gt;</code> that references the
     * contiguous data region of a container <code>C</code>. The access method
     * (field, member function, or ADL helper) is determined at compile time
     * according to the precomputed <code>linear_status&lt;C&gt;</code>.
     * </p>
     *
     * <h4>Design Rationale</h4>
     * <p>
     * Accessor detection follows a fixed precedence:
     * <br/><b>ADL</b> (<code>get_data()</code>, <code>get_size()</code>)
     * &rightarrow; <b>field</b> (<code>data</code>, <code>len</code>)
     * &rightarrow; <b>member function</b> (<code>data()</code>, <code>size()</code>).
     * <br/>
     * This allows users to explicitly provide ADL overloads to override internal
     * members when the container layout is not standard or may be misleading.
     * </p>
     *
     * @note
     * <ul>
     *   <li><code>data</code> and <code>data()</code> are mutually exclusive — defining both is undefined behavior.</li>
     *   <li><code>len</code> and <code>size()</code> may coexist, but they are expected to report identical lengths.</li>
     *   <li>If your container exposes inconsistent or unexpected accessors,
     *       you can define <code>my_ns::get_size(my_ns::MyContainer)</code>
     *       or <code>get_data()</code> via ADL to override detection.</li>
     *   <li>If even ADL overrides cannot match your intended semantics,
     *       do not use this helper — manually construct <code>span{ptr, len}</code> instead.</li>
     *   <li>This is a syntactic convenience, not a semantic requirement.</li>
     * </ul>
     *
     * <h4>Return Value</h4>
     * <p>
     * Returns a non-owning <code>span&lt;Elem&gt;</code> referencing the same
     * contiguous memory as the source container.
     * </p>
     */
    template<detail::linear_container C>
    [[nodiscard]] constexpr auto to_span(C &c) noexcept {
        constexpr auto status = jh::pod::detail::linear_status<C>;

        using Ref = typename decltype(detail::ref_type_helper<status, C>::get())::type;
        using Elem = std::remove_pointer_t<std::remove_reference_t<Ref>>;

        Elem *ptr{};
        std::uint64_t len{};

        if constexpr (status.first == jh::pod::detail::data_status::field)
            ptr = c.data;
        else if constexpr (status.first == jh::pod::detail::data_status::method)
            ptr = c.data();
        else if constexpr (status.first == jh::pod::detail::data_status::adl)
            ptr = get_data(c);

        if constexpr (status.second == jh::pod::detail::len_status::field)
            len = c.len;
        else if constexpr (status.second == jh::pod::detail::len_status::method)
            len = c.size();
        else if constexpr (status.second == jh::pod::detail::len_status::adl)
            len = get_size(c);

        return span<Elem>{ptr, len};
    }
} // namespace jh::pod
