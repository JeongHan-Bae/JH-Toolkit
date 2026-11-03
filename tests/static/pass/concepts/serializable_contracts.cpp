#include "jh/concepts"
#include "jh/bloom_filter"

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

struct string_only {
    explicit string_only(const std::string &) {}
    explicit operator std::string() const { return "value"; }
};

struct stream_only {
    int value = 0;
};

std::ostream &operator<<(std::ostream &out, const stream_only &value);
std::istream &operator>>(std::istream &in, stream_only &value);

struct string_output_only {
    explicit operator std::string() const { return "value"; }
};

struct string_input_only {
    explicit string_input_only(const std::string &) {}
};

struct borrowed_string_type {
    std::string_view view;
    explicit borrowed_string_type(const std::string &text) : view(text) {}
    explicit operator std::string() const { return std::string{view}; }
};

struct stream_output_only {};
std::ostream &operator<<(std::ostream &out, const stream_output_only &);

struct stream_input_only {};
std::istream &operator>>(std::istream &in, stream_input_only &);

struct immovable_stream {
    immovable_stream() = default;
    immovable_stream(const immovable_stream &) = delete;
    immovable_stream(immovable_stream &&) = delete;
};
std::ostream &operator<<(std::ostream &out, const immovable_stream &);
std::istream &operator>>(std::istream &in, immovable_stream &);

static_assert(jh::concepts::serializable<std::string>);
static_assert(jh::concepts::serializable<int>);
static_assert(jh::concepts::serializable<string_only>);
static_assert(jh::concepts::serializable<stream_only>);
static_assert(!jh::concepts::serializable<string_output_only>);
static_assert(!jh::concepts::serializable<string_input_only>);
static_assert(!jh::concepts::serializable<stream_output_only>);
static_assert(!jh::concepts::serializable<stream_input_only>);
static_assert(jh::concepts::serializable<immovable_stream>);
static_assert(jh::concepts::serializable<std::string_view>);
static_assert(jh::concepts::serializable<borrowed_string_type>);

template<typename T>
concept deserializes_lvalue_string = requires(const std::string &text) {
    jh::serio::deserialize<T>(text);
};
template<typename T>
concept deserializes_temporary_string = requires {
    jh::serio::deserialize<T>(std::string{"key"});
};
template<typename T>
concept deserializes_literal_string = requires {
    jh::serio::deserialize<T>("key");
};
static_assert(deserializes_lvalue_string<string_only>);
static_assert(deserializes_lvalue_string<std::string>);
static_assert(deserializes_lvalue_string<int>);
static_assert(deserializes_lvalue_string<std::string_view>);
static_assert(deserializes_lvalue_string<borrowed_string_type>);
static_assert(!deserializes_lvalue_string<immovable_stream>);
static_assert(!deserializes_temporary_string<std::string>);
static_assert(!deserializes_temporary_string<int>);
static_assert(!deserializes_temporary_string<std::string_view>);
static_assert(!deserializes_temporary_string<borrowed_string_type>);
static_assert(!deserializes_literal_string<int>);
static_assert(!deserializes_literal_string<std::string_view>);

template<typename T>
concept deserializes_into_existing = requires(const std::string &text, T &value) {
    jh::serio::deserialize(text, value);
};
static_assert(deserializes_into_existing<immovable_stream>);
template<typename T>
concept deserializes_temporary_into_existing = requires(T &value) {
    jh::serio::deserialize(std::string{"key"}, value);
};
static_assert(!deserializes_temporary_into_existing<immovable_stream>);

struct invalid_type_hash {
    auto operator()(const string_output_only &) const {
        return std::tuple{std::size_t{1}};
    }
};

using invalid_filter = jh::bloom_filter<string_output_only, 65,
        jh::typed::monostate, invalid_type_hash{}>;
template<typename Filter>
concept can_persist = requires {
    sizeof(jh::bloom_persistence<"hello", Filter>);
};
static_assert(!can_persist<invalid_filter>);

struct sentinel_hash_a {
    auto operator()(const std::string &value) const {
        return std::tuple{value.size(), value.size() + 1};
    }
};

struct sentinel_hash_b {
    auto operator()(const std::string &value) const {
        return std::tuple{value.size() + 2, value.size() + 3};
    }
};

using filter = jh::bloom_filter<std::string, 65, jh::typed::monostate,
        sentinel_hash_a{}, sentinel_hash_b{}>;
using persistence = jh::bloom_persistence<"hello", filter>;
template<typename Candidate>
concept accepts_bloom_persistence = requires {
    typename jh::bloom_persistence<"hello", Candidate>;
};
static_assert(accepts_bloom_persistence<filter>);
static_assert(!accepts_bloom_persistence<int>);
static_assert(!accepts_bloom_persistence<const filter>);
static_assert(persistence::signature_bytes == 5);
static_assert(persistence::signature_words == 1);
static_assert(persistence::hash_count == 4);
static_assert(persistence::word_count == 2);
static_assert(persistence::serialized_size == 80);
