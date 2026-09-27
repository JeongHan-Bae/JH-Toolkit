#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <random>
#include <stdexcept>
#include "jh/serio"
#include "jh/pod"

#include <sstream>


// Random helpers (fresh randomness)
static std::string random_ascii(size_t n) {
    static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 127);
    std::string s;
    s.reserve(n);
    for (size_t i = 0; i < n; i++) s.push_back(char(dist(rng)));
    return s;
}

static std::string random_bytes(size_t n) {
    static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 255);
    std::string s;
    s.reserve(n);
    for (size_t i = 0; i < n; i++) s.push_back(char(dist(rng)));
    return s;
}

// Correctness test (each time new random input)
template<jh::meta::TStr Sig, jh::serio::huff_algo Algo>
static void verify_correctness_once(size_t n, bool ascii) {
    std::string input;

    if (ascii)
        input = random_ascii(n);
    else
        input = random_bytes(n);

    using HUF = jh::serio::huffman<Sig, Algo>;

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    HUF::compress(ss, input);
    ss.seekg(0);

    jh::test::tiny_test::expect(static_cast<bool>((HUF::decompress(ss) == input)), "HUF::decompress(ss) == input");
}

// run correctness 4 times with different random inputs
template<jh::meta::TStr Sig, jh::serio::huff_algo Algo>
static void verify_correctness_4(size_t n, bool ascii = true) {
    for (int i = 0; i < 4; i++)
        verify_correctness_once<Sig, Algo>(n, ascii);
}

template<jh::meta::TStr Sig, jh::serio::huff_algo Algo>
static void verify_single_symbol_roundtrip() {
    using HUF = jh::serio::huffman<Sig, Algo>;
    const std::string input(8, 'A');

    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    HUF::compress(stream, input);
    stream.seekg(0);

    jh::test::tiny_test::expect(static_cast<bool>((HUF::decompress(stream) == input)), "HUF::decompress(stream) == input");
}

JH_POD_STRUCT(Payload,
              int x;
                      double y;
                      jh::pod::array<char, 12> msg;
);

Payload random_payload(std::mt19937 &rng) {
    std::uniform_int_distribution<int> di(0, 1000000);
    std::uniform_real_distribution<double> dd(0.0, 1000000.0);
    std::uniform_int_distribution<int> len_dist(1, 11);
    std::uniform_int_distribution<int> char_dist(97, 122); // a-z

    Payload p{};
    p.x = di(rng);
    p.y = dd(rng);

    int L = len_dist(rng);
    for (int i = 0; i < L; i++)
        p.msg[i] = static_cast<char>(char_dist(rng));
    p.msg[L] = '\0';

    return p;
}


namespace test {
void tiny_test_case_1() {
    constexpr size_t N = 256;

    verify_correctness_4<"serio_huff128", jh::serio::huff_algo::huff128>(N);
    verify_correctness_4<"serio_huff128can", jh::serio::huff_algo::huff128_canonical>(N);
    verify_correctness_4<"serio_huff256", jh::serio::huff_algo::huff256>(N);
    verify_correctness_4<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(N);

}
}
template<>
struct jh::test::tiny_test::test<"Huffman ASCII correctness">
    : jh::test::tiny_test::test_definition<"Huffman ASCII correctness", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_huffman case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_huffman case 1", jh::test::tiny_test::test<"Huffman ASCII correctness">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_huffman case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    constexpr size_t N = 256;

    verify_correctness_4<"serio_huff256", jh::serio::huff_algo::huff256>(N, false);
    verify_correctness_4<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(N, false);

}
}
template<>
struct jh::test::tiny_test::test<"Huffman BYTE correctness">
    : jh::test::tiny_test::test_definition<"Huffman BYTE correctness", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_huffman case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_huffman case 2", jh::test::tiny_test::test<"Huffman BYTE correctness">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_huffman case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    verify_single_symbol_roundtrip<"single_huff128", jh::serio::huff_algo::huff128>();
    verify_single_symbol_roundtrip<"single_huff128_canonical", jh::serio::huff_algo::huff128_canonical>();
    verify_single_symbol_roundtrip<"single_huff256", jh::serio::huff_algo::huff256>();
    verify_single_symbol_roundtrip<"single_huff256_canonical", jh::serio::huff_algo::huff256_canonical>();

}
}
template<>
struct jh::test::tiny_test::test<"Huffman preserves a payload containing one repeated symbol">
    : jh::test::tiny_test::test_definition<"Huffman preserves a payload containing one repeated symbol", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_huffman case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_huffman case 3", jh::test::tiny_test::test<"Huffman preserves a payload containing one repeated symbol">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_huffman case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
    constexpr size_t N = 256;

    for (int i = 0; i < 4; i++) {
        // 1) random raw bytes
        std::string raw = random_bytes(N);

        std::vector<uint8_t> raw_bytes(raw.begin(), raw.end());

        // 2) base64 encode -> string
        std::string b64 = jh::serio::base64::encode(raw_bytes.data(), raw_bytes.size());

        using HUF = jh::serio::huffman<
                "mixed_huff128can",
                jh::serio::huff_algo::huff128_canonical
        >;

        // 3) Huffman compress encoded base64
        std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
        HUF::compress(ss, b64);
        ss.seekg(0);

        // 4) Huffman decompress -> base64
        std::string b64_out = HUF::decompress(ss);

        // 5) Base64 decode -> string
        std::string raw_out;
        jh::serio::base64::decode(b64_out, raw_out);

        jh::test::tiny_test::expect(static_cast<bool>((raw_out == raw)), "raw_out == raw");
    }

}
}
template<>
struct jh::test::tiny_test::test<"Base64 + Huff128Canonical roundtrip">
    : jh::test::tiny_test::test_definition<"Base64 + Huff128Canonical roundtrip", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_huffman case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_huffman case 4", jh::test::tiny_test::test<"Base64 + Huff128Canonical roundtrip">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_huffman case 4"> registration_4{};
}



