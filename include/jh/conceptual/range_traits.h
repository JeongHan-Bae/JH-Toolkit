// jh/conceptual/range_traits.h

#pragma once

#include <iterator>
#include <ranges>
#include <functional>
#include <type_traits>


namespace jh::concepts {

    template<typename F, typename R>
    concept vis_function_for = requires(const std::remove_cvref_t<R> &r, const F &f) {
        { std::begin(r) } -> std::input_iterator;
        { *std::begin(r) };
        { std::invoke(f, *std::begin(r)) };
        requires (!std::is_void_v<decltype(std::invoke(f, *std::begin(r)))>);
    };

    template<typename R, bool UseRefWrapper = false>
    struct range_storage_traits {
        using raw_t [[maybe_unused]] = std::remove_cvref_t<R>;
        static constexpr bool is_lvalue = std::is_lvalue_reference_v<R>;

        using stored_t = std::conditional_t<
                is_lvalue,
                std::conditional_t<UseRefWrapper,
                        std::reference_wrapper<std::remove_reference_t<R>>,
                        R>,
                std::remove_cvref_t<R>
        >;

        static constexpr auto wrap(R &&v) noexcept(
        std::is_nothrow_constructible_v<stored_t, R&&>
        ) {
            if constexpr (is_lvalue && UseRefWrapper) {
                if constexpr (std::is_const_v<std::remove_reference_t<R>>)
                    return std::cref(v);
                else
                    return std::ref(v);
            } else if constexpr (is_lvalue)
                return v;
            else
                return std::move(v);
        }

        static constexpr decltype(auto) get(stored_t &v) noexcept {
            if constexpr (is_lvalue && UseRefWrapper)
                return v.get();
            else
                return (v);
        }

        static constexpr decltype(auto) get(const stored_t &v) noexcept {
            if constexpr (is_lvalue && UseRefWrapper)
                return v.get();
            else
                return (v);
        }
    };
}
