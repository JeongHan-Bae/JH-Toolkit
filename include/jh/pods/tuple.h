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
 * @file tuple.h (pods)
 * @brief Implementation of POD-compatible tuple, pair and array bindings.
 * <p>
 * This header defines <code>jh::pod::tuple&lt;Ts...&gt;</code> and its
 * interoperability with <code>std::tuple_size</code> and
 * <code>std::tuple_element</code>. It also provides <code>jh::pod::get</code>
 * overloads for <code>tuple</code>, <code>pair</code>, and
 * <code>array</code> so that they can participate in structured bindings
 * and tuple-like generic programming.
 * </p>
 * <p>
 * The implementation is based on recursive aggregate composition:
 * every element is stored inside a trivial wrapper
 * <code>tuple_field&lt;I, T&gt;</code>, and each layer of
 * <code>tuple_impl</code> provides one field and a sublayer for the rest.
 * This ensures that the resulting type remains a true POD while
 * supporting element-wise access.
 * </p>
 *
 * <h4>Example</h4>
 *
 * @code
 * #include &lt;jh/pod&gt;
 * using namespace jh::pod;
 *
 * int main() {
 *     auto t = make_tuple(7, 3.14f);
 *     std::cout << t << std::endl;     // prints: (7, 3.14)
 * }
 * @endcode
 *
 *
 * <h4>Important Notes on Initialization</h4>
 * Clang (C++20 and later) fully supports direct aggregate initialization:
 * @code
 * jh::pod::tuple&lt;int, float&gt; t{7, 3.14f}; // valid in Clang
 * @endcode
 * However, GCC may reject the same form with an error such as
 * "too many initializers". This is due to stricter handling of
 * aggregate inheritance.
 *
 * @warning
 * For GCC (especially ≤ 13), prefer one of the following forms:
 * @code
 * // Explicit nested braces
 * jh::pod::tuple&lt;int, float&gt; t{{ {7}, {{3.14f}, {}} }};
 *
 * // or, recommended portable helper
 * auto t = jh::pod::make_tuple(7, 3.14f);
 * @endcode
 *
 * @note
 * The output operator (<code>operator&lt;&lt;</code>) for <code>tuple</code>,
 * <code>pair</code> and <code>array</code> is provided in
 * <code>&lt;jh/pods/stringify.h&gt;</code>. After including it, all
 * these POD containers can be directly printed using <code>std::ostream</code>.
 *
 * @see jh::pod::pair
 * @see jh::pod::array
 */

#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <iostream>
#include "jh/pods/pod_like.h"
#include "jh/pods/pair.h"
#include "jh/pods/array.h"

namespace jh::pod {
    namespace detail {
        template<std::size_t, typename T>
        struct tuple_field final {
            T value;
        };

        template<typename IndexSeq, jh::pod::cv_free_pod_like... Ts>
        struct tuple_impl;

        template<std::size_t I, typename T, std::size_t... Is, typename... Rest>
        struct tuple_impl<std::index_sequence<I, Is...>, T, Rest...> {
            tuple_field<I, T> field;
            tuple_impl<std::index_sequence<Is...>, Rest...> rest;
        };

        template<>
        struct tuple_impl<std::index_sequence<>> {
        };

        template<std::size_t I, typename IndexSeq, typename... Ts>
        decltype(auto) get(tuple_impl<IndexSeq, Ts...> &t) noexcept {
            if constexpr (I == 0)
                return (t.field.value);
            else
                return get<I - 1>(t.rest);
        }

        template<std::size_t I, typename IndexSeq, typename... Ts>
        auto get(const tuple_impl<IndexSeq, Ts...> &t) noexcept {
            if constexpr (I == 0)
                return t.field.value; // copy elision
            else
                return get<I - 1>(t.rest);
        }

    } // namespace detail

    /**
     * @brief POD-compatible tuple type supporting structured bindings and tuple-like utilities.
     *
     * @details
     * The <code>jh::pod::tuple&lt;Ts...&gt;</code> template implements a recursive aggregate
     * where each element is stored in a <code>tuple_field&lt;I, T&gt;</code> layer.
     * This ensures full POD compliance while supporting element-wise access through
     * <code>jh::pod::get</code>, and interoperability with <code>std::tuple_size</code>
     * and <code>std::tuple_element</code>.
     *
     * <ul>
     *   <li>Fully trivial and standard layout — memcpy-safe</li>
     *   <li>Usable in structured bindings</li>
     *   <li>Compatible with <code>jh::pod::pair</code> and <code>jh::pod::array</code></li>
     * </ul>
     *
     * @tparam Ts Parameter pack of element types; each must satisfy
     *            <code>cv_free_pod_like</code>.
     *
     * @note
     * When targeting GCC ≤ 13, direct brace initialization may fail;
     * prefer using <code>jh::pod::make_tuple</code> for portable creation.
     *
     * @see jh::pod::make_tuple
     * @see jh::pod::get
     */
    template<jh::pod::cv_free_pod_like... Ts>
    struct tuple final : detail::tuple_impl<std::index_sequence_for<Ts...>, Ts...> {
    };

