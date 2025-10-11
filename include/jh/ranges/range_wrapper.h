#pragma once

#include "jh/conceptual/iterator.h"
#include <ranges>


namespace jh::ranges {
    namespace detail {

        template<typename Inner, typename Sentinel = Inner> requires jh::concepts::input_iterator<Inner>
        struct completed_iterator : public Inner {

            // ---- Input Basics ----
            using inner_type = Inner;
            using sentinel_type = Sentinel;

            using value_type = jh::concepts::iterator_value_t<Inner>;
            using reference = jh::concepts::iterator_reference_t<Inner>;
            using rvalue_reference = jh::concepts::iterator_rvalue_reference_t<Inner>;
            using difference_type = std::ptrdiff_t;

            using iterator_category =
                    std::conditional_t<
                            jh::concepts::random_access_iterator<Inner>, std::random_access_iterator_tag,
                            std::conditional_t<
                                    jh::concepts::bidirectional_iterator<Inner>, std::bidirectional_iterator_tag,
                                    std::conditional_t<
                                            jh::concepts::forward_iterator<Inner>, std::forward_iterator_tag,
                                            std::input_iterator_tag>>>;

            completed_iterator() = default;

            constexpr explicit completed_iterator(const Inner &it) : Inner(it) {}

            constexpr explicit completed_iterator(Inner &&it) noexcept(std::is_nothrow_move_constructible_v<Inner>)
                    : Inner(std::move(it)) {}

            // should always have it++
            completed_iterator &operator++() {
                Inner::operator++();
                return *this;
            }

            // only expose ++it if exists
            constexpr auto operator++(int)
            noexcept(noexcept(std::declval<Inner &>()++))
            requires requires(Inner & it) { it++; }
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr decltype(auto)
            operator*() noexcept(noexcept(*std::declval<Inner &>()))
            requires requires(Inner & it) { *it; }
            {
                return *static_cast<Inner &>(*this);
            }

            constexpr decltype(auto)
            operator*() const noexcept(noexcept(*std::declval<const Inner &>()))
            requires requires(const Inner &it) { *it; }
            {
                return *static_cast<const Inner &>(*this);
            }

            // compare with sentinel
            friend constexpr bool operator==(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(static_cast<const Inner &>(a) == b)) {
                return static_cast<const Inner &>(a) == b;
            }

            friend constexpr bool operator==(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(a == static_cast<const Inner &>(b))) {
                return a == static_cast<const Inner &>(b);
            }

            friend constexpr bool operator!=(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(!(a == b))) {
                return !(a == b);
            }

            friend constexpr bool operator!=(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(!(a == b))) {
                return !(a == b);
            }

            // move
            constexpr completed_iterator(
                    completed_iterator &&) noexcept(std::is_nothrow_move_constructible_v<Inner>) = default;

            constexpr completed_iterator &
            operator=(completed_iterator &&) noexcept(std::is_nothrow_move_assignable_v<Inner>) = default;

            // ---- Forward Addition ----

            // copy
            constexpr completed_iterator(const completed_iterator &) requires std::copy_constructible<Inner> = default;

            constexpr completed_iterator &operator=(const completed_iterator &) requires std::copyable<Inner> = default;

            // ---- Bidirectional Addition ----
            constexpr completed_iterator &operator--()
            noexcept(noexcept(--std::declval<Inner &>()))
            requires requires(Inner & it) { --it; }
            {
                Inner::operator--();
                return *this;
            }

            constexpr completed_iterator operator--(int)
            noexcept(noexcept(std::declval<Inner &>()--))
            requires requires(Inner & it) { it--; }
            {
                auto tmp = *this;
                --(*this);
                return tmp;
            }
            // ---- Random Access Addition ----

            // forward N
            constexpr completed_iterator &operator+=(difference_type n)
            noexcept(noexcept(std::declval<Inner &>() += n))
            requires requires(Inner &it, difference_type n) { it += n; }
            {
                Inner::operator+=(n);
                return *this;
            }

            // backwards N
            constexpr completed_iterator &operator-=(difference_type n)
            noexcept(noexcept(std::declval<Inner &>() -= n))
            requires requires(Inner &it, difference_type n) { it -= n; }
            {
                Inner::operator-=(n);
                return *this;
            }

