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
 * @file variant_adt.h (metax)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief ADT utilities for <code>std::variant</code> &mdash; compile-time checks and transformations.
 *
 * <h3>Design Goals</h3>
 * <p>
 * Abstract Data Type (ADT) is better than inheriting.<br>
 * <em><b>MCPP Philosophy</b>: Composition over inheritance, separating data from objects.</em>
 * </p>
 * <p>
 * <b>About using <code>std::variant</code> as a closed, memory-friendly alternative to inheritance:</b>
 * <ol>
 *  <li>Memory locality & predictable storage</li>
 *  <li>Closed set of types (closed world assumption)</li>
 *  <li>No vtable / no RTTI dependency</li>
 *  <li>A modern and safer C++20 pattern</li>
 * </ol>
 * </p>
 * <p>
 * The ADT-based checking framework leverages these benefits by providing a
 * generic way to apply compile-time conditions to every type inside a <code>std::variant</code>.
 * </p>
 * <h3>Design Notes</h3>
 * <p>
 * A type-transform applied to a variant must produce either:
 * <ul>
 * <li>a <b>uniform</b> result type for all alternatives (e.g., a hash &rarr; <code>size_t</code>),
 *    in which case the variant can be collapsed to a single type</li>
 * <li>a set of <b>pairwise distinct</b> result types,
 *   in which case the variant can be transformed into another variant</li>
 * </ul>
 * Mixed outcomes (some alternatives mapping to identical types while others do not)
 * are considered invalid in real-world business models. Such a design indicates
 * inconsistent semantics and should be rejected by static checks (e.g. via <code>check_all</code>).
 * Sometimes you might need mappings like:
 * <pre>
 * A  -> TA;
 * BA -> TB;
 * BB -> TB;
 * BC -> TB;
 * C  -> TC;
 * D  -> TD;
 * </pre>
 * </p><p>
 * But note that <tt>BA</tt>, <tt>BB</tt>, and <tt>BC</tt> are actually of the same family from the outermost perspective.
 * You should use <code>std::variant&lt;A, std::variant&lt;BA, BB, BC&gt;, C, D&gt;</code> instead of
 * <code>std::variant&lt;A, BA, BB, BC, C, D&gt;</code>.
 * Here we actually notice that the outer layer is <tt>ALL-to-ALL</tt> inflexion, but <tt>family B</tt> undergoes collapse.
 * </p><p>
 * This can be easily achieved using <code>using VB = std::variant&lt;BA, BB, BC&gt;</code>
 * and then applying <code>variant_collapse_t&lt;VB, your_transform&gt;</code> inside the outer variant transform
 * <code>Transformer&lt;VB&gt{...}</code> , (aligned with<code>Transformer&lt;A&gt</code>,
 * <code>Transformer&lt;C&gt</code>, <code>Transformer&lt;D&gt</code>).
 * </p>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstddef>
#include <variant>

namespace jh::meta {

    namespace detail {
        /**
         * @brief Helper to check all alternatives in a variant with a wide Check struct.
         *
         * @note
         * Variant itself is forced as the second template parameter of Check.
         * This allows Check to access the full variant type if needed without declaring it
         * again in the user-facing interface.<br>
         * We would avoid redundant declaration like:
         * <code>jh::meta::check_all&lt;some_check, V, V&gt;</code>
         * and instead use:
         * <code>jh::meta::check_all&lt;some_check, V&gt;</code>.
         */
        template<
                template<typename, typename, typename...> typename Check,
                typename Variant,
                std::size_t I,
                typename... Args
        >
        constexpr bool check_all_wide_impl() {
            if constexpr (I >= std::variant_size_v<Variant>) {
                return true;
            } else {
                if constexpr (requires {
                    Check<typename std::variant_alternative_t<I, Variant>, Variant, Args...>::value;
                } && std::is_convertible_v<
                        decltype(Check<typename std::variant_alternative_t<I, Variant>, Variant, Args...>::value),
                bool >) {
                    if constexpr (!Check<
                            typename std::variant_alternative_t<I, Variant>,
                            Variant,
                            Args...
                    >::value) {
                        return false;
                    } else {
                        return check_all_wide_impl<Check, Variant, I + 1, Args...>();
                    }
                }
                else {
                    // Invalid Check struct: missing 'value' or not convertible to bool
                    return false;
                }
            }
        }

        /// @brief Helper to check all alternatives in a variant with a narrow Check struct.
        template<
                template<typename> typename Check,
                typename Variant,
                std::size_t I
        >
        constexpr bool check_all_narrow_impl() {
            if constexpr (I >= std::variant_size_v<Variant>) {
                return true;
            } else {
                if constexpr (requires {
                    Check<typename std::variant_alternative_t<I, Variant>>::value;
                } && std::is_convertible_v<
                        decltype(Check<typename std::variant_alternative_t<I, Variant>>::value),
                bool >) {
                    if constexpr (!Check<
                            typename std::variant_alternative_t<I, Variant>
                    >::value) {
                        return false;
                    } else {
                        return check_all_narrow_impl<Check, Variant, I + 1>();
                    }
                }
                else {
                    // Invalid Check struct: missing 'value' or not convertible to bool
                    return false;
                }
            }
        }

        /// @brief Deduce transformed type at index I in Variant using TpTrans
        template<std::size_t I, typename Variant, template<typename> typename TpTrans>
        struct deduce_type final {
            using type =
                    typename TpTrans<std::variant_alternative_t<I, Variant>>::type;
        };

        template<typename Variant, template<typename> typename TpTrans>
        struct variant_transform_impl;

        /// @brief Transform all types in Variant using TpTrans
        template<template<typename...> typename V, typename... Ts,
                template<typename> typename TpTrans>
        struct variant_transform_impl<V<Ts...>, TpTrans> final {
            using type [[maybe_unused]] = std::variant<typename TpTrans<Ts>::type...>;
        };

        // Extract one collapsed type
        template<typename T, template<typename> typename TpTrans, typename = void>
        struct collapse_one final {
            using type = void;               // SFINAE failure -> void
        };

        /// @brief Specialization when TpTrans<T>::type exists
        template<typename T, template<typename> typename TpTrans>
        struct collapse_one<T, TpTrans, std::void_t<typename TpTrans<T>::type>> final {
            using type = typename TpTrans<T>::type;
        };

        // Check if all types in a variant collapse to the same thing
        template<typename Variant, template<typename> typename TpTrans>
        struct variant_collapse_impl;

        /// @brief Specialization for variant types
        template<template<typename...> typename V, typename... Ts,
                template<typename> typename TpTrans>
        struct variant_collapse_impl<V<Ts...>, TpTrans> final {

            // The collapsed results of each alternative
            using first = typename collapse_one<
                    std::tuple_element_t<0, std::tuple<Ts...>>,
                    TpTrans
            >::type;

            template<typename T>
            using collapsed_t = typename collapse_one<T, TpTrans>::type;

            static constexpr bool all_same = (
                    std::conjunction_v<
                            std::bool_constant<std::is_same_v<collapsed_t<Ts>, first>>...
                    >
            );

            using type = std::conditional_t<all_same, first, void>;
        };

        /// @brief Checker to validate TpTrans produces non-void types
        template<template<typename> typename TpTrans>
        struct is_valid_trans_holder final {
            template<typename T>
            struct apply final {
                static constexpr bool value = requires {
                    typename TpTrans<T>::type;
                    requires (!std::is_void_v<typename TpTrans<T>::type>);
                };
            };
        };

        /// @brief Auto detection of wide vs narrow Check struct
        template<template<typename...> typename Check, typename Variant, typename... Args>
        consteval bool check_all_impl() {
            if constexpr (std::variant_size_v<Variant> == 0) {
                return true;
            } else if constexpr (requires {
                typename Check<std::variant_alternative_t<0, Variant>>;
                requires requires{ Check<std::variant_alternative_t<0, Variant>>::value; };
            }
            && (sizeof...(Args) == 0)) {
                return detail::check_all_narrow_impl<Check, Variant, 0>();
            }
            else if constexpr (requires {
                typename Check<
                        std::variant_alternative_t<0, Variant>,
                        Variant,
                        Args...
                >;
                requires requires {
                    Check<
                            std::variant_alternative_t<0, Variant>,
                            Variant,
                            Args...
                    >::value;
                };
            }) {
                return detail::check_all_wide_impl<Check, Variant, 0, Args...>();
            } else {
                return false;
            }
        }
    } // namespace detail