    template<std::size_t I, typename... Ts>
    decltype(auto) get(tuple<Ts...> &t) noexcept {
        return get<I>(static_cast<detail::tuple_impl<std::index_sequence_for<Ts...>, Ts...> &>(t));
    }

    template<std::size_t I, typename... Ts>
    auto get(const tuple<Ts...> &t) noexcept {
        return get<I>(static_cast<const detail::tuple_impl<std::index_sequence_for<Ts...>, Ts...> &>(t));
    }

    template<std::size_t I, typename T1, typename T2>
    requires (I < 2)
    constexpr decltype(auto) get(jh::pod::pair<T1, T2>& p) noexcept {
        if constexpr (I == 0) return (p.first);
        else return (p.second);
    }

    template<std::size_t I, typename T1, typename T2>
    requires (I < 2)
    constexpr decltype(auto) get(const jh::pod::pair<T1, T2>& p) noexcept {
        if constexpr (I == 0) return (p.first);
        else return (p.second);
    }

    template<std::size_t I, typename T, std::uint16_t N>
    requires (I < N)
    constexpr decltype(auto) get(jh::pod::array<T, N>& a) noexcept {
        return (a.data[I]);
    }

    template<std::size_t I, typename T, std::uint16_t N>
    requires (I < N)
    constexpr decltype(auto) get(const jh::pod::array<T, N>& a) noexcept {
        return a.data[I];
    }

} // namespace jh::pod

namespace std {
    template<typename... Ts>
    struct tuple_size<jh::pod::tuple<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {
    };

    template<size_t I, typename... Ts>
    requires (I < sizeof...(Ts))
    struct tuple_element<I, jh::pod::tuple<Ts...>> {
        using type [[maybe_unused]] = typename tuple_element<I, tuple<Ts...>>
        ::type;
    };

    template<typename T1, typename T2>
    struct tuple_size<jh::pod::pair<T1, T2>> : std::integral_constant<std::size_t, 2> {};

    template<std::size_t I, typename T1, typename T2>
    requires (I < 2)
    struct tuple_element<I, jh::pod::pair<T1, T2>> {
        using type = std::conditional_t<I == 0, T1, T2>;
    };

    template<typename T, std::uint16_t N>
    struct tuple_size<jh::pod::array<T, N>> : std::integral_constant<std::size_t, N> {};

    template<std::size_t I, typename T, std::uint16_t N>
    requires (I < N)
    struct tuple_element<I, jh::pod::array<T, N>> {
        using type = T;
    };
} // namespace std

namespace jh::pod {

    /**
     * @brief Constructs a POD-compatible tuple from given arguments.
     *
     * @details
     * This is the general-purpose tuple construction interface aligned with
     * <code>std::make_tuple</code>, but restricted to POD-compatible types.
     * Each element is assigned by index, ensuring consistent aggregate
     * initialization semantics across compilers.
     *
     * <ul>
     *   <li>Portable alternative to direct brace initialization</li>
     *   <li>Preserves POD guarantees of <code>jh::pod::tuple</code></li>
     *   <li>Automatically decays arguments to their underlying types</li>
     * </ul>
     *
     * @note
     * Arrays and string literals decay to pointers when passed by value.
     * To preserve the complete POD content and avoid type decay,
     * wrap them explicitly with <code>jh::pod::array&lt;T, N&gt;</code> or
     * <code>jh::pod::array&lt;char, N&gt;</code> when constructing tuples.
     *
     * @tparam Ts Variadic list of POD-compatible argument types.
     * @param args The values to store in the tuple.
     * @return A <code>jh::pod::tuple</code> containing the given values.
     *
     * @see jh::pod::tuple
     */
    template <jh::pod::cv_free_pod_like... Ts>
    constexpr auto make_tuple([[maybe_unused]] Ts&&... args) noexcept {
        using tuple_t = tuple<std::decay_t<Ts>...>;
        tuple_t t{};
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((get<Is>(t) = std::forward<Ts>(args)), ...);
        }(std::index_sequence_for<Ts...>{});
        return t;
    }

    template<typename... Ts>
    constexpr bool operator==(const tuple<Ts...>& lhs, const tuple<Ts...>& rhs) noexcept {
        bool result = true;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((result = result && (get<Is>(lhs) == get<Is>(rhs))), ...);
        }(std::index_sequence_for<Ts...>{});
        return result;
    }

    template<typename... Ts>
    constexpr bool operator!=(const tuple<Ts...>& lhs, const tuple<Ts...>& rhs) noexcept {
        return !(lhs == rhs);
    }
} // namespace jh::pod
