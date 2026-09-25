#include "jh/conceptual/sequence.h"
#include "jh/pod"

static_assert(jh::concepts::sequence<jh::pod::array<int, 4>>);
static_assert(!jh::concepts::sequence<jh::pod::bytes_view>);
static_assert(jh::concepts::sequence<jh::pod::span<int>>);
static_assert(jh::concepts::sequence<jh::pod::string_view>);
static_assert(!jh::concepts::sequence<jh::pod::bitflags<64>>);