namespace test {
void tiny_test_case_5() {
    using Compress = jh::serio::huffman<"sig_one", jh::serio::huff_algo::huff256_canonical>;
    using Decompress = jh::serio::huffman<"sig_two", jh::serio::huff_algo::huff256_canonical>;

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    Compress::compress(ss, "signature check");
    ss.seekg(0);

    jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(Decompress::decompress(ss)); }, "throws std::runtime_error: Decompress::decompress(ss)");

}
}
template<>
struct jh::test::tiny_test::test<"Huffman signature mismatch is rejected">
    : jh::test::tiny_test::test_definition<"Huffman signature mismatch is rejected", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_huffman case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_huffman case 5", jh::test::tiny_test::test<"Huffman signature mismatch is rejected">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_huffman case 5"> registration_5{};
}


__attribute__((noinline))
static jh::pod::bytes_view make_view(Payload* data, size_t n) {
    return jh::pod::bytes_view::from(data, n);
}


namespace test {
void tiny_test_case_6() {
    using HUF = jh::serio::huffman<
            "payload_demo",
            jh::serio::huff_algo::huff256_canonical
    >;

    constexpr size_t N = 8;
    std::mt19937 rng(12345);

    // -------- 1) random Payloads --------
    static std::vector<Payload> vec(N);
    for (auto &p: vec)
        p = random_payload(rng);

    // -------- 2) as bytes_view -> string_view --------
    auto bv = make_view(vec.data(), vec.size());
    const auto source_data = bv.fetch<char>();
    jh::test::tiny_test::expect(static_cast<bool>((source_data)), "source_data");
    std::string_view sv(source_data.value(), bv.len);

    // -------- 3) compress with ostringstream --------
    std::ostringstream out(std::ios::binary);
    HUF::compress(out, sv);
    std::string compressed = out.str();

    // -------- 4) decompress with istringstream --------
    std::istringstream in(compressed, std::ios::binary);
    std::string decompressed = HUF::decompress(in);

    jh::test::tiny_test::expect(static_cast<bool>((decompressed.size() == bv.len)), "decompressed.size() == bv.len");

    // -------- 5) bytes_view -> Payload --------
    jh::pod::bytes_view bv2 = jh::pod::bytes_view::from(decompressed.data(), decompressed.size());
    std::vector<Payload> vec2(N);
    const auto decoded_data = bv2.fetch<Payload>();
    jh::test::tiny_test::expect(static_cast<bool>((decoded_data)), "decoded_data");

    std::copy(
            decoded_data.value(),
            decoded_data.value() + N,
            vec2.data()
    );

    // -------- 6) compare --------
    jh::test::tiny_test::expect(static_cast<bool>((vec == vec2)), "vec == vec2");

}
}
template<>
struct jh::test::tiny_test::test<"POD Payload roundtrip via huff256_canonical (binary o/istringstream)">
    : jh::test::tiny_test::test_definition<"POD Payload roundtrip via huff256_canonical (binary o/istringstream)", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_huffman case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_huffman case 6", jh::test::tiny_test::test<"POD Payload roundtrip via huff256_canonical (binary o/istringstream)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_huffman case 6"> registration_6{};
}

