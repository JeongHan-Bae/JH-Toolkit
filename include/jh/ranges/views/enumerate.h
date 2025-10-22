#pragma once

#include "jh/ranges/views/zip.h"

namespace jh::ranges::views {

    /**
     * @brief <b>Enumerate adaptor</b> that pairs each element of a sequence with an incrementing index.
     *
     * @details
     * <p>
     * Returns a zipped range combining an index sequence
     * (generated using <code>std::views::iota(start)</code>) with the given sequence.
     * </p>
     *
     * <p>
     * Each element of the resulting range is a pair <code>(index, value)</code>,
     * where <code>index</code> starts from <code>start</code> and increases by 1 for each element.
     * </p>
     *
     * @tparam Seq The sequence type satisfying <code>jh::concepts::sequence</code>.
     * @param seq The input sequence to be enumerated.
     * @param start The starting index (default is <code>0</code>).
     * @return A <code>jh::ranges::zip_view</code> that zips indices with elements of the sequence.
     */
    template<jh::concepts::sequence Seq>
    constexpr auto enumerate(
            Seq &&seq,
            jh::concepts::sequence_difference_t<Seq> start = 0
    ) {
        return zip(
                std::views::iota(start),
                std::forward<Seq>(seq)
        );
    }

} // namespace jh::views