    /**
     * @brief Compile-time predicate applied to all alternatives in a variant.
     *
     * @details
     * This concept verifies that a user-defined metafunction <code>Check</code>
     * succeeds for every alternative type contained in <code>Variant</code>.
     * Two evaluation modes are supported:
     * <ul>
     *   <li><b>Narrow form</b>: <code>Check&lt;T&gt;::value</code></li>
     *   <li><b>Wide form</b>:   <code>Check&lt;T, Variant, Args...&gt;::value</code></li>
     * </ul>
     * In the wide form, the second template parameter is <b>always</b> the full
     * <code>Variant</code> type.
     * This design ensures that <code>Check</code> can inspect the entire variant
     * structure without requiring users to redundantly pass it.
     * The mechanism short-circuits: if any alternative fails, the overall
     * result is <code>false</code>.
     *
     * <h4>Usage Example</h4>
     * This is how you define your own check concept:
     * @code
     * template &lt;typename Inner, typename Variant, typename... Args&gt;
     * struct your_wide_check
     * {
     *     // avoid unused parameter warning if your concept doesn't use Variant
     *     using _unused [[maybe_unused]] = Variant;
     *     static constexpr bool value = your_concept_1&lt;typename Inner, ... &gt;;
     *     // add Variant and Args as needed
     * };
     *
     * template &lt;typename Inner&gt;
     * struct your_narrow_check
     * {
     *     static constexpr bool value = your_concept_2&lt;typename Inner&gt;;
     * };
     * @endcode
     *
     * @tparam Check    A metafunction template that exposes a boolean <code>value</code>.
     * @tparam Variant  The <code>std::variant</code> whose alternatives are checked.
     * @tparam Args     Additional parameters forwarded to wide-form checks.
     *
     * @note
     * If <code>Check</code> does not define <code>value</code>, or if <code>value</code>
     * is not convertible to <code>bool</code>, the result is <code>false</code>.
     */
    template<template<typename...> typename Check, typename Variant, typename... Args>
    concept check_all = detail::check_all_impl<Check, Variant, Args...>();

    /**
     * @brief Extracts the transformed type of the I-th alternative in a variant.
     *
     * @details
     * This alias applies the user-provided transformation <code>TpTrans</code> to the
     * <code>I</code>-th alternative of <code>Variant</code>.
     * Before extraction, a full-variant validity check is performed: every alternative
     * <code>T</code> in <code>Variant</code> must satisfy
     * <code>TpTrans&lt;T&gt;::type</code> being a valid, non-void type.
     * If any alternative fails this requirement, substitution fails and the alias
     * cannot be instantiated.
     *
     * @tparam I        Index of the alternative in the variant.
     * @tparam Variant  A <code>std::variant</code> whose alternatives are transformed.
     * @tparam TpTrans  A unary metafunction template exposing <code>::type</code>.
     *
     * @note
     * If the transformation is invalid for any alternative, this alias participates
     * in SFINAE failure rather than producing a type.
     */
    template<std::size_t I, typename Variant, template<typename> typename TpTrans> requires (detail::check_all_narrow_impl<
            detail::is_valid_trans_holder<TpTrans>::template apply,
            Variant, 0
    >())
    using deduce_type_t = typename detail::deduce_type<I, Variant, TpTrans>::type;

    /**
     * @brief Applies a unary type transformation to every alternative in a variant.
     *
     * @details
     * The alias constructs a new <code>std::variant</code> whose alternatives are
     * precisely <code>TpTrans&lt;T&gt;::type</code> for each alternative <code>T</code>
     * in <code>Variant</code>.
     *
     * @note
     * <b>No additional semantic validation is performed.</b>  
     * If the transformation leads to conflicting or duplicated alternatives, such as:
     * <pre><code>
     *     std::variant&lt;TA, TB, ..., TA&gt;
     * </code></pre>
     * the compiler's standard diagnostic for <code>std::variant</code> will report
     * the error directly.  
     * This intentional design helps users discover logical mistakes in their ADT
     * mappings without silently suppressing them.
     *
     * @tparam Variant  The source <code>std::variant</code>.
     * @tparam TpTrans  A unary metafunction template providing <code>::type</code>.
     *
     * @note
     * Transformation failure for any alternative results in SFINAE failure of this alias.
     * No collapse or uniformity assumptions are made.
     */
    template<typename Variant, template<typename> typename TpTrans> requires (detail::check_all_narrow_impl<
            detail::is_valid_trans_holder<TpTrans>::template apply,
            Variant, 0
    >())
    using variant_transform_t = typename detail::variant_transform_impl<Variant, TpTrans>::type;

    /**
     * @brief Attempts to collapse a transformed variant into a single uniform type.
     *
     * @details
     * Each alternative <code>T</code> in the input <code>Variant</code> is mapped
     * through <code>TpTrans&lt;T&gt;::type</code>.
     * If all mapped results are exactly the same type, that type is exposed as the
     * result.
     * If any alternative maps to a different type, collapse fails and the alias
     * becomes <code>void</code>.
     *
     * @tparam Variant  A <code>std::variant</code> whose alternatives may collapse.
     * @tparam TpTrans  A unary metafunction template providing <code>::type</code>.
     *
     * @note
     * Collapse does not permit mixed mappings. If uniformity cannot be proven,
     * the resulting type is <code>void</code>.
     * This mechanism allows users to detect the boundary at which alternatives
     * can be considered belonging to the same external semantic family.
     */
    template<typename Variant, template<typename> typename TpTrans> requires (detail::check_all_narrow_impl<
            detail::is_valid_trans_holder<TpTrans>::template apply,
            Variant, 0
    >())
    using variant_collapse_t =
            typename detail::variant_collapse_impl<Variant, TpTrans>::type;
} // namespace jh::meta
