#pragma once

#include <ranges>
#include <type_traits>
#include "jh/conceptual/iterator.h"
#include "jh/conceptual/sequence.h"

namespace jh {

    /**
     * @brief User customization point for container element deduction.
     *
     * This template allows users to explicitly register the value type
     * of custom or non-standard containers that cannot be automatically
     * deduced by the generic detection logic.
     *
     * When specialized, this helper provides a consistent <code>value_type</code>
     * that will be used in preference to any automatic deduction.
     * The specialization must define:
     * @code
     * template&lt;&gt;
     * struct jh::container_deduction&lt;YourContainer&gt; {
     *     using value_type = YourElementType;
     * };
     * @endcode
     *
     * Registration should be used in the following cases:
     * <ul>
     * <li>The container does not define <code>value_type</code> internally.</li>
     * <li>The container's <code>value_type</code> does not match the element
     *     type deduced from its iterator behavior.</li>
     * </ul>
     *
     * In either case, a registered specialization of this template takes
     * precedence over all deduction mechanisms and resolves conflicts
     * between declared and deduced types.
     */
    template<typename C>
    struct container_deduction; ///< intentionally undefined, detected via SFINAE

} // namespace jh


namespace jh::concepts::detail {

    /// @brief Detector for <code>C::value_type</code> declaration.
    template<typename C, typename = void>
    struct declared_value {
        using type = void;
    };

    /// @brief Specialization when container declares a member <code>value_type</code>.
    template<typename C>
    struct declared_value<C, std::void_t<typename C::value_type>> {
        using type = typename C::value_type;
    };

/// @brief Detector for iterator-based deduction.
    template<typename C, typename = void>
    struct deduced_value {
        using type = void;
    };

/// @brief Deduction specialization when iterator value type is available.
    template<typename C>
    struct deduced_value<C,
            std::void_t<jh::concepts::iterator_value_t<jh::concepts::iterator_t<C>>>> {
        using type = jh::concepts::iterator_value_t<jh::concepts::iterator_t<C>>;
    };

/// @brief Detector for user override via <code>jh::container_deduction&lt;T&gt;::value</code>.
    template<typename T, typename = void>
    struct container_deduction_resolver {
        using type = void;
    };

/// @brief Specialization when <code>jh::container_deduction&lt;T&gt;::value</code> is defined.
    template<typename T>
    struct container_deduction_resolver<
            T,
            std::void_t<typename jh::container_deduction<T>::value_type>
    > {
        using type = typename jh::container_deduction<T>::value_type;
    };

/**
 * @brief Unified deduction logic for container value type.
 *
 * Deduction priority:
 * <ol>
 * <li>User override via <code>jh::container_deduction&lt;T&gt;::value</code></li>
 * <li>Declared <code>C::value_type</code></li>
 * <li>Deduced from <code>iterator_t&lt;C&gt;</code> and <code>iterator_value_t</code></li>
 * <li>If both declared and deduced exist and have a common reference type,
 *     declared type is preferred</li>
 * <li>Otherwise, <tt>void</tt></li>
 * </ol>
 */
    template<typename C>
    struct container_value_type_impl {
    private:
        using override_t = typename container_deduction_resolver<C>::type;
        using declared_t = typename declared_value<C>::type;
        using deduced_t = typename deduced_value<C>::type;

    public:
        static constexpr bool has_override = !std::same_as<override_t, void>;
        static constexpr bool has_declared = !std::same_as<declared_t, void>;
        static constexpr bool has_deduced = !std::same_as<deduced_t, void>;

        using type = decltype([]() {
            // Step 1: user override always wins
            if constexpr (has_override) {
                return std::type_identity<override_t>{};
            }
                // Step 2: declared only
            else if constexpr (has_declared && !has_deduced) {
                return std::type_identity<declared_t>{};
            }
                // Step 3: deduced only
            else if constexpr (!has_declared && has_deduced) {
                return std::type_identity<deduced_t>{};
            }
                // Step 4: both exist, and share a common reference
            else if constexpr (has_declared && has_deduced &&
                               std::common_reference_with<declared_t, deduced_t>) {
                return std::type_identity<declared_t>{};
            }
                // Step 5: fallback
            else {
                return std::type_identity<void>{};
            }
        }())::type;
    };

} // namespace jh::concepts::detail

namespace jh::concepts {

    /**
     * @brief Deduce the value type of a container <code>C</code>.
     *
     * @details
     * Resolution rules:
     * <ol>
     * <li>If <code>jh::container_deduction&lt;C&gt;::value_type</code> is explicitly defined,
     *     it overrides all other deduction mechanisms.</li>
     * <li>Otherwise, deduction proceeds using the following logic:
     *     <ul>
     *     <li>If only <code>C::value_type</code> exists, it is used.</li>
     *     <li>If only iterator deduction (<code>iterator_t</code> and
     *         <code>iterator_value_t</code>) is available, it is used.</li>
     *     <li>If both <code>C::value_type</code> and iterator deduction exist,
     *         they must not conflict. If they share a common reference type,
     *         the declared <code>C::value_type</code> is selected.</li>
     *     </ul>
     * </li>
     * <li>If no valid deduction is possible, the result is <tt>void</tt>.</li>
     * </ol>
     *
     * @note
     * When using a proxy reference type within an iterator, the proxy should be
     * implicitly convertible to the iterator's <code>value_type</code> to ensure correct
     * participation in generic deduction. For full interoperability, it is
     * recommended to register explicit <code>std::common_reference</code>
     * specializations as follows:
     *
     * @code
     * template&lt;&gt; struct std::common_reference&lt;ProxyT, T&gt; { using type = T; };
     * template&lt;&gt; struct std::common_reference&lt;T, ProxyT&gt; { using type = T; };
     * template&lt;&gt; struct std::common_reference&lt;ProxyT, ProxyT&gt; { using type = ProxyT; };
     * @endcode
     *
     * For completeness, derived forms should be added for reference and rvalue
     * combinations, inheriting from the base forms:
     * @code
     * template&lt;&gt; struct std::common_reference&lt;ProxyT&amp;, T&amp;&gt;
     *     : std::common_reference&lt;ProxyT, T&gt; {};
     * template&lt;&gt; struct std::common_reference&lt;ProxyT&amp;&amp;, T&amp;&amp;&gt;
     *     : std::common_reference&lt;ProxyT, T&gt; {};
     * // and T, ProxyT swapped forms, ProxyT with itself, etc.
     * @endcode
     *
     * This ensures that proxy iterators remain compatible with generic
     * range-based algorithms and container deduction mechanisms.
     *
     * @tparam C The container type to deduce from.
     * @return The deduced element type or <tt>void</tt> if deduction fails.
     */
    template<typename C>
    using container_value_t = typename detail::container_value_type_impl<C>::type;

} // namespace jh::concepts
