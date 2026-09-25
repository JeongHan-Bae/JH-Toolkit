#include <type_traits>

#include "jh/immutable_str"

static_assert(!std::is_copy_constructible_v<jh::immutable_str>);
static_assert(!std::is_copy_assignable_v<jh::immutable_str>);
static_assert(!std::is_move_constructible_v<jh::immutable_str>);
static_assert(!std::is_move_assignable_v<jh::immutable_str>);

