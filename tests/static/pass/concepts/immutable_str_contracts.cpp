#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>

#include "jh/immutable_str"

static_assert(!std::is_copy_constructible_v<jh::immutable_str<>>);
static_assert(!std::is_copy_assignable_v<jh::immutable_str<>>);
static_assert(!std::is_move_constructible_v<jh::immutable_str<>>);
static_assert(!std::is_move_assignable_v<jh::immutable_str<>>);

static_assert(std::is_constructible_v<jh::immutable_str<>, const char *>);
static_assert(!std::is_convertible_v<const char *, jh::immutable_str<>>);
static_assert(!std::is_constructible_v<jh::immutable_str<>, std::string>);
static_assert(!std::is_constructible_v<jh::immutable_str<>, std::string_view>);
static_assert(!std::is_convertible_v<std::string, jh::immutable_str<>>);
static_assert(!std::is_convertible_v<std::string_view, jh::immutable_str<>>);
static_assert(std::is_constructible_v<jh::immutable_str<>, std::string_view, std::mutex &>);

static_assert(jh::immutable_str<>::auto_trim == JH_IMMUTABLE_STR_AUTO_TRIM);
static_assert(jh::immutable_str<true>::auto_trim);
static_assert(!jh::immutable_str<false>::auto_trim);
static_assert(!std::is_same_v<jh::immutable_str<true>, jh::immutable_str<false>>);
static_assert(!std::is_convertible_v<jh::shared_str_ptr<true>, jh::shared_str_ptr<false>>);
static_assert(!std::is_invocable_v<jh::atomic_str_hash<true>, jh::shared_str_ptr<false>>);
static_assert(!std::is_invocable_v<jh::atomic_str_eq<true>, jh::shared_str_ptr<true>, jh::shared_str_ptr<false>>);
static_assert(jh::immutable_str_compatible<jh::shared_str_ptr<>>);
static_assert(jh::immutable_str_compatible<const char *>);
static_assert(jh::immutable_str_compatible<const char (&)[5]>);
static_assert(!jh::immutable_str_compatible<jh::shared_str_ptr<false>, true>);

static_assert(std::is_same_v<decltype(jh::make_atomic_slot("text")), jh::atomic_str_slot<>>);
static_assert(std::is_same_v<decltype(jh::make_atomic_slot(nullptr)), jh::atomic_str_slot<>>);
static_assert(std::is_same_v<decltype(jh::make_atomic_slot<false>("text")), jh::atomic_str_slot<false>>);

static_assert(!std::is_convertible_v<std::string, const char *>);
