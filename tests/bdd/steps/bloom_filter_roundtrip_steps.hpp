/**
 * @file bloom_filter_roundtrip_steps.hpp
 * @brief BDD steps for Bloom filter binary roundtrips.
 */

#pragma once

#include "jh/bloom_filter"
#include "jh/pod"
#include "jh/test/cucumber/cucumber.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

namespace jh_toolkit_bdd {
    struct BloomFirstHash {
        auto operator()(const std::string_view& value) const {
            const auto first = value.empty() ? std::size_t{0} :
                static_cast<std::size_t>(static_cast<unsigned char>(value.front()));
            return std::tuple{value.size(), first};
        }
    };

    struct BloomSecondHash {
        auto operator()(const std::string_view& value) const {
            const auto first = value.empty() ? std::size_t{0} :
                static_cast<std::size_t>(static_cast<unsigned char>(value.front()));
            const auto last = value.empty() ? std::size_t{0} :
                static_cast<std::size_t>(static_cast<unsigned char>(value.back()));
            return jh::pod::make_tuple(last + 7, value.size() * 13 + first);
        }
    };

    using InlineBloom = jh::bloom_filter<std::string_view, 65, jh::typed::monostate,
                                         BloomFirstHash{}, BloomSecondHash{}>;
    using AllocatedBloom = jh::bloom_filter<std::string_view, 65, std::allocator<std::uint64_t>,
                                            BloomFirstHash{}, BloomSecondHash{}>;
    using BloomRecord = jh::bloom_persistence<"view-sentinel", InlineBloom>;

    class BloomFilterRoundtripSteps final {
        std::unique_ptr<InlineBloom> inline_source_;
        std::unique_ptr<AllocatedBloom> allocated_source_;
        std::unique_ptr<InlineBloom> inline_target_;
        std::unique_ptr<AllocatedBloom> allocated_target_;
        std::string record_;
        std::string pear_ = "pear";
        std::string plum_ = "plum";
        std::string kiwi_ = "kiwi";
        std::string marker_ = "marker";
        std::string boundary_key_ = std::string(64, 'x');
        bool rejected_ = false;

        template<typename Fn>
        void with_source(Fn&& action) {
            if (inline_source_) action(*inline_source_);
            else if (allocated_source_) action(*allocated_source_);
            else throw std::logic_error("Bloom source is absent");
        }

        template<typename Fn>
        void with_target(Fn&& action) const {
            if (inline_target_) action(*inline_target_);
            else if (allocated_target_) action(*allocated_target_);
            else throw std::logic_error("Bloom destination is absent");
        }

    public:
        void given_source(const std::string_view storage) {
            if (storage == "inline") inline_source_ = std::make_unique<InlineBloom>();
            else if (storage == "allocated") allocated_source_ = std::make_unique<AllocatedBloom>();
            else throw std::invalid_argument("unknown Bloom source storage");
        }

        void given_destination(const std::string_view storage) {
            if (storage == "inline") {
                inline_target_ = std::make_unique<InlineBloom>();
                inline_target_->insert(std::string_view{marker_});
            } else if (storage == "allocated") {
                allocated_target_ = std::make_unique<AllocatedBloom>();
                allocated_target_->insert(std::string_view{marker_});
            } else {
                throw std::invalid_argument("unknown Bloom destination storage");
            }
        }

        void insert_values() {
            with_source([&](auto& filter) {
                filter.insert(std::string_view{pear_});
                filter.insert(std::string_view{plum_});
            });
        }

        void insert_boundary_values() {
            with_source([&](auto& filter) {
                filter.insert(std::string_view{});
                filter.insert(std::string_view{boundary_key_});
            });
        }

        void save_record() {
            std::ostringstream out(std::ios::binary);
            with_source([&](const auto& filter) {
                jh::serialize_bloom<"view-sentinel">(out, filter);
            });
            record_ = out.str();
        }

        void restore_record() {
            std::istringstream in(record_, std::ios::binary);
            with_target([&](auto& filter) {
                jh::deserialize_bloom<"view-sentinel">(in, filter);
            });
        }

        void change_signature() {
            if (record_.size() <= 8) throw std::logic_error("Bloom record is absent");
            record_[8] ^= 1;
        }

        void attempt_restore() {
            rejected_ = false;
            try {
                restore_record();
            } catch (const std::runtime_error&) {
                rejected_ = true;
            }
        }

        void inserted_values_possible(jh::test::cucumber::StepContext& context) const {
            with_target([&](const auto& filter) {
                context.expect(filter.possibly_contains(std::string_view{pear_}) &&
                               filter.possibly_contains(std::string_view{plum_}),
                               "restored Bloom filter lost an inserted value");
            });
        }

        void absent_values_rejected(jh::test::cucumber::StepContext& context) const {
            with_target([&](const auto& filter) {
                context.expect(!filter.possibly_contains(std::string_view{kiwi_}) &&
                               !filter.possibly_contains(std::string_view{marker_}),
                               "restored Bloom filter retained an absent marker");
            });
        }

        void boundary_values_possible(jh::test::cucumber::StepContext& context) const {
            with_target([&](const auto& filter) {
                context.expect(filter.possibly_contains(std::string_view{}) &&
                               filter.possibly_contains(std::string_view{boundary_key_}),
                               "restored Bloom filter lost an empty or word-boundary key");
            });
        }

        void marker_absent(jh::test::cucumber::StepContext& context) const {
            with_target([&](const auto& filter) {
                context.expect(!filter.possibly_contains(std::string_view{marker_}),
                               "restored Bloom filter retained the destination marker");
            });
        }

        void record_length_matches(jh::test::cucumber::StepContext& context) const {
            context.expect(record_.size() == BloomRecord::serialized_size,
                           "Bloom record length differs from the compile-time layout");
        }

        void damaged_record_rejected(jh::test::cucumber::StepContext& context) const {
            context.expect(rejected_, "damaged Bloom record was accepted");
        }

        void marker_preserved(jh::test::cucumber::StepContext& context) const {
            with_target([&](const auto& filter) {
                context.expect(filter.possibly_contains(std::string_view{marker_}),
                               "failed restoration cleared the destination marker");
            });
        }

        void source_value_absent(jh::test::cucumber::StepContext& context) const {
            with_target([&](const auto& filter) {
                context.expect(!filter.possibly_contains(std::string_view{pear_}),
                               "failed restoration changed destination membership");
            });
        }
    };

    using BloomFilterRoundtripDefinition = jh::test::cucumber::StepDefinition<
        BloomFilterRoundtripSteps,
        jh::test::cucumber::Given<
            "an <string> Bloom filter for string-view keys with 65 bits", &BloomFilterRoundtripSteps::given_source>,
        jh::test::cucumber::Given<
            "an <string> destination containing the marker key", &BloomFilterRoundtripSteps::given_destination>,
        jh::test::cucumber::When<
            "the pear and plum keys are inserted", &BloomFilterRoundtripSteps::insert_values>,
        jh::test::cucumber::When<
            "the empty and 64-byte keys are inserted", &BloomFilterRoundtripSteps::insert_boundary_values>,
        jh::test::cucumber::When<
            "the filter is saved as a binary record", &BloomFilterRoundtripSteps::save_record>,
        jh::test::cucumber::When<
            "the binary record is restored into the destination", &BloomFilterRoundtripSteps::restore_record>,
        jh::test::cucumber::When<
            "the signature byte is changed", &BloomFilterRoundtripSteps::change_signature>,
        jh::test::cucumber::When<
            "restoration is attempted", &BloomFilterRoundtripSteps::attempt_restore>,
        jh::test::cucumber::Then<
            "the pear and plum keys are possibly present", &BloomFilterRoundtripSteps::inserted_values_possible>,
        jh::test::cucumber::Then<
            "the kiwi and marker keys are definitely absent", &BloomFilterRoundtripSteps::absent_values_rejected>,
        jh::test::cucumber::Then<
            "the empty and 64-byte keys are possibly present", &BloomFilterRoundtripSteps::boundary_values_possible>,
        jh::test::cucumber::Then<
            "the marker key is definitely absent", &BloomFilterRoundtripSteps::marker_absent>,
        jh::test::cucumber::Then<
            "the binary record has its compile-time length", &BloomFilterRoundtripSteps::record_length_matches>,
        jh::test::cucumber::Then<
            "the damaged record is rejected", &BloomFilterRoundtripSteps::damaged_record_rejected>,
        jh::test::cucumber::Then<
            "the marker key remains possibly present", &BloomFilterRoundtripSteps::marker_preserved>,
        jh::test::cucumber::Then<
            "the pear key remains definitely absent", &BloomFilterRoundtripSteps::source_value_absent>
    >;
}
