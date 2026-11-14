#include <type_traits>
#include "jh/async"

static_assert(!std::is_copy_constructible_v<jh::async::slot>);
static_assert(!std::is_copy_assignable_v<jh::async::slot>);
static_assert(std::is_move_constructible_v<jh::async::slot>);
static_assert(std::is_move_assignable_v<jh::async::slot>);
