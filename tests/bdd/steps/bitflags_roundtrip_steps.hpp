#pragma once

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <system_error>

#include "jh/pod"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class BitflagsSteps final {
        jh::pod::bitflags<16> original_{};
        jh::pod::bitflags<16> recovered_{};
        jh::pod::array<std::uint8_t, 2> saved_bytes_{};

    public:
        void given_enabled_bits(const std::string_view indices)
        {
            original_.reset_all();
            recovered_.reset_all();
            std::size_t begin = 0;
            while (begin < indices.size()) {
                const std::size_t end = indices.find(',', begin);
                const std::size_t count = (end == std::string_view::npos ? indices.size() : end) - begin;
                unsigned int bit{};
                const auto [parsed_end, error] = std::from_chars(indices.data() + begin,
                                                                  indices.data() + begin + count,
                                                                  bit);
                if (error != std::errc{} || parsed_end != indices.data() + begin + count || bit >= 16) {
                    throw std::invalid_argument("flag indices must be between 0 and 15");
                }
                original_.set(bit);
                if (end == std::string_view::npos) break;
                begin = end + 1;
            }
        }

        void given_no_enabled_bits()
        {
            original_.reset_all();
            recovered_.reset_all();
        }

        void save_as_bytes()
        {
            saved_bytes_ = jh::pod::to_bytes(original_);
        }

        void restore_from_bytes()
        {
            recovered_ = jh::pod::from_bytes<16>(saved_bytes_);
        }

        void enabled_bits_match(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_,
                           "byte roundtrip changed the enabled flags");
        }

        void no_bits_are_enabled(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_ && recovered_.count() == 0,
                           "an empty flag set should remain empty after restoration");
        }
    };

    using BitflagsDefinition = jh::test::cucumber::StepDefinition<
        BitflagsSteps,
        jh::test::cucumber::Given<
            "a 16-bit flag set with enabled bits <string>", &BitflagsSteps::given_enabled_bits>,
        jh::test::cucumber::Given<
            "a 16-bit flag set with no enabled bits", &BitflagsSteps::given_no_enabled_bits>,
        jh::test::cucumber::When<
            "its state is saved to bytes", &BitflagsSteps::save_as_bytes>,
        jh::test::cucumber::When<
            "the state is restored from those bytes", &BitflagsSteps::restore_from_bytes>,
        jh::test::cucumber::Then<
            "the restored flag set contains exactly those bits", &BitflagsSteps::enabled_bits_match>,
        jh::test::cucumber::Then<
            "the restored flag set contains no enabled bits", &BitflagsSteps::no_bits_are_enabled>
    >;
}
