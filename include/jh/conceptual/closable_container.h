#pragma once

#include <cstdint>
#include <ranges>
#include <type_traits>
#include <vector>
#include "jh/conceptual/container_traits.h"

namespace jh::concepts::detail {

// ============================================================
// closure classification
// ============================================================
/**
 * @brief Classification of how a container `C` can be constructed ("closed")
 *        from a range `R`.
 */
    enum class closable_status : std::uint8_t {
        none = 0,               ///< Not closable

        // --- Direct constructions ---
        direct_copy,            ///< C(begin, end)
        direct_move,            ///< C(make_move_iterator(begin), make_move_iterator(end))

        // --- Via vector bridge ---
        via_vector_whole,       ///< C(vector(...))
        via_vector_copy,        ///< C(vector(...).begin(), vector(...).end())
        via_vector_move,        ///< C(make_move_iterator(vector(...).begin()), ...)

        // --- Adapter wrapping ---
        adapter_via_underlying  ///< e.g. stack(queue) built from its container_type
    };

    template<typename Tuple, typename F, std::size_t... I>
    consteval auto tuple_apply_impl(F &&f, std::index_sequence<I...>) {
        return f.template operator()<std::tuple_element_t<I, Tuple>...>();
    }

    template<typename Tuple, typename F>
    consteval auto tuple_apply(F &&f) {
        return tuple_apply_impl<Tuple>(std::forward<F>(f),
                                       std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }

    template<class R>
    struct safe_range_value {
    private:
        template<class T>
        using rm_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

        template<class T>
        static constexpr bool constructible = requires { T{}; };

        static constexpr bool has_value_type =
                requires { typename std::ranges::range_value_t<R>; } &&
                constructible<std::ranges::range_value_t<R>>;

        static constexpr bool has_reference_type =
                requires { typename std::ranges::range_reference_t<R>; } &&
                constructible<rm_cvref<std::ranges::range_reference_t<R>>>;

    public:
        using type =
                std::conditional_t<
                        has_value_type,
                        std::ranges::range_value_t<R>,
                        std::conditional_t<
                                has_reference_type,
                                rm_cvref<std::ranges::range_reference_t<R>>,
                                rm_cvref<decltype(*std::ranges::begin(std::declval<R&>()))>
                        >
                >;
    };

    template<class R>
    using safe_range_value_t = typename safe_range_value<R>::type;

    template<typename C, typename R, typename ArgsTuple = std::tuple<>>
    consteval closable_status compute_closable_status() {
        using Cv = jh::concepts::container_value_t<C>;
        using Rv = safe_range_value_t<R>;
        static_assert(!std::same_as<safe_range_value_t<R>, void>,
                      "safe_range_value_t<R> is void — can't deduce element type!");


        if constexpr (std::same_as<Cv, void> || std::same_as<Rv, void>)
            return closable_status::none;

        constexpr bool type_convertible =
                std::is_constructible_v<Cv, Rv> || std::is_convertible_v<Rv, Cv>;
        if constexpr (!type_convertible)
            return closable_status::none;

        return tuple_apply<ArgsTuple>([]<typename... Args>() consteval {
            // --- direct_copy ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::ranges::begin(r), std::ranges::end(r),
                  std::declval<Args>()...);
            })
                return closable_status::direct_copy;

            // --- direct_move ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::make_move_iterator(std::ranges::begin(r)),
                  std::ranges::end(r),
                  std::declval<Args>()...);
            })
                return closable_status::direct_move;

            // --- via_vector_whole ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r)),
                  std::declval<Args>()...);
            })
                return closable_status::via_vector_whole;

            // --- via_vector_copy ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r)).begin(),
                  std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r)).end(),
                  std::declval<Args>()...);
            })
                return closable_status::via_vector_copy;

            // --- via_vector_move ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::make_move_iterator(std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r)).begin()),
                  std::make_move_iterator(std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r)).end()),
                  std::declval<Args>()...);
            })
                return closable_status::via_vector_move;

            // --- adapter_via_underlying ---
            if constexpr (requires { typename C::container_type; }) {
                using Underlying = typename C::container_type;
                constexpr auto sub = compute_closable_status<Underlying, R, ArgsTuple>();
                if constexpr (sub != closable_status::none) {
                    if constexpr (std::is_constructible_v<C, Underlying> ||
                                  std::is_constructible_v<C, Underlying &&> ||
                                  std::is_constructible_v<C, const Underlying &>)
                        return closable_status::adapter_via_underlying;
                }
            }

            return closable_status::none;
        });
    }

    template<typename C, typename R, typename ArgsTuple = std::tuple<>>
    struct closable_container_for_impl {
        static constexpr closable_status status = compute_closable_status<C, R, ArgsTuple>();
        static constexpr bool value = status != closable_status::none;
    };

} // namespace jh::concepts::detail


// ============================================================================
// Concept interface
// ============================================================================
namespace jh::concepts {

    template<typename C, typename R, typename ArgsTuple = std::tuple<>>
    concept closable_container_for =
    std::ranges::input_range<R> &&
    detail::closable_container_for_impl<C, R, ArgsTuple>::value;

} // namespace jh::concepts
