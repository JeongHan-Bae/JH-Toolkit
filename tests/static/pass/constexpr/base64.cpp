#include <cstdint>
#include <type_traits>

#include "jh/jindallae"
#include "jh/pod"

static_assert(jh::jindallae::decode_base64<"SGVsbG8=">() ==
              jh::pod::array<std::uint8_t, 5>{{'H', 'e', 'l', 'l', 'o'}});
static_assert(jh::jindallae::decode_base64url<"SGVsbG8=">() ==
              jh::pod::array<std::uint8_t, 5>{{'H', 'e', 'l', 'l', 'o'}});
static_assert(jh::jindallae::decode_base64url<"SGVsbG8">() ==
              jh::pod::array<std::uint8_t, 5>{{'H', 'e', 'l', 'l', 'o'}});

constexpr jh::pod::array<std::uint8_t, 3> hi_bytes{{'H', 'i', '!'}};
static_assert(jh::jindallae::encode_base64(hi_bytes) == jh::jindallae::t_str<5>("SGkh"));
static_assert(jh::jindallae::encode_base64url(hi_bytes, std::false_type{}) ==
              jh::jindallae::t_str<5>("SGkh"));
static_assert(jh::jindallae::encode_base64url(hi_bytes, std::true_type{}) ==
              jh::jindallae::t_str<5>("SGkh"));

constexpr auto base64_decoded = jh::jindallae::decode_base64<"QUJD">();
static_assert(jh::jindallae::encode_base64(base64_decoded) == jh::jindallae::t_str<5>("QUJD"));
constexpr auto url_decoded = jh::jindallae::decode_base64url<"QQ">();
static_assert(jh::jindallae::encode_base64url(url_decoded) == jh::jindallae::t_str<3>("QQ"));

constexpr jh::jindallae::t_str hello{"Hello"};
constexpr auto hello_bytes = hello.to_bytes();
constexpr auto hello_encoded = jh::jindallae::encode_base64(hello_bytes);
constexpr auto hello_restored = jh::jindallae::t_str{jh::jindallae::decode_base64<hello_encoded>()};
static_assert(hello_restored == hello);

constexpr auto hello_literal_bytes = jh::jindallae::decode_base64<"SGVsbG8=">();
constexpr auto hello_literal = jh::jindallae::t_str{hello_literal_bytes};
constexpr auto hello_literal_encoded = jh::jindallae::encode_base64(hello_literal.to_bytes());
static_assert(hello_literal_encoded == jh::jindallae::t_str("SGVsbG8="));
