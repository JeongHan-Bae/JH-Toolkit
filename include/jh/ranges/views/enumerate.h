#pragma once

#include "jh/ranges/views/zip.h"

namespace jh::ranges::views {

    template<jh::sequence Seq>
    constexpr auto enumerate(
            Seq &&seq,
            jh::sequence_difference_t<Seq> start = 0
    ) {
        return zip(
                std::views::iota(start),
                std::forward<Seq>(seq)
        );
    }

} // namespace jh::views
