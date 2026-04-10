/**
 * @file example_pod.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/pod&gt;</code>.
 *
 * <p>
 * This file demonstrates practical, docs-aligned usage of the POD module in
 * JH Toolkit. It focuses on layout-stable data contracts, non-owning views,
 * and deterministic byte-level behavior.
 * </p>
 *
 * <p>
 * Canonical include:
 * </p>
 *
 * @code
 * #include &lt;jh/pod&gt;
 * @endcode
 *
 * <h3>Why This Module Exists</h3>
 *
 * <p>
 * <code>jh::pod</code> provides trivially copyable, standard-layout building
 * blocks for binary-safe systems programming, where predictable memory layout
 * matters more than rich runtime object behavior.
 * </p>
 *
 * <h3>Usage Philosophy</h3>
 *
 * <ul>
 *   <li>Treat POD types as explicit memory contracts.</li>
 *   <li>Prefer non-owning views (<code>span</code>, <code>string_view</code>, <code>bytes_view</code>) for observation.</li>
 *   <li>Use helper factories (<code>make_optional</code>, <code>make_pair</code>, <code>make_tuple</code>) for portable clarity.</li>
 *   <li>Use byte-oriented APIs deliberately and with clear bounds assumptions.</li>
 * </ul>
 */

#include <jh/pod>

#include <array>
#include <compare>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

namespace example {

    using namespace jh;

    JH_POD_STRUCT(SensorPoint,
                  std::uint32_t id;
                          std::uint16_t reading;
    );

    struct ManualPacket {
        std::uint16_t tag;
        std::uint32_t payload;
    };
    JH_ASSERT_POD_LIKE(ManualPacket);

    struct method_buffer {
        std::array<int, 5> storage{};

        constexpr int *data() noexcept { return storage.data(); }

        [[nodiscard]] constexpr const int *data() const noexcept { return storage.data(); }

        [[nodiscard]] constexpr std::uint64_t size() const noexcept {
            return static_cast<std::uint64_t>(storage.size());
        }
    };

    namespace adl_span_demo {

        struct buffer {
            std::array<int, 4> storage{};
        };

        [[nodiscard]] constexpr const int *get_data(const buffer &b) noexcept {
            return b.storage.data();
        }

        [[nodiscard]] constexpr std::uint64_t get_size(const buffer &b) noexcept {
            return static_cast<std::uint64_t>(b.storage.size());
        }

    } // namespace adl_span_demo

/**
 * @brief Fixed-size inline storage, CTAD, tuple-like access for <code>pod::array</code>.
 */
    void array_usage() {
        std::cout << "\n[1] pod::array\n";

        pod::array<int, 4> arr = {{1, 2, 3, 4}};
        arr[2] = 99;

        const pod::array ctad_numbers = {{7, 8, 9}};
        const pod::array ctad_text = {"POD"};

        const auto [x, y, z] = ctad_numbers;

        std::cout << "arr: ";
        for (int v: arr) {
            std::cout << v << " ";
        }
        std::cout << "\n";

        std::cout << "ctad_numbers size=" << ctad_numbers.size()
                  << ", unpacked=" << x << "," << y << "," << z << "\n";
        std::cout << "ctad_text(raw) = " << ctad_text << "\n";
    }

/**
 * @brief Bit operations, bulk operations, and byte snapshot roundtrip.
 */
    void bitflags_usage() {
        std::cout << "\n[2] pod::bitflags\n";

        pod::bitflags<16> a{};
        pod::bitflags<16> b{};

        a.set(1);
        a.set(4);
        a.flip(4);

        b.set(4);
        b.set(7);

        auto merged = a | b;

        auto common = a & b;

        auto diff = merged ^ common;

        auto inverted_common = ~common;

        const auto raw = pod::to_bytes(merged);
        const auto restored = pod::from_bytes<16>(raw);

        std::cout << "a.count=" << a.count() << ", b.count=" << b.count() << "\n";
        std::cout << "merged=" << merged << ", common=" << common << "\n";
        std::cout << "diff=" << diff << ", inverted_common=" << inverted_common << "\n";
        std::cout << "restored == merged: " << (restored == merged) << "\n";

        restored.has(7) ? std::cout << "bit7 set\n" : std::cout << "bit7 clear\n";

        pod::bitflags<16> all{};
        all.set_all();
        all.reset_all();
        all.flip_all();
        std::cout << "bulk ops result count=" << all.count() << ", size=" << all.size() << "\n";
    }

/**
 * @brief <code>bytes_view</code> reinterpretation, bounds-safe fetch, clone, and hash.
 */
    void bytes_view_usage() {
        std::cout << "\n[3] pod::bytes_view\n";

        struct Packet {
            std::uint32_t id;
            std::uint16_t len;
        };
        static_assert(pod::trivial_bytes<Packet>);

        const Packet packet{0x12345678U, 42};
        const auto view = pod::bytes_view::from(packet);

        const auto id = view.at<std::uint32_t>(0);
        const auto *len_ptr = view.fetch<std::uint16_t>(sizeof(packet.id));
        const auto packet_copy = view.clone<Packet>();

        std::cout << "view.size=" << view.size() << "\n";
        std::cout << "packet.id=0x" << std::hex << id << std::dec
                  << ", packet.len=" << (len_ptr ? *len_ptr : 0) << "\n";
        std::cout << "clone.id=0x" << std::hex << packet_copy.id << std::dec
                  << ", hash=" << view.hash() << "\n";

        const pod::array<std::uint32_t, 4> words = {10U, 20U, 30U, 40U};
        const auto words_view = pod::bytes_view::from(words.data, words.size());
        if (const auto *third = words_view.fetch<std::uint32_t>(2U * sizeof(std::uint32_t))) {
            std::cout << "third word via fetch=" << *third << "\n";
        }

        const pod::array<pod::pair<int, int>, 2> points = {{{1, 2}, {3, 4}}};
        const auto flattened = pod::bytes_view::from(points).clone<pod::array<int, 4>>();
        std::cout << "flattened clone=" << flattened << "\n";
    }

/**
 * @brief POD optional lifecycle: store/get/ref/value_or/clear/equality.
 */
    void optional_usage() {
        std::cout << "\n[4] pod::optional\n";

        pod::optional<int> maybe{};
        std::cout << "empty=" << maybe.empty() << ", value_or(-1)=" << maybe.value_or(-1) << "\n";

        maybe.store(2024);
        std::cout << "has=" << maybe.has() << ", *get=" << (maybe.get() ? *maybe.get() : -1)
                  << ", ref=" << maybe.ref() << "\n";

        const auto same = pod::make_optional(2024);
        const auto different = pod::make_optional(7);
        std::cout << "maybe==same: " << (maybe == same)
                  << ", maybe==different: " << (maybe == different) << "\n";

        maybe.clear();
        std::cout << "after clear, has=" << maybe.has() << ", printable=" << maybe << "\n";
    }

/**
 * @brief Covers span slicing and <code>to_span()</code> for raw/STL/POD/ADL forms.
 */
    void span_usage() {
        std::cout << "\n[5] pod::span\n";

        int raw[6] = {1, 2, 3, 4, 5, 6};
        const auto raw_view = pod::to_span(raw);

        pod::array<int, 5> pod_arr = {{10, 20, 30, 40, 50}};
        const auto pod_view = pod::to_span(pod_arr);

        std::array<int, 5> std_arr = {11, 22, 33, 44, 55};
        const auto std_view = pod::to_span(std_arr);

        std::vector<int> vec = {100, 200, 300, 400, 500};
        const auto vec_view = pod::to_span(vec);

        method_buffer method_buf{{7, 14, 21, 28, 35}};
        const auto method_view = pod::to_span(method_buf);

        adl_span_demo::buffer adl_buf{{9, 18, 27, 36}};
        const auto adl_view = pod::to_span(adl_buf);

        const pod::span<int> alias_of_raw{raw, 6};
        std::cout << "raw_view == alias_of_raw: " << (raw_view == alias_of_raw) << "\n";

        std::cout << "raw first(3): " << raw_view.first(3) << "\n";
        std::cout << "pod last(2): " << pod_view.last(2) << "\n";
        std::cout << "std sub(1,3): " << std_view.sub(1, 3) << "\n";
        std::cout << "vector sub(2): " << vec_view.sub(2) << "\n";
        std::cout << "method size=" << method_view.size() << ", element[2]=" << method_view[2] << "\n";
        std::cout << "adl span=" << adl_view << "\n";
    }

/**
 * @brief <code>string_view</code> searching, slicing, ordering, hashing, validation.
 */
    void string_view_usage() {
        std::cout << "\n[6] pod::string_view\n";

        using namespace jh::pod::literals;

        constexpr auto text = pod::string_view::from_literal("pod_string");
        constexpr auto digits = "123456"_psv;
        constexpr auto number = "-12.5e3"_psv;
        constexpr auto hex = "0A0B"_psv;
        constexpr auto b64 = "QUJDRA=="_psv;

        const auto tail = text.sub(4);
        const auto cmp = text.compare("pod_stream"_psv);
        const auto pos = text.find('_');
        const auto ord = text <=> "pod_string"_psv;

        char legacy[16]{};
        text.copy_to(legacy, sizeof(legacy));

        std::cout << "text=" << text << ", size=" << text.size() << ", empty=" << text.empty() << "\n";
        std::cout << "tail=" << tail << ", starts_with(pod)=" << text.starts_with("pod"_psv)
                  << ", ends_with(ing)=" << text.ends_with("ing"_psv) << "\n";
        std::cout << "compare(pod_stream)=" << cmp << ", find('_')=" << pos
                  << ", hash=" << text.hash() << "\n";
        std::cout << "digits.is_digit=" << digits.is_digit()
                  << ", number.is_number=" << number.is_number()
                  << ", hex.is_hex=" << hex.is_hex()
                  << ", b64.is_base64=" << b64.is_base64() << "\n";
        std::cout << "ord == equal: " << (ord == std::strong_ordering::equal)
                  << ", to_std=" << text.to_std() << ", copy_to=" << legacy << "\n";
    }

/**
 * @brief Pair,tuple,tools,stringify examples from POD docs.
 */
    void pair_tuple_tools_usage() {
        std::cout << "\n[7] pair + tuple + tools + stringify\n";

        const auto p = pod::make_pair(7, 11);
        const auto [lhs, rhs] = p;

        auto t = pod::make_tuple(42, 3.5f, p);
        const auto first = pod::get<0>(t);
        get<1>(t) = 2.5F;
        const auto pair_in_tuple = get<2>(t);

        const SensorPoint sensor{1001U, 55U};
        const auto [sid, reading] = sensor;

        const pod::tuple<> empty_tuple{};
        const auto single_tuple = pod::make_tuple(123);

        std::cout << "pair=" << p << ", with: {" << lhs << ", " << rhs << "}\n";
        std::cout << "tuple first=" << first << ", pair_in_tuple=" << pair_in_tuple
                  << ", tuple now=" << t << "\n";
        std::cout << "SensorPoint{" << sid << ", " << reading << "}\n";
        std::cout << "empty_tuple=" << empty_tuple << ", single_tuple=" << single_tuple << "\n";

        auto inner = pod::make_pair(1, 2);
        const auto opt_inner = pod::make_optional(inner);
        pod::array<pod::optional<decltype(inner)>, 2> nested{};
        nested[0] = opt_inner;
        std::cout << "nested printable=" << nested << "\n";
    }

/**
 * @brief Matrix reinterpretation via <code>bytes_view::fetch</code> and nested POD arrays.
 */
    void matrix_view_usage() {
        std::cout << "\n[8] matrix view via bytes_view\n";

        constexpr std::size_t kRows = 3;
        constexpr std::size_t kCols = 4;
        using row_type = pod::array<int, kCols>;
        using matrix_type = pod::array<row_type, kRows>;

        pod::array<int, kRows * kCols> flat{};
        std::iota(flat.begin(), flat.end(), 1);

        const auto view = pod::bytes_view::from(flat);

        std::cout << "row-wise fetch:\n";
        for (std::size_t row = 0; row < kRows; ++row) {
            const auto *current = view.fetch<row_type>(row * sizeof(row_type));
            if (current == nullptr) {
                continue;
            }

            for (int v: *current) {
                std::cout << std::setw(2) << v << " ";
            }
            std::cout << "\n";
        }

        if (const auto *matrix = view.fetch<matrix_type>(); matrix != nullptr) {
            std::cout << "full matrix fetch:\n";
            for (const auto &row: *matrix) {
                for (int v: row) {
                    std::cout << std::setw(2) << v << " ";
                }
                std::cout << "\n";
            }
        }
    }

} // namespace example

/**
 * @brief Runs all POD module usage demonstrations.
 */
int main() {
    example::array_usage();
    example::bitflags_usage();
    example::bytes_view_usage();
    example::optional_usage();
    example::span_usage();
    example::string_view_usage();
    example::pair_tuple_tools_usage();
    example::matrix_view_usage();
    return 0;
}
