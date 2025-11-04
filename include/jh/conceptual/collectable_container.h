#pragma once

#include <cstdint>
#include "jh/conceptual/closable_container.h"


namespace jh::concepts::detail {

    enum class collectable_status : std::uint8_t {
        none = 0,
        closable,      // can be constructed directly via jh::ranges::to
        insert,        // supports c.insert(value)
        emplace,       // supports c.emplace(value)
        emplace_back,  // supports c.emplace_back(value)
    };

    template<typename C, typename R>
    consteval collectable_status compute_collectable_status() {
        // 1. prior: closable
        if constexpr (jh::concepts::closable_container_for<C, R>)
            return collectable_status::closable;

        using V = std::ranges::range_value_t<R>;

        // 2. fallback: insert
        if constexpr (std::default_initializable<C> &&
                      requires([[maybe_unused]] C &c, [[maybe_unused]] V v) { c.insert(v); })
            return collectable_status::insert;

        // 3. fallback: emplace
        if constexpr (std::default_initializable<C> &&
                      requires([[maybe_unused]] C &c, [[maybe_unused]] V v) { c.emplace(v); })
            return collectable_status::emplace;

        // 4. fallback: emplace_back
        if constexpr (std::default_initializable<C> &&
                      requires([[maybe_unused]] C &c, [[maybe_unused]] V v) { c.emplace_back(v); })
            return collectable_status::emplace_back;

        // 5. none
        return collectable_status::none;
    }

    template<typename C, typename R>
    struct collectable_container_for_impl {
        static constexpr collectable_status status = compute_collectable_status<C, R>();
        static constexpr bool value = status != collectable_status::none;
    };

} // namespace jh::concepts::detail

namespace jh::concepts {

    template<typename C, typename R>
    concept collectable_container_for =
    std::ranges::input_range<R> &&
    detail::collectable_container_for_impl<C, R>::value;
}
