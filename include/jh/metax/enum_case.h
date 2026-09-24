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
 * @file enum_case.h
 * @brief Lightweight, strongly-typed match domain for scoped enum states.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::meta::enum_case</code> models a predicate domain over a scoped
 * enum type instead of extending that enum's value set. A case object may
 * represent one exact enum value, any present value, the absence of value,
 * or any state regardless of presence.
 * </p>
 *
 * <p>
 * The public interface intentionally centers on matching semantics. Callers
 * create constexpr case objects through <code>jh::meta::enum_case</code> and
 * then use <code>matches</code> or symmetric comparison operators against
 * <code>E</code> or <code>std::optional&lt;E&gt;</code>.
 * </p>
 *
 * <h4>Typical use cases</h4>
 * <ul>
 *   <li>state-machine guard domains,</li>
 *   <li>rule tables and filters,</li>
 *   <li>permission / role matching,</li>
 *   <li>pre-state to accepted post-state domain calculation.</li>
 * </ul>
 *
 * <h4>Design boundaries</h4>
 * <ul>
 *   <li>Only scoped enums (<code>enum class</code>) are accepted.</li>
 *   <li>The type is not a replacement for <code>std::optional&lt;E&gt;</code>.</li>
 *   <li>The implementation does not expose public tag inspection APIs.</li>
 * </ul>
 *
 * <h4>Basic usage</h4>
 * @code
 * enum class State {
 *     Idle,
 *     Running,
 *     Done
 * };
 *
 * using C = jh::meta::enum_case;
 *
 * static_assert(C::of&lt;State::Running&gt;.matches(State::Running));
 * static_assert(C::some&lt;State&gt;.matches(std::optional{State::Idle}));
 * static_assert(C::none&lt;State&gt;.matches(std::optional&lt;State&gt;{}));
 * static_assert(C::any&lt;State&gt;.matches(std::optional{State::Done}));
 * @endcode
 */

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "jh/conceptual/enum.h"

namespace jh::meta {

    /**
     * @brief Factory namespace object for constructing <code>enum_case_v</code>.
     */
    struct enum_case;

    /**
     * @brief Match-domain object for a scoped enum type.
     * @tparam E Scoped enum type to match against.
     *
     * @details
     * This type encapsulates one of four matching modes:
     * <ul>
     *   <li><code>tag::value</code> - one exact enum value,</li>
     *   <li><code>tag::some</code> - any present value,</li>
     *   <li><code>tag::any</code> - any state, including empty optional,</li>
     *   <li><code>tag::none</code> - only the empty optional state.</li>
     * </ul>
     *
     * Callers do not construct this type directly. Use
     * <code>jh::meta::enum_case::of</code>, <code>some</code>,
     * <code>any</code>, or <code>none</code>.
     */
    template<jh::concepts::scoped_enum E>
    class enum_case_v {
    public:
        /**
         * @brief Structural kind of the case object.
         * @details
         * <table>
         * <tr><th>tag</th><th>Meaning</th><th>Matches <code>E</code></th><th>Matches <code>std::optional&lt;E&gt;</code></th></tr>
         * <tr><td><code>value</code></td><td>Specific enum value</td><td>Equal to that value</td><td>Present and equal</td></tr>
         * <tr><td><code>some</code></td><td>Any present value</td><td>Always true</td><td><code>has_value()</code></td></tr>
         * <tr><td><code>any</code></td><td>Any state</td><td>Always true</td><td>Always true</td></tr>
         * <tr><td><code>none</code></td><td>No value</td><td>Always false</td><td><code>!has_value()</code></td></tr>
         * </table>
         */
        enum class tag {
            value,
            some,
            any,
            none
        };

    private:
        tag tag_;
        E enum_;

        constexpr enum_case_v(E e)
                : tag_(tag::value), enum_(e) {}

        constexpr enum_case_v(tag t)
                : tag_(t), enum_{} {}

        friend struct enum_case;

    public:
        enum_case_v() = delete;

