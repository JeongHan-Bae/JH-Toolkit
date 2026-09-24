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
 */
/**
 * @file expected.h
 * @brief Expected values for scoped-enum error codes, POD-like when T qualifies.
 *
 * The representation is intentionally small and simple: a value and an
 * optional enum underlying value. An empty error optional means success.
 */

#pragma once

#include <cstdint>
#include <concepts>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

#include "jh/conceptual/enum.h"
#include "jh/pods/optional.h"

namespace jh::meta {
    /**
     * @brief A lightweight error value for constructing an unsuccessful expected.
     * @tparam E Scoped enum class used as the error domain.
     */
    template<jh::concepts::scoped_enum E>
    struct unexpected final {
        using error_type = E;
        using underlying_type [[maybe_unused]]
                = typename jh::concepts::enum_class<error_type>::underlying;

    private:
        error_type error_;

    public:
        constexpr unexpected() noexcept = default;

        constexpr explicit unexpected(E value) noexcept: error_(value) {}

        [[nodiscard]] constexpr E &error() & noexcept { return error_; }

        [[nodiscard]] constexpr const E &error() const & noexcept { return error_; }

        [[nodiscard]] constexpr E &&error() && noexcept { return std::move(error_); }

        constexpr bool operator==(const unexpected &) const noexcept = default;
    };

    template<jh::concepts::scoped_enum E>
    [[maybe_unused]] unexpected(E
    e) ->
    unexpected<E>;

    /**
     * @brief Result of an operation that may fail with a scoped enum.
     *
     * Success stores a value and leaves its error optional empty. Failure stores
     * a default value and the error enum's underlying integer. When T satisfies
     * <code>jh::pod::cv_free_pod_like</code>, this wrapper is POD-like too.
     *
     * @tparam T Success value type. Any object type is allowed. When T satisfies
     *           <code>jh::pod::cv_free_pod_like</code>, this wrapper is POD-like too.
     * @tparam E Scoped enum class used as the error domain.
     */
    template<typename T, jh::concepts::scoped_enum E> requires (std::is_object_v<T> && !std::is_array_v<T>)
    struct expected final {
        using value_type = T;
        using error_type = E;
        using error_underlying_type = typename jh::concepts::enum_class<error_type>::underlying;

        template<typename U>
        using rebind = expected<U, E>;

    private:
        T value_;
        jh::pod::optional<error_underlying_type> ec_;

    public:

        constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

        constexpr expected(const expected &) = default;

        constexpr expected(expected &&) = default;

        constexpr expected &operator=(const expected &) = default;

        constexpr expected &operator=(expected &&) = default;

        constexpr ~expected() = default;

        constexpr expected(const T &success_value)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
                : value_(success_value), ec_{} {}

        constexpr expected(T &&success_value)
        noexcept(std::is_nothrow_move_constructible_v<T>)
                : value_(std::move(success_value)), ec_{} {}

        constexpr expected(unexpected<E> failure) noexcept(
        std::is_nothrow_default_constructible_v<T>
        ) requires(std::is_default_constructible_v<T>)
                : value_{}, ec_{} {
            ec_.store(static_cast<error_underlying_type>(failure.error()));
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return has_value();
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return !ec_.has();
        }

        [[nodiscard]] constexpr bool has_error() const noexcept {
            return ec_.has();
        }

        [[nodiscard]] constexpr T *operator->() noexcept { return std::addressof(value()); }

        [[nodiscard]] constexpr const T *operator->() const noexcept { return std::addressof(value()); }

        [[nodiscard]] constexpr T &operator*() & noexcept { return value(); }

        [[nodiscard]] constexpr const T &operator*() const & noexcept { return value(); }

        [[nodiscard]] constexpr T &&operator*() && noexcept { return std::move(value()); }

        [[nodiscard]] constexpr const T &&operator*() const && noexcept { return std::move(value()); }

        [[nodiscard]] constexpr T &value() & noexcept {
            if (!has_value()) std::abort();
            return value_;
        }

        [[nodiscard]] constexpr const T &value() const & noexcept {
            if (!has_value()) std::abort();
            return value_;
        }

        [[nodiscard]] constexpr T &&value() && noexcept {
            if (!has_value()) std::abort();
            return std::move(value_);
        }

        [[nodiscard]] constexpr const T &&value() const && noexcept {
            if (!has_value()) std::abort();
            return std::move(value_);
        }

        /** @brief Return the typed error code; requires <code>has_error()</code>. */
        [[nodiscard]] constexpr E error() const noexcept {
            if (!has_error()) std::abort();
            return static_cast<E>(ec_.ref());
        }

        template<typename U>
        requires (std::is_copy_constructible_v<T> && std::is_convertible_v<U &&, T>)
        [[nodiscard]] constexpr T value_or(U &&fallback) const &
        noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_constructible_v<T, U &&>) {
            return has_value() ? value_ : static_cast<T>(std::forward<U>(fallback));
        }

        template<typename U>
        requires (std::is_move_constructible_v<T> && std::is_convertible_v<U &&, T>)
        [[nodiscard]] constexpr T value_or(U &&fallback) &&
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_constructible_v<T, U &&>) {
            return has_value() ? std::move(value_) : static_cast<T>(std::forward<U>(fallback));
        }

        template<typename U>
        requires std::is_convertible_v<U &&, E>
        [[nodiscard]] constexpr E error_or(U &&fallback) const
        noexcept(std::is_nothrow_convertible_v<U &&, E>) {
            return has_error() ? error() : static_cast<E>(std::forward<U>(fallback));
        }

        constexpr expected &operator=(const T &success_value)
        noexcept(std::is_nothrow_copy_assignable_v<T>) requires
                (std::is_copy_assignable_v<T>) {
            value_ = success_value;
            ec_.clear();
            return *this;
        }

        constexpr expected &operator=(T &&success_value)
        noexcept(std::is_nothrow_move_assignable_v<T>) requires
                (std::is_move_assignable_v<T>) {
            value_ = std::move(success_value);
            ec_.clear();
            return *this;
        }

        constexpr expected &operator=(unexpected<E> failure) noexcept(
        std::is_nothrow_default_constructible_v<T> &&
        std::is_nothrow_assignable_v<T &, T> &&
        std::is_nothrow_destructible_v<T>
        ) requires (std::is_default_constructible_v<T> && std::is_assignable_v<T &, T>) {
            value_ = T{};
            ec_.store(static_cast<error_underlying_type>(failure.error()));
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(const expected &rhs) const
        noexcept(noexcept(std::declval<const T &>() == std::declval<const T &>())) requires requires
        (
        const T &lhs,
        const T &other
        ) {
            { lhs == other } -> std::convertible_to<bool>;
        }

        {
            if (has_value() != rhs.has_value()) return false;
            if (has_value()) return value_ == rhs.value_;
            return error() == rhs.error();
        }
    };

}
