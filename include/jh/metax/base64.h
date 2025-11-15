#pragma once

#include <cstdint>
#include "jh/pods/array.h"
#include "jh/metax/t_str.h"
#include "jh/detail/base64_common.h"

namespace jh::meta {
    namespace detail {
        template<TStr S>
        requires (S.is_base64())
        constexpr std::uint16_t decoded_len_base64() {
            auto pad = jh::detail::base64_common::base64_check(S.val(), S.size());
            return jh::detail::base64_common::decoded_len_base64(S.size(), pad);
        }

        template<TStr S>
        requires (S.is_base64url())
        constexpr std::uint16_t decoded_len_base64url() {
            constexpr auto n = S.size();
            constexpr int pad = jh::detail::base64_common::base64url_check(S.val(), n);
            if constexpr (pad > 0) {
                // padded Base64URL : same as padded Base64
                return jh::detail::base64_common::decoded_len_base64(n, static_cast<uint8_t>(pad));
            } else {
                // pad == 0 : unpadded Base64URL
                return jh::detail::base64_common::decoded_len_base64url_no_pad(n);
            }
        }
    }
    template<TStr S>
    requires (S.is_base64())
    constexpr auto decode_base64() {
        constexpr std::uint16_t out_len = detail::decoded_len_base64<S>();
        constexpr std::uint64_t enc_len = S.size();

        jh::pod::array<std::uint8_t, out_len> out{};

        jh::detail::base64_common::base64_decode_unchecked(
                S.val(),
                enc_len,
                out.data,
                out_len
        );

        return out;
    }

    template<TStr S>
    requires (S.is_base64url())
    constexpr auto decode_base64url() {
        constexpr std::uint16_t out_len = detail::decoded_len_base64url<S>();
        constexpr std::uint64_t enc_len = S.size();

        jh::pod::array<std::uint8_t, out_len> out{};

        jh::detail::base64_common::base64_decode_unchecked(
                S.val(),
                enc_len,
                out.data,
                out_len
        );

        return out;
    }

    template<std::uint16_t N>
    constexpr auto encode_base64(const jh::pod::array<std::uint8_t, N>& raw)
    {
        constexpr std::uint64_t raw_len = N;
        constexpr std::uint64_t enc_len =
                jh::detail::base64_common::encoded_len_base64(raw_len);

        // total string size = encoded_length + null
        constexpr std::uint64_t s_len = enc_len + 1;

        jh::pod::array<char, s_len> out_char{};

        jh::detail::base64_common::base64_encode_unchecked<false>(
                raw.data,
                raw_len,
                out_char.data,
                true // padded
        );

        out_char.data[enc_len] = '\0';

        return t_str<s_len>(out_char);
    }

    template<std::uint16_t N, class PadT = std::false_type>
    constexpr auto encode_base64url(const jh::pod::array<std::uint8_t, N>& raw, PadT = {})
    {
        constexpr std::uint64_t raw_len = N;

        constexpr std::uint64_t enc_len_pad =
                jh::detail::base64_common::encoded_len_base64(raw_len);

        constexpr std::uint64_t enc_len_nopad =
                jh::detail::base64_common::encoded_len_base64url_no_pad(raw_len);

        constexpr std::uint64_t enc_len = PadT::value ? enc_len_pad : enc_len_nopad;
        constexpr std::uint64_t s_len = enc_len + 1;

        jh::pod::array<char, s_len> out_char{};

        jh::detail::base64_common::base64_encode_unchecked<true>(
                raw.data,
                raw_len,
                out_char.data,
                PadT::value
        );

        out_char.data[enc_len] = '\0';

        return jh::meta::t_str<s_len>(out_char);
    }

}
