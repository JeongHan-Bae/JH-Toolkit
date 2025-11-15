#define CATCH_CONFIG_MAIN

#include <random>
#include <catch2/catch_all.hpp>
#include "jh/serio"

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

    REQUIRE(HUF::decompress(ss) == input);
}

// run correctness 4 times with different random inputs
template<jh::meta::TStr Sig, jh::serio::huff_algo Algo>
static void verify_correctness_4(size_t n, bool ascii = true) {
    for (int i = 0; i < 4; i++)
        verify_correctness_once<Sig, Algo>(n, ascii);
}

// Benchmark (must reuse SAME input for fairness)
template<jh::meta::TStr Sig, jh::serio::huff_algo Algo>
static void bench_compress(std::string_view input) {
    using HUF = jh::serio::huffman<Sig, Algo>;
    BENCHMARK("compress " + std::string(Sig.view())) {
                                                         std::stringstream ss(std::ios::in | std::ios::out |
                                                                                             std::ios::binary);
                                                         HUF::compress(ss, input);
                                                     };
}

template<jh::meta::TStr Sig, jh::serio::huff_algo Algo>
static void bench_decompress(std::stringstream &prepared) {
    using HUF = jh::serio::huffman<Sig, Algo>;

    BENCHMARK("decompress " + std::string(Sig.view())) {
                                                           std::stringstream ss_copy(prepared.str());
                                                           return HUF::decompress(ss_copy);
                                                       };
}

TEST_CASE("Huffman ASCII correctness") {
    constexpr size_t N = 20000;

    verify_correctness_4<"serio_huff128", jh::serio::huff_algo::huff128>(N);
    verify_correctness_4<"serio_huff128can", jh::serio::huff_algo::huff128_canonical>(N);
    verify_correctness_4<"serio_huff256", jh::serio::huff_algo::huff256>(N);
    verify_correctness_4<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(N);
}

TEST_CASE("Huffman BYTE correctness") {
    constexpr size_t N = 20000;

    verify_correctness_4<"serio_huff256", jh::serio::huff_algo::huff256>(N, false);
    verify_correctness_4<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(N, false);
}

TEST_CASE("Base64 + Huff128Canonical roundtrip") {
    constexpr size_t N = 20000;

    for (int i = 0; i < 4; i++) {
        // 1) random raw bytes
        std::string raw = random_bytes(N);

        std::vector<uint8_t> raw_bytes(raw.begin(), raw.end());

        // 2) base64 encode → string
        std::string b64 = jh::serio::base64::encode(raw_bytes.data(), raw_bytes.size());

        using HUF = jh::serio::huffman<
                "mixed_huff128can",
                jh::serio::huff_algo::huff128_canonical
        >;

        // 3) Huffman compress encoded base64
        std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
        HUF::compress(ss, b64);
        ss.seekg(0);

        // 4) Huffman decompress → base64
        std::string b64_out = HUF::decompress(ss);

        // 5) Base64 decode → string
        std::string raw_out;
        jh::serio::base64::decode(b64_out, raw_out);

        REQUIRE(raw_out == raw);
    }
}

TEST_CASE("Huffman benchmark") {
    constexpr size_t N = 200'000;

    // -------- ASCII Benchmark --------
    {
        std::string data = random_ascii(N);

        bench_compress<"serio_huff128", jh::serio::huff_algo::huff128>(data);
        bench_compress<"serio_huff128can", jh::serio::huff_algo::huff128_canonical>(data);
        bench_compress<"serio_huff256", jh::serio::huff_algo::huff256>(data);
        bench_compress<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(data);

        {
            std::stringstream ss;
            jh::serio::huffman<"serio_huff128", jh::serio::huff_algo::huff128>::compress(ss, data);
            ss.seekg(0);
            bench_decompress<"serio_huff128", jh::serio::huff_algo::huff128>(ss);
        }
        {
            std::stringstream ss;
            jh::serio::huffman<"serio_huff128can", jh::serio::huff_algo::huff128_canonical>::compress(ss, data);
            ss.seekg(0);
            bench_decompress<"serio_huff128can", jh::serio::huff_algo::huff128_canonical>(ss);
        }
        {
            std::stringstream ss;
            jh::serio::huffman<"serio_huff256", jh::serio::huff_algo::huff256>::compress(ss, data);
            ss.seekg(0);
            bench_decompress<"serio_huff256", jh::serio::huff_algo::huff256>(ss);
        }
        {
            std::stringstream ss;
            jh::serio::huffman<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>::compress(ss, data);
            ss.seekg(0);
            bench_decompress<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(ss);
        }
    }

    // -------- BYTE Benchmark --------
    {
        std::string data = random_bytes(N);

        bench_compress<"serio_huff256", jh::serio::huff_algo::huff256>(data);
        bench_compress<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(data);

        {
            std::stringstream ss;
            jh::serio::huffman<"serio_huff256", jh::serio::huff_algo::huff256>::compress(ss, data);
            ss.seekg(0);
            bench_decompress<"serio_huff256", jh::serio::huff_algo::huff256>(ss);
        }
        {
            std::stringstream ss;
            jh::serio::huffman<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>::compress(ss, data);
            ss.seekg(0);
            bench_decompress<"serio_huff256can", jh::serio::huff_algo::huff256_canonical>(ss);
        }
    }
}