        /**
         * @brief Compare two cases structurally.
         * @param rhs Right-hand side case.
         * @return Three-way comparison result.
         *
         * @details
         * Comparison first orders by <code>tag</code>. If both operands are
         * <code>tag::value</code>, the stored enum values are compared next.
         * For equal non-value tags, the objects compare equal.
         *
         * @note
         * This ordering is structural only. It does not represent business
         * priority or specificity.
         */
        [[nodiscard]] constexpr auto operator<=>(const enum_case_v& rhs) const {
            if (auto cmp = tag_ <=> rhs.tag_; cmp != 0) {
                return cmp;
            }

            if (tag_ == tag::value) {
                return enum_ <=> rhs.enum_;
            }

            return std::strong_ordering::equal;
        }

        /**
         * @brief Structural equality comparison between two cases.
         * @param rhs Right-hand side case.
         * @return <code>true</code> when both cases encode the same matching rule.
         */
        [[nodiscard]] constexpr bool operator==(const enum_case_v& rhs) const {
            return (*this <=> rhs) == 0;
        }

        /**
         * @brief Match this case against an actual enum value.
         * @param enum_state Actual enum state.
         * @return <code>true</code> if the case accepts <code>enum_state</code>.
         *
         * @details
         * Matching rules:
         * <ul>
         *   <li><code>value</code> - exact equality,</li>
         *   <li><code>some</code> - always true,</li>
         *   <li><code>any</code> - always true,</li>
         *   <li><code>none</code> - always false.</li>
         * </ul>
         */
        [[nodiscard]] constexpr bool matches(E enum_state) const {
            switch (tag_) {
                case tag::value:
                    return enum_ == enum_state;
                case tag::some:
                case tag::any:
                    return true;
                case tag::none:
                    return false;
            }

            return false;
        }

        /**
         * @brief Match this case against an optional enum state.
         * @param op_enum_state Actual optional state.
         * @return <code>true</code> if the case accepts <code>op_enum_state</code>.
         *
         * @details
         * Matching rules:
         * <ul>
         *   <li><code>value</code> - optional is engaged and equal to the stored value,</li>
         *   <li><code>some</code> - optional is engaged,</li>
         *   <li><code>any</code> - always true,</li>
         *   <li><code>none</code> - optional is disengaged.</li>
         * </ul>
         */
        [[nodiscard]] constexpr bool matches(std::optional<E> op_enum_state) const {
            switch (tag_) {
                case tag::value:
                    return op_enum_state.has_value() && enum_ == *op_enum_state;
                case tag::some:
                    return op_enum_state.has_value();
                case tag::any:
                    return true;
                case tag::none:
                    return !op_enum_state.has_value();
            }

            return false;
        }

        /**
         * @brief Compute a constexpr structural hash for the case object.
         * @return Encoded structural hash value.
         *
         * @details
         * Encoding:
         * <ul>
         *   <li><code>none</code>  -> <code>1</code></li>
         *   <li><code>some</code>  -> <code>2</code></li>
         *   <li><code>any</code>   -> <code>3</code></li>
         *   <li><code>value</code> -> <code>(underlying(enum_) &lt;&lt; 2) + 4</code></li>
         * </ul>
         *
         * @note
         * This hash is suitable for structural lookup over case objects. It is
         * not a reverse index from actual state to all matching cases because a
         * single state may match multiple cases.
         */
        [[nodiscard]] constexpr std::size_t hash() const noexcept {
            switch (tag_) {
                case tag::none:
                    return static_cast<std::size_t>(1ULL);

                case tag::some:
                    return static_cast<std::size_t>(2ULL);

                case tag::any:
                    return static_cast<std::size_t>(3ULL);

                case tag::value: {
                    using enum_underlying_t = std::underlying_type_t<E>;
                    using enum_unsigned_t = std::make_unsigned_t<enum_underlying_t>;

                    const auto value_hash = static_cast<std::size_t>(
                            static_cast<enum_unsigned_t>(
                                    static_cast<enum_underlying_t>(enum_)
                            )
                    );

                    return static_cast<std::size_t>((value_hash << 2) + 4ULL);
                }
            }

            return static_cast<std::size_t>(0ULL);
        }
    };

    /**
     * @brief Factory entry point for <code>enum_case_v</code> objects.
     *
     * @details
     * The nested variable templates construct the four supported case forms
     * while keeping <code>enum_case_v</code>'s constructors private.
     */
    struct enum_case {