            // return new ite (forward N)
            constexpr completed_iterator operator+(difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>() + n))
            requires requires(const Inner &it, difference_type n) { it + n; }
            {
                return completed_iterator(static_cast<const Inner &>(*this) + n);
            }

            // return new ite (backwards N)
            constexpr completed_iterator operator-(difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>() - n))
            requires requires(const Inner &it, difference_type n) { it - n; }
            {
                return completed_iterator(static_cast<const Inner &>(*this) - n);
            }

            // difference
            friend constexpr difference_type operator-(const completed_iterator &a,
                                                       const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) - static_cast<const Inner &>(b)))
            requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x - y; }

            {
                return static_cast<const Inner &>(a) - static_cast<const Inner &>(b);
            }

            // index access
            constexpr decltype(auto) operator[](difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>()[n]))
            requires requires(const Inner &it, difference_type n) { it[n]; }
            {
                return static_cast<const Inner &>(*this)[n];
            }

            // compare operators
            friend constexpr bool operator<(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) < static_cast<const Inner &>(b)))
            requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x < y; }

            {
                return static_cast<const Inner &>(a) < static_cast<const Inner &>(b);
            }

            friend constexpr bool operator>(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) > static_cast<const Inner &>(b)))
            requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x > y; }

            {
                return static_cast<const Inner &>(a) > static_cast<const Inner &>(b);
            }

            friend constexpr bool operator<=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) <= static_cast<const Inner &>(b)))
            requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x <= y; }

            {
                return static_cast<const Inner &>(a) <= static_cast<const Inner &>(b);
            }

            friend constexpr bool operator>=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) >= static_cast<const Inner &>(b)))
            requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x >= y; }

            {
                return static_cast<const Inner &>(a) >= static_cast<const Inner &>(b);
            }

            // n + ite
            friend constexpr completed_iterator operator+(difference_type n, const completed_iterator &it)
            noexcept(noexcept(n + static_cast<const Inner &>(it)))
            requires requires(const Inner &i, difference_type n) { n + i; }
            {
                return completed_iterator(n + static_cast<const Inner &>(it));
            }
            // ---- Sentinel difference support ----

            // completed_iterator - Sentinel
            friend constexpr difference_type operator-(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(static_cast<const Inner &>(a) - b))
            requires requires(
                    const Inner &x,
                    const Sentinel &y
            ) { x - y; }

            {
                return static_cast<const Inner &>(a) - b;
            }

            // Sentinel - completed_iterator
            friend constexpr difference_type operator-(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(a - static_cast<const Inner &>(b)))
            requires requires(
                    const Sentinel &x,
                    const Inner &y
            ) { x - y; }

            {
                return a - static_cast<const Inner &>(b);
            }
        };
    }

    template<typename Seq>
    class range_wrapper : public std::ranges::view_interface<range_wrapper<Seq>> {
        using Stored = std::conditional_t<
                std::is_reference_v<Seq>,
                std::reference_wrapper<std::remove_reference_t<Seq>>,
                Seq>;
        Stored seq_;
    public:

        using inner_iterator = decltype(std::declval<Seq &>().begin());
        using sentinel = decltype(std::declval<Seq &>().end());
        using iterator = detail::completed_iterator<inner_iterator, sentinel>;

        explicit range_wrapper(Seq &&s)
                : seq_(wrap(std::forward<Seq>(s))) {}

        auto begin() noexcept(noexcept(get().begin())) { return iterator(get().begin()); }

        auto end() noexcept(noexcept(get().end())) { return get().end(); }

    private:
        static auto wrap(auto &&v) {
            if constexpr (std::is_reference_v<decltype(v)>)
                return std::ref(v);
            else
                return std::forward<decltype(v)>(v);
        }

        constexpr decltype(auto) get() noexcept {
            if constexpr (std::is_reference_v<Seq>)
                return seq_.get();
            else
                return (seq_);
        }

        constexpr decltype(auto) get() const noexcept {
            if constexpr (std::is_reference_v<Seq>)
                return seq_.get();
            else
                return (seq_);
        }
    };
}