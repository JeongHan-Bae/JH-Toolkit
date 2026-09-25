#include <compare>
#include <optional>

#include "jh/conceptual/enum.h"
#include "jh/conceptual/hashable.h"
#include "jh/metax/enum_case.h"

enum class ScopedState { idle, active };
enum UnscopedState { legacy_idle };

static_assert(jh::concepts::enum_type<ScopedState>);
static_assert(jh::concepts::enum_type<UnscopedState>);
static_assert(!jh::concepts::enum_type<int>);
static_assert(jh::concepts::scoped_enum<ScopedState>);
static_assert(!jh::concepts::scoped_enum<UnscopedState>);
static_assert(!jh::concepts::scoped_enum<int>);

using StateCase = jh::meta::enum_case;
using ScopedCase = jh::meta::enum_case_v<ScopedState>;
static_assert(std::same_as<jh::meta::enum_case::value_type<ScopedState>, ScopedCase>);
static_assert(jh::concepts::extended_hashable<ScopedCase>);

static_assert(StateCase::of<ScopedState::idle> == StateCase::of<ScopedState::idle>);
static_assert(StateCase::of<ScopedState::idle> != StateCase::of<ScopedState::active>);
static_assert(StateCase::some<ScopedState> == StateCase::some<ScopedState>);
static_assert(StateCase::any<ScopedState> == StateCase::any<ScopedState>);
static_assert(StateCase::none<ScopedState> == StateCase::none<ScopedState>);
static_assert(StateCase::some<ScopedState> != StateCase::any<ScopedState>);
static_assert((StateCase::of<ScopedState::idle> <=> StateCase::of<ScopedState::active>) ==
              std::strong_ordering::less);
static_assert((StateCase::of<ScopedState::active> <=> StateCase::some<ScopedState>) ==
              std::strong_ordering::less);
static_assert((StateCase::none<ScopedState> <=> StateCase::any<ScopedState>) ==
              std::strong_ordering::greater);

static_assert(StateCase::of<ScopedState::active>.matches(ScopedState::active));
static_assert(StateCase::some<ScopedState>.matches(std::optional{ScopedState::idle}));
static_assert(StateCase::none<ScopedState>.matches(std::optional<ScopedState>{}));
static_assert(StateCase::any<ScopedState>.matches(std::optional{ScopedState::active}));