        /**
         * @brief Alias for the concrete case type associated with <code>E</code>.
         * @tparam E Scoped enum type.
         */
        template<jh::concepts::scoped_enum E>
        using value_type = enum_case_v<E>;
        /**
         * @brief Construct a case that matches one exact enum value.
         * @tparam enum_state Scoped enum constant.
         */
        template<auto enum_state> requires jh::concepts::scoped_enum<decltype(enum_state)>
        inline static constexpr enum_case_v<decltype(enum_state)> of{enum_state};

        /**
         * @brief Construct a case that matches any present value of <code>E</code>.
         * @tparam E Scoped enum type.
         */
        template<jh::concepts::scoped_enum E>
        inline static constexpr enum_case_v<E> some{value_type<E>::tag::some};

        /**
         * @brief Construct a case that matches any state of <code>E</code>.
         * @tparam E Scoped enum type.
         */
        template<jh::concepts::scoped_enum E>
        inline static constexpr enum_case_v<E> any{value_type<E>::tag::any};

        /**
         * @brief Construct a case that matches only the empty optional state.
         * @tparam E Scoped enum type.
         */
        template<jh::concepts::scoped_enum E>
        inline static constexpr enum_case_v<E> none{value_type<E>::tag::none};

    };

    /**
     * @brief Symmetric match comparison between a case and an actual enum value.
     * @tparam E Scoped enum type.
     * @param lhs Case object.
     * @param rhs Actual enum state.
     * @return Equivalent to <code>lhs.matches(rhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator==(const enum_case_v<E>& lhs, E rhs) {
        return lhs.matches(rhs);
    }

    /**
     * @brief Symmetric match comparison between an actual enum value and a case.
     * @tparam E Scoped enum type.
     * @param lhs Actual enum state.
     * @param rhs Case object.
     * @return Equivalent to <code>rhs.matches(lhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator==(E lhs, const enum_case_v<E>& rhs) {
        return rhs.matches(lhs);
    }

    /**
     * @brief Negated match comparison between a case and an actual enum value.
     * @tparam E Scoped enum type.
     * @param lhs Case object.
     * @param rhs Actual enum state.
     * @return Equivalent to <code>!lhs.matches(rhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator!=(const enum_case_v<E>& lhs, E rhs) {
        return !lhs.matches(rhs);
    }

    /**
     * @brief Negated match comparison between an actual enum value and a case.
     * @tparam E Scoped enum type.
     * @param lhs Actual enum state.
     * @param rhs Case object.
     * @return Equivalent to <code>!rhs.matches(lhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator!=(E lhs, const enum_case_v<E>& rhs) {
        return !rhs.matches(lhs);
    }

    /**
     * @brief Symmetric match comparison between a case and an optional enum state.
     * @tparam E Scoped enum type.
     * @param lhs Case object.
     * @param rhs Actual optional state.
     * @return Equivalent to <code>lhs.matches(rhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator==(const enum_case_v<E>& lhs, const std::optional<E>& rhs) {
        return lhs.matches(rhs);
    }

    /**
     * @brief Symmetric match comparison between an optional enum state and a case.
     * @tparam E Scoped enum type.
     * @param lhs Actual optional state.
     * @param rhs Case object.
     * @return Equivalent to <code>rhs.matches(lhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator==(const std::optional<E>& lhs, const enum_case_v<E>& rhs) {
        return rhs.matches(lhs);
    }

    /**
     * @brief Negated match comparison between a case and an optional enum state.
     * @tparam E Scoped enum type.
     * @param lhs Case object.
     * @param rhs Actual optional state.
     * @return Equivalent to <code>!lhs.matches(rhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator!=(const enum_case_v<E>& lhs, const std::optional<E>& rhs) {
        return !lhs.matches(rhs);
    }

    /**
     * @brief Negated match comparison between an optional enum state and a case.
     * @tparam E Scoped enum type.
     * @param lhs Actual optional state.
     * @param rhs Case object.
     * @return Equivalent to <code>!rhs.matches(lhs)</code>.
     */
    template<jh::concepts::scoped_enum E>
    constexpr bool operator!=(const std::optional<E>& lhs, const enum_case_v<E>& rhs) {
        return !rhs.matches(lhs);
    }
}
