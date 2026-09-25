#pragma once

#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "jh/runtime_arr"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class RuntimeArrBoolSteps final {
        std::vector<bool> original_;
        std::vector<bool> recovered_;
        std::optional<jh::runtime_arr<bool>> runtime_bits_;

    public:
        void given_values(const std::string_view values)
        {
            original_.clear();
            recovered_.clear();
            runtime_bits_.reset();
            for (const char value : values) {
                if (value == '0' || value == '1') original_.push_back(value == '1');
                else if (value != ',' && value != ' ') {
                    throw std::invalid_argument("boolean values must contain only 0 and 1");
                }
            }
        }

        void given_empty_values()
        {
            original_.clear();
            recovered_.clear();
            runtime_bits_.reset();
        }

        void given_alternating_values_across_word_boundary()
        {
            original_.clear();
            recovered_.clear();
            original_.reserve(65);
            for (std::size_t i = 0; i < 65; ++i) original_.push_back(i % 2 == 0);
        }

        void move_into_runtime_array()
        {
            auto values = original_;
            runtime_bits_.emplace(std::move(values));
        }

        void move_back_to_vector()
        {
            if (!runtime_bits_) throw std::logic_error("boolean sequence was not moved into runtime storage");
            recovered_ = static_cast<std::vector<bool>>(std::move(*runtime_bits_));
        }

        void values_match(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_, "runtime array conversion changed the boolean values");
        }
    };

    using RuntimeArrBoolDefinition = jh::test::cucumber::StepDefinition<
        RuntimeArrBoolSteps,
        jh::test::cucumber::Given<
            "boolean values <string>", &RuntimeArrBoolSteps::given_values>,
        jh::test::cucumber::Given<
            "an empty boolean vector", &RuntimeArrBoolSteps::given_empty_values>,
        jh::test::cucumber::Given<
            "65 alternating boolean values", &RuntimeArrBoolSteps::given_alternating_values_across_word_boundary>,
        jh::test::cucumber::When<
            "they are moved into a runtime-sized bit sequence", &RuntimeArrBoolSteps::move_into_runtime_array>,
        jh::test::cucumber::When<
            "the sequence is moved back to a boolean vector", &RuntimeArrBoolSteps::move_back_to_vector>,
        jh::test::cucumber::Then<
            "the original boolean values are restored", &RuntimeArrBoolSteps::values_match>
    >;
}
