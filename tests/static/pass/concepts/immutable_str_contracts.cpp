#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>

#include "jh/immutable_str"

static_assert(!std::is_copy_constructible_v<jh::immutable_str>);
static_assert(!std::is_copy_assignable_v<jh::immutable_str>);
static_assert(!std::is_move_constructible_v<jh::immutable_str>);
static_assert(!std::is_move_assignable_v<jh::immutable_str>);

static_assert(std::is_constructible_v<jh::immutable_str, const char *>);
static_assert(!std::is_convertible_v<const char *, jh::immutable_str>);
static_assert(!std::is_constructible_v<jh::immutable_str, std::string>);
static_assert(!std::is_constructible_v<jh::immutable_str, std::string_view>);
static_assert(!std::is_convertible_v<std::string, jh::immutable_str>);
static_assert(!std::is_convertible_v<std::string_view, jh::immutable_str>);
static_assert(std::is_constructible_v<jh::immutable_str, std::string_view, std::mutex &>);
