/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file hashable.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief Unified hashing concept suite with semantic extension and ADL support.
 *
 * @details
 * This header defines the <b>duck-typed hashability model</b> used by the JH framework.
 * It generalizes hashing semantics beyond <code>std::hash&lt;T&gt;</code> by allowing
 * user-defined and semantically meaningful hash strategies.
 *
 * <h3>Why Allow Custom Hashing?</h3>
 * <p>
 * While <code>std::hash&lt;T&gt;</code> provides a universal entry point for associative
 * containers, it cannot describe semantic-specific hash behaviors such as:
 * </p>
 * <ul>
 *   <li><b>Lazy evaluation</b> &mdash; deferred hash computation with caching (e.g. <code>jh::immutable_str</code>).</li>
 *   <li><b>Algorithm selection</b> &mdash; e.g. <code>jh::pod::string_view</code> uses
 *       <code>c_hash::fnv1a64</code> by default, but may switch algorithms via an enum parameter.</li>
 *   <li><b>Semantic integrity</b> &mdash; certain domain-specific hashes carry meaning beyond raw bytes
 *       (e.g. type-level discriminators, persistent keys, etc.).</li>
 * </ul>
 *
 * <p>
 * Therefore, the JH framework permits three resolution layers:
 * </p>
 * <ol>
 *   <li><b>Standard hash</b> &mdash; <code>std::hash&lt;T&gt;{}</code> (highest precedence)</li>
 *   <li><b>ADL-discovered free hash()</b> &mdash; found via argument-dependent lookup</li>
 *   <li><b>Member function hash()</b> &mdash; <code>t.hash()</code> if provided</li>
 * </ol>
 *
 * <p>
 * This priority chain ensures full interoperability with the STL, while supporting
 * domain-specific customization and lazy semantics without breaking standard containers.
 * </p>
 *
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace jh::concepts {

    /**
     * @brief Checks whether <code>std::hash&lt;T&gt;</code> is valid and callable.
     * @details
     * Equivalent to:
     * @code
     * { std::hash&lt;T&gt;{}(v) } -> std::convertible_to<size_t>;
     * @endcode
     */
    template <typename T>
    concept has_std_hash = requires(const T& v) {
        { std::hash<T>{}(v) } -> std::convertible_to<size_t>;
    };

    /**
     * @brief Checks whether a free (ADL-discoverable) <code>hash()</code> function exists.
     * @details
     * This allows user-defined global hash functions to participate in resolution
     * without specializing <code>std::hash</code> or modifying the type.
     * @code
     * size_t hash(const MyType& t);
     * @endcode
     */
    template <typename T>
    concept has_adl_hash = requires(const T& v) {
        { hash(v) } -> std::convertible_to<size_t>;
    };

    /**
     * @brief Checks whether a type defines a <code>hash()</code> member function.
     * @details
     * Equivalent to:
     * @code
     * { v.hash() } -> std::convertible_to<size_t>;
     * @endcode
     */
    template <typename T>
    concept has_mbr_hash = requires(const T& v) {
        { v.hash() } -> std::convertible_to<size_t>;
    };

    /**
     * @brief Concept for types that can be hashed through any supported mechanism.
     *
     * @details
     * A type <code>T</code> satisfies <code>extended_hashable</code> if it supports at least one
     * of the following:
     * <ol>
     *   <li><code>std::hash&lt;T&gt;</code> specialization</li>
     *   <li>ADL-discoverable <code>hash()</code> free function</li>
     *   <li>Member function <code>T::hash()</code></li>
     * </ol>
     *
     * <p>
     * The precedence follows:
     * </p>
     * <pre>
     * std::hash&lt;T&gt;{}(t) &gt; hash(t) &gt; t.hash()
     * </pre>
     *
     * <p>
     * This ensures that standard hash behavior dominates if explicitly defined,
     * while still enabling semantic extensions and user customization.
     * </p>
     */
    template <typename T>
    concept extended_hashable =
    has_std_hash<T> || has_adl_hash<T> || has_mbr_hash<T>;

} // namespace jh::concepts


namespace jh {

    /**
     * @brief Behaviorally deduced hash functor.
     *
     * @details
     * Implements a unified hashing strategy consistent with
     * <code>jh::concepts::extended_hashable</code>.
     *
     * <h4>Resolution Order:</h4>
     * <ol>
     *   <li><b>std::hash&lt;T&gt;</b> if available.</li>
     *   <li><b>ADL-discovered</b> free <code>hash(t)</code>.</li>
     *   <li><b>Member function</b> <code>t.hash()</code>.</li>
     * </ol>
     */
    template <typename T, typename = void>
    struct hash;

    /// @brief Case 1: std::hash<T> is valid.
    template <typename T>
    struct hash<T, std::enable_if_t<jh::concepts::has_std_hash<T>>> {
        constexpr size_t operator()(const T& v) const noexcept {
            return std::hash<T>{}(v);
        }
    };

    /// @brief Case 2: ADL-discovered hash(T)
    template <typename T>
    struct hash<T, std::enable_if_t<!jh::concepts::has_std_hash<T> && jh::concepts::has_adl_hash<T>>> {
        constexpr size_t operator()(const T& v) const noexcept(noexcept(hash(v))) {
            return static_cast<size_t>(hash(v));
        }
    };

    /// @brief Case 3: Member hash()
    template <typename T>
    struct hash<T, std::enable_if_t<!jh::concepts::has_std_hash<T> &&
                                    !jh::concepts::has_adl_hash<T> &&
                                    jh::concepts::has_mbr_hash<T>>> {
        constexpr size_t operator()(const T& v) const noexcept(noexcept(v.hash())) {
            return static_cast<size_t>(v.hash());
        }
    };

} // namespace jh
