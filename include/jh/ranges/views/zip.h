#pragma once
#include "jh/sequence"
#include <ranges>
#include <cstdint>
#include "jh/ranges/zip_view.h"


namespace jh::ranges::views {

    /**
     * @brief Zip adaptor that accepts any jh::sequence.
     *
     * @details
     * Converts each argument via `jh::to_range()` into a range,
     * then constructs a `jh::ranges::zip_view`.
     *
     * Works seamlessly with `jh::pod::array` and any other `sequence`.
     */
    template <jh::concepts::sequence... Seq>
    constexpr auto zip(Seq&&... seqs) {
        return jh::ranges::zip_view{
                std::views::all(jh::to_range(seqs))...
        };
    }

} // namespace jh::views
