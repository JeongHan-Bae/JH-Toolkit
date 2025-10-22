#pragma once
#include "jh/conceptual/sequence.h"
#include <ranges>
#include <cstdint>
#include "jh/ranges/zip_view.h"


namespace jh::ranges::views {

    /**
     * @brief <b>Zip adaptor</b> that accepts any <code>jh::sequence</code>.
     *
     * @details
     * <p>
     * Converts each argument via <code>jh::to_range()</code> into a range,
     * then constructs a <code>jh::ranges::zip_view</code>.
     * </p>
     *
     * @tparam Seq One or more types satisfying <code>jh::concepts::sequence</code>.
     * @param seqs One or more sequences to be zipped.
     * @return A <code>jh::ranges::zip_view</code> constructed from all given sequences.
     */
    template <jh::concepts::sequence... Seq>
    constexpr auto zip(Seq&&... seqs) {
        return jh::ranges::zip_view{
                std::views::all(jh::to_range(seqs))...
        };
    }

} // namespace jh::views
