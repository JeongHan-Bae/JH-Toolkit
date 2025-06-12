#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include "../pods/array.h"
#include "../pods/string_view.h"
#include "../pods/bytes_view.h"

namespace jh::utils::base64 {

    static constexpr char kBase64Chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

    constexpr bool is_base64_char(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '+' || c == '/' || c == '=';
    }

    namespace detail {

        consteval auto make_base64_decode_table() {
            jh::pod::array<uint8_t, 256> table{};

            // initialize as illegal
            for (int i = 0; i < 256; ++i) {
                table[i] = 64;
            }

            // A–Z
            for (int i = 'A'; i <= 'Z'; ++i) {
                table[i] = i - 'A';
            }

            // a–z
            for (int i = 'a'; i <= 'z'; ++i) {
                table[i] = i - 'a' + 26;
            }

            // 0–9
            for (int i = '0'; i <= '9'; ++i) {
                table[i] = i - '0' + 52;
            }

            table['+'] = 62;
            table['/'] = 63;

            return table;
        }

        static constexpr auto kDecodeTable = detail::make_base64_decode_table();

        inline std::vector<uint8_t> decode_base(const std::string &input, std::vector<uint8_t>& output) {
            const std::size_t len = input.size();

            // Length must be multiple of 4
            if (len % 4 != 0) {
                throw std::runtime_error("Invalid base64: input length must be multiple of 4");
            }

            // Validate all characters
            if (std::any_of(input.begin(), input.end(), [](char c) {
                return !is_base64_char(c);
            })) {
                throw std::runtime_error("Invalid base64: contains illegal characters");
            }

            output.reserve(input.size() * 3 / 4);

            for (size_t i = 0; i < input.size(); i += 4) {
                uint32_t val = 0;
                int pad = 0;

                for (int j = 0; j < 4; ++j) {
                    char c = input[i + j];
                    if (c == '=') {
                        val <<= 6;
                        pad++;
                    } else {
                        uint8_t decoded = kDecodeTable[static_cast<unsigned char>(c)];
                        if (decoded == 64) throw std::runtime_error("Invalid character in base64");
                        val = (val << 6) | decoded;
                    }
                }

                output.push_back((val >> 16) & 0xFF);
                if (pad < 2) output.push_back((val >> 8) & 0xFF);
                if (pad < 1) output.push_back(val & 0xFF);
            }

            return output;
        }
    }


    /**
     * @brief Encode binary data into a Base64 string
     * @param data Pointer to input data
     * @param len Length of input data
     * @return Base64-encoded string
     */
    [[maybe_unused]] inline std::string encode(const uint8_t *data, std::size_t len) noexcept {
        std::vector<uint8_t> out;
        out.reserve((len + 2) / 3 * 4);  // estimate output size

        for (std::size_t i = 0; i < len; i += 3) {
            uint32_t chunk = 0;
            int pad = 0;

            chunk |= data[i] << 16;
            if (i + 1 < len) {
                chunk |= data[i + 1] << 8;
            } else {
                pad++;
            }
            if (i + 2 < len) {
                chunk |= data[i + 2];
            } else {
                pad++;
            }

            out.push_back(kBase64Chars[(chunk >> 18) & 0x3F]);
            out.push_back(kBase64Chars[(chunk >> 12) & 0x3F]);
            out.push_back(pad >= 2 ? '=' : kBase64Chars[(chunk >> 6) & 0x3F]);
            out.push_back(pad >= 1 ? '=' : kBase64Chars[chunk & 0x3F]);
        }

        return {out.begin(), out.end()};
    }

    /**
     * @brief Decode a Base64 string into a byte vector
     * @param input Base64-encoded string
     * @return Decoded byte vector
     */
    [[maybe_unused]] inline std::vector<uint8_t> decode(const std::string &input) {
        std::vector<uint8_t> output;
        detail::decode_base(input, output);
        return output;
    }

    /**
     * @brief Decode a Base64 string into a reusable byte buffer and return a view.
     *
     * This version decodes the input into the provided `output_buffer` and returns
     * a `bytes_view` pointing to that buffer. The caller must ensure that
     * `output_buffer` remains alive for the duration of the view.
     *
     * @param input Base64-encoded string
     * @param output_buffer Target buffer to store decoded bytes (will be cleared and written)
     * @return `bytes_view` into the decoded content (points to `output_buffer.data()`)
     *
     * @warning Do not use the returned view after `output_buffer` is modified or destroyed.
     * @note This is efficient for zero-copy access patterns, especially in POD systems.
     */
    [[maybe_unused]] inline jh::pod::bytes_view decode(const std::string &input, std::vector<uint8_t>& output_buffer) {
        output_buffer.clear();
        detail::decode_base(input, output_buffer);
        return jh::pod::bytes_view::from(output_buffer.data(), output_buffer.size());
    }

    /**
     * @brief Decode a Base64 string into a `std::string` and return a `string_view`.
     *
     * This version is suitable for text content decoding. It decodes into the
     * provided `output_buffer` and returns a view into its contents. The buffer
     * must remain valid as long as the returned `string_view` is in use.
     *
     * @param input Base64-encoded string
     * @param output_buffer Target string to store decoded data (will be overwritten)
     * @return `string_view` pointing to the decoded text content
     *
     * @warning The returned `string_view` becomes invalid if `output_buffer` is destroyed or changed.
     * @note Use this for UTF-8 or plain text decoding into reusable `std::string` buffers.
     */
    [[maybe_unused]] inline jh::pod::string_view decode(const std::string &input, std::string& output_buffer) {
        std::vector<uint8_t> output;
        detail::decode_base(input, output);
        output_buffer.assign(output.begin(), output.end());
        return {output_buffer.data(), output_buffer.size()};
    }

} // namespace jh::utils::base64