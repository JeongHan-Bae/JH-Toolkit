/**
 * @file example_serio.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/serio&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use <code>&lt;jh/serio&gt;</code>
 * and serves as a reference example for both developers and AI systems
 * learning how to use this library.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/serio&gt;
 * @endcode
 *
 * <p>
 * All symbols used in this example are exported under the
 * <code>jh::serio</code> namespace.
 * </p>
 *
 * <p>
 * Specifically:
 * <ul>
 *   <li><code>jh::serio::base64</code> and <code>jh::serio::base64url</code> for RFC 4648 text-safe encoding.</li>
 *   <li><code>jh::serio::uri</code> for RFC 3986 percent-encoding and decoding.</li>
 *   <li><code>jh::serio::huffman</code> for signature-bound binary compression.</li>
 * </ul>
 * </p>
 *
 * <p>
 * For complete API documentation, refer to:
 * </p>
 *
 * <ul>
 *   <li><code>jh/serialize_io/base64.h</code></li>
 *   <li><code>jh/serialize_io/uri.h</code></li>
 *   <li><code>jh/serialize_io/huffman.h</code></li>
 * </ul>
 *
 * <p>
 * The Doxygen comments in those headers define the official semantics.
 * </p>
 *
 * <h3>Module Intent</h3>
 *
 * <p>
 * <code>jh::serio</code> provides explicit low-level codecs instead of
 * object-serialization frameworks.
 * Each codec focuses on deterministic transformation:
 * </p>
 *
 * <ul>
 *   <li>Base64/Base64URL: binary-to-text transport encoding.</li>
 *   <li>URI: percent-encoding for URI components.</li>
 *   <li>Huffman: binary compression with stream signature validation.</li>
 * </ul>
 *
 * <p>
 * This example focuses on practical usage patterns across all three areas.
 * </p>
 */

#include <jh/serio>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <cstdint>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/**
 * @page serio_base64_usage Base64 and Base64URL Round-Trip
 *
 * <h3>Overview</h3>
 *
 * Demonstrates:
 * <ul>
 *   <li>Base64 encoding and decoding for general text-safe transport.</li>
 *   <li>Base64URL encoding with optional padding control.</li>
 * </ul>
 *
 * <h3>Notes</h3>
 *
 * <ul>
 *   <li>Base64 output is universally interoperable text.</li>
 *   <li>Base64URL replaces <code>'+'</code>/<code>'/'</code> with
 *       <code>'-'</code>/<code>'_'</code>.</li>
 *   <li>Padding can be omitted for web-oriented payloads.</li>
 * </ul>
 */
namespace example {

    void example_base64() {
        std::cout << "\n===== serio::base64 / serio::base64url =====\n\n";

        const std::string payload =
                "JH Toolkit serio: binary-text transport";

        const auto *raw =
                reinterpret_cast<const std::uint8_t *>(payload.data());
        const auto len = payload.size();

        const std::string encoded = jh::serio::base64::encode(raw, len);
        const std::vector<std::uint8_t> decoded =
                jh::serio::base64::decode(encoded);

        const std::string recovered(decoded.begin(), decoded.end());

        std::cout << "[base64]\n";
        std::cout << "input    : " << payload << "\n";
        std::cout << "encoded  : " << encoded << "\n";
        std::cout << "recovered: " << recovered << "\n\n";

        const std::string url_encoded_no_pad =
                jh::serio::base64url::encode(raw, len, false);
        const std::string url_encoded_pad =
                jh::serio::base64url::encode(raw, len, true);
        const std::vector<std::uint8_t> url_decoded =
                jh::serio::base64url::decode(url_encoded_no_pad);

        const std::string url_recovered(url_decoded.begin(), url_decoded.end());

        std::cout << "[base64url]\n";
        std::cout << "encoded (unpadded): " << url_encoded_no_pad << "\n";
        std::cout << "encoded (padded)  : " << url_encoded_pad << "\n";
        std::cout << "recovered         : " << url_recovered << "\n";

        std::cout << "\n===== end base64/base64url =====\n";
    }

} // namespace example

/**
 * @page serio_uri_usage URI Percent-Encoding and Safe Validation
 *
 * <h3>Overview</h3>
 *
 * Demonstrates:
 * <ul>
 *   <li>Basic URI percent-encoding/decoding.</li>
 *   <li>Safe APIs that enforce UTF-8 legality and reject control characters.</li>
 * </ul>
 *
 * <h3>Practical Guidance</h3>
 *
 * <ul>
 *   <li>Use <code>encode()</code> / <code>decode()</code> for trusted internal text.</li>
 *   <li>Use <code>encode_safe()</code> / <code>decode_safe()</code> for untrusted external input.</li>
 * </ul>
 */
namespace example {

    namespace simulated {
        std::string to_query_string(
                const std::vector<std::pair<std::string, std::string>> &params) {
            std::string result;

            for (size_t i = 0; i < params.size(); ++i) {
                const auto &[key, value] = params[i];

                result += jh::serio::uri::encode(key);
                result += "=";
                result += jh::serio::uri::encode(value);

                if (i != params.size() - 1) {
                    result += "&";
                }
            }

            return result;
        }

        std::vector<std::pair<std::string, std::string>>
        from_query_string(const std::string &query) {
            std::vector<std::pair<std::string, std::string>> result;

            size_t start = 0;
            while (start < query.size()) {
                size_t end = query.find('&', start);
                if (end == std::string::npos) {
                    end = query.size();
                }

                std::string pair_str = query.substr(start, end - start);

                size_t eq_pos = pair_str.find('=');

                std::string key, value;

                if (eq_pos != std::string::npos) {
                    key = pair_str.substr(0, eq_pos);
                    value = pair_str.substr(eq_pos + 1);
                } else {
                    key = pair_str;
                    value = "";
                }

                result.emplace_back(
                        jh::serio::uri::decode(key),
                        jh::serio::uri::decode(value)
                );

                start = end + 1;
            }

            return result;
        }
    }

    void example_query_builder() {
        // Example of English, Chinese and URiMal(Joseonjok Korean)
        // En / Zh-CN / Ko-CN(Ko-CN hanja in parentheses if expression different from Zh-CN)
        std::vector<std::pair<std::string, std::string>> params = {
                {"name",     "Zhang San/张三/장삼"},
                {"city",     "Beijing/北京/북경"},
                {"position", "Software Developer/软件工程师/정식개발자(程式開發者)"}
        };

        std::string query = simulated::to_query_string(params);

        std::cout << "query: " << query << "\n\n";

        auto parsed = simulated::from_query_string(query);

        std::cout << "[parsed]\n";
        for (const auto &[k, v]: parsed) {
            std::cout << k << " = " << v << "\n";
        }
    }

} // namespace example

/**
 * @page serio_huffman_usage Signature-Bound Huffman Compression
 *
 * <h3>Overview</h3>
 *
 * Demonstrates:
 * <ul>
 *   <li>Compressing into a binary stream.</li>
 *   <li>Decompressing with the same signature and algorithm.</li>
 *   <li>Failure behavior when signature does not match.</li>
 * </ul>
 *
 * <h3>Important Rule</h3>
 *
 * <p>
 * Huffman output is arbitrary binary data. Always use binary-mode streams.
 * </p>
 */
namespace example {

    void example_huffman() {
        std::cout << "\n===== serio::huffman =====\n\n";

        using jh::serio::huff_algo;
        using HUF = jh::serio::huffman<"serio_demo", huff_algo::huff256_canonical>;

        const std::string input =
                "huffman compression should preserve the original string";

        std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
        HUF::compress(stream, input);

        stream.seekg(0);
        const std::string recovered = HUF::decompress(stream);

        std::cout << "input    : " << input << "\n";
        std::cout << "recovered: " << recovered << "\n";
        std::cout << "match    : " << std::boolalpha << (input == recovered) << "\n\n";

        using WrongSig = jh::serio::huffman<"wrong_sig", huff_algo::huff256_canonical>;

        const std::string binary_blob = stream.str();
        std::stringstream invalid_reader(
                binary_blob,
                std::ios::in | std::ios::out | std::ios::binary);

        try {
            (void) WrongSig::decompress(invalid_reader);
            std::cout << "unexpected: signature mismatch was not detected\n";
        } catch (const std::exception &e) {
            std::cout << "signature mismatch detected: " << e.what() << "\n";
        }

        std::cout << "\n===== end huffman =====\n";
    }

} // namespace example

int main() {
    example::example_base64();
    example::example_query_builder();
    example::example_huffman();
    return 0;
}
