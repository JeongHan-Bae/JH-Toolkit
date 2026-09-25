#pragma once

#include <charconv>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "jh/runtime_arr"
#include "jh/test/cucumber/cucumber.hpp"
#include "jh/views"

namespace jh_toolkit_bdd {
    class MutableEnumerationSteps final {
        std::vector<int> values_;
        std::optional<jh::runtime_arr<int>> runtime_values_;
        std::size_t expected_size_{};

        static std::vector<int> parse_values(std::string_view values)
        {
            std::vector<int> result;
            std::size_t begin = 0;
            while (begin < values.size()) {
                const std::size_t end = values.find(',', begin);
                const std::size_t finish = end == std::string_view::npos ? values.size() : end;
                int value{};
                const auto [parsed_end, error] = std::from_chars(
                    values.data() + begin, values.data() + finish, value);
                if (error != std::errc{} || parsed_end != values.data() + finish) {
                    throw std::invalid_argument("sequence values must be comma-separated integers");
                }
                result.push_back(value);
                if (end == std::string_view::npos) break;
                begin = end + 1;
            }
            return result;
        }

    public:
        void given_sequence(std::string_view values)
        {
            values_ = parse_values(values);
            expected_size_ = values_.size();
            runtime_values_.reset();
        }

        void given_runtime_sequence(std::string_view values)
        {
            const auto parsed = parse_values(values);
            values_.clear();
            expected_size_ = parsed.size();
            runtime_values_.emplace(parsed.size());
            for (std::size_t i = 0; i < parsed.size(); ++i) runtime_values_->set(i, parsed[i]);
        }

        void update_values()
        {
            if (runtime_values_) {
                for (auto [position, value] : jh::views::enumerate(*runtime_values_, 100)) {
                    value = static_cast<int>(position * 10);
                }
            } else {
                for (auto [position, value] : jh::views::enumerate(values_, 100)) {
                    value = static_cast<int>(position * 10);
                }
            }
        }

        void values_are_visible_on_later_traversal(jh::test::cucumber::StepContext& context) const
        {
            bool matches = true;
            if (runtime_values_) {
                matches = runtime_values_->size() == expected_size_;
                for (auto [position, value] : jh::views::enumerate(*runtime_values_, 100)) {
                    matches = matches && value == static_cast<int>(position * 10);
                }
            } else {
                matches = values_.size() == expected_size_;
                for (auto [position, value] : jh::views::enumerate(values_, 100)) {
                    matches = matches && value == static_cast<int>(position * 10);
                }
            }
            context.expect(matches, "later traversal did not observe the updates");
        }
    };

    using MutableEnumerationDefinition = jh::test::cucumber::StepDefinition<
        MutableEnumerationSteps,
        jh::test::cucumber::Given<
            "a mutable integer sequence containing <string>", &MutableEnumerationSteps::given_sequence>,
        jh::test::cucumber::Given<
            "a runtime-sized integer sequence containing <string>", &MutableEnumerationSteps::given_runtime_sequence>,
        jh::test::cucumber::When<
            "each value is set to ten times its displayed position starting at 100",
            &MutableEnumerationSteps::update_values>,
        jh::test::cucumber::Then<
            "a later traversal shows the updated values at those positions",
            &MutableEnumerationSteps::values_are_visible_on_later_traversal>
    >;
}
