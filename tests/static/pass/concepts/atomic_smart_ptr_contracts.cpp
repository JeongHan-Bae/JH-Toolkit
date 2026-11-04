#include "jh/atomic_ptr"
#include "jh/immutable_str"
#include <type_traits>

using shared_slot = jh::atomic::shared_ptr<int>;
using weak_slot = jh::atomic::weak_ptr<int>;
static_assert(std::is_class_v<shared_slot> && std::is_class_v<weak_slot>);
static_assert(!std::is_same_v<shared_slot, std::atomic<std::shared_ptr<int>>>);
static_assert(!std::is_same_v<weak_slot, std::atomic<std::weak_ptr<int>>>);
static_assert(std::is_same_v<shared_slot::value_type, std::shared_ptr<int>>);
static_assert(std::is_same_v<weak_slot::value_type, std::weak_ptr<int>>);
static_assert(!std::is_copy_constructible_v<shared_slot> && !std::is_move_constructible_v<shared_slot>);
static_assert(!std::is_copy_assignable_v<shared_slot> && !std::is_move_assignable_v<shared_slot>);
static_assert(!std::is_copy_constructible_v<weak_slot> && !std::is_move_constructible_v<weak_slot>);
static_assert(!std::is_copy_assignable_v<weak_slot> && !std::is_move_assignable_v<weak_slot>);
static_assert(std::is_same_v<jh::shared_str_ptr, std::shared_ptr<jh::immutable_str>>);
static_assert(std::is_same_v<jh::atomic_str_slot, jh::atomic::shared_ptr<jh::immutable_str>>);
static_assert(std::is_same_v<jh::atomic_weak_str_slot, jh::atomic::weak_ptr<jh::immutable_str>>);
static_assert(noexcept(std::declval<shared_slot&>().load()));
static_assert(noexcept(std::declval<weak_slot&>().load()));
static_assert(std::is_same_v<decltype(std::declval<shared_slot&>() = std::shared_ptr<int>{}), shared_slot&>);
static_assert(std::is_same_v<decltype(std::declval<shared_slot&>() = nullptr), shared_slot&>);
static_assert(std::is_same_v<decltype(std::declval<weak_slot&>() = std::weak_ptr<int>{}), weak_slot&>);
