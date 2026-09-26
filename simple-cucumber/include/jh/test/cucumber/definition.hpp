/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file definition.hpp
 * @brief Compile-time Cucumber step definitions and dispatch.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "jh/metax/expected.h"
#include "jh/metax/t_str.h"
#include "jh/test/cucumber/expression.hpp"
#include "jh/test/cucumber/runtime.hpp"
#include "jh/test/cucumber/scalar.hpp"

namespace jh::test::cucumber {
    namespace detail {
        template<typename T>
        struct member_method_traits {
            static constexpr bool valid = false;
            static constexpr bool is_static = false;
            using class_type = void;
            using return_type = void;
            using arguments = std::tuple<>;
        };

        template<typename C, typename... Args>
        struct member_method_traits<void (C::*)(Args...)> {
            static constexpr bool valid = true;
            static constexpr bool is_static = false;
            using class_type = C;
            using return_type = void;
            using arguments = std::tuple<Args...>;
        };

        template<typename C, typename... Args>
        struct member_method_traits<void (C::*)(Args...) noexcept>
            : member_method_traits<void (C::*)(Args...)> {};

        template<typename C, typename... Args>
        struct member_method_traits<void (C::*)(Args...) const> {
            static constexpr bool valid = true;
            static constexpr bool is_static = false;
            using class_type = C;
            using return_type = void;
            using arguments = std::tuple<Args...>;
        };

        template<typename C, typename... Args>
        struct member_method_traits<void (C::*)(Args...) const noexcept>
            : member_method_traits<void (C::*)(Args...) const> {};

        template<typename T>
        struct member_method_traits<const T> : member_method_traits<T> {};

        template<typename... Args>
        struct member_method_traits<void (*)(Args...)> {
            static constexpr bool valid = true;
            static constexpr bool is_static = true;
            using class_type = void;
            using return_type = void;
            using arguments = std::tuple<Args...>;
        };

        template<typename... Args>
        struct member_method_traits<void (*)(Args...) noexcept>
            : member_method_traits<void (*)(Args...)> {};

        template<auto Method, class Object, class... Args>
        void invoke_callable([[maybe_unused]] Object& object, Args&&... args) {
            using traits = member_method_traits<std::remove_cv_t<decltype(Method)>>;
            if constexpr (traits::is_static) {
                std::invoke(Method, std::forward<Args>(args)...);
            } else {
                std::invoke(Method, object, std::forward<Args>(args)...);
            }
        }

        template<typename Tuple>
        struct signature_layout {
            static constexpr std::size_t argument_count = std::tuple_size_v<Tuple>;
            static constexpr bool has_context = [] {
                if constexpr (argument_count == 0) return false;
                else return std::is_same_v<
                    std::tuple_element_t<argument_count - 1, Tuple>,
                    StepContext&
                >;
            }();
            static constexpr std::size_t without_context =
                argument_count - (has_context ? 1u : 0u);
            static constexpr bool has_table = [] {
                if constexpr (without_context == 0) return false;
                else return std::is_same_v<
                    std::tuple_element_t<without_context - 1, Tuple>,
                    const DataTable&
                >;
            }();
            static constexpr std::size_t scalar_count =
                without_context - (has_table ? 1u : 0u);
        };

        template<typename Tuple, typename Sequence>
        struct tuple_prefix;

        template<typename Tuple, std::size_t... I>
        struct tuple_prefix<Tuple, std::index_sequence<I...>> {
            using type = std::tuple<std::tuple_element_t<I, Tuple>...>;
        };

        template<typename Tuple, std::size_t Count>
        using tuple_prefix_t = typename tuple_prefix<
            Tuple,
            std::make_index_sequence<Count>
        >::type;

        template<typename Tuple, std::size_t... I>
        bool captured_arguments_are_valid(
            const auto& captures,
            std::index_sequence<I...>
        ) noexcept {
            return (
                scalar_value_is_valid<std::tuple_element_t<I, Tuple>>(captures[I]) && ...
            );
        }

        template<ParameterType Type>
        struct parameter_cpp_type;

        template<>
        struct parameter_cpp_type<ParameterType::string> { using type = std::string_view; };
        template<>
        struct parameter_cpp_type<ParameterType::integer> { using type = std::int64_t; };
        template<>
        struct parameter_cpp_type<ParameterType::unsigned_integer> { using type = std::uint64_t; };
        template<>
        struct parameter_cpp_type<ParameterType::real> { using type = double; };
        template<>
        struct parameter_cpp_type<ParameterType::boolean> { using type = bool; };

        template<jh::meta::TStr Expression, std::size_t Index>
        using expression_argument_t = typename parameter_cpp_type<
            expression_spec<Expression>.parameters[Index]
        >::type;

        template<jh::meta::TStr Expression, typename Tuple, std::size_t... I>
        consteval bool scalar_parameters_match(std::index_sequence<I...>) {
            return (
                std::is_same_v<
                    std::tuple_element_t<I, Tuple>,
                    expression_argument_t<Expression, I>
                > && ...
            );
        }

        template<class Object, class Binding>
        consteval bool binding_signature_valid() {
            constexpr auto spec = expression_spec<Binding::expression>;
            using traits = member_method_traits<decltype(Binding::method)>;
            if constexpr (!spec.valid || !traits::valid) {
                return false;
            } else {
                using args = typename traits::arguments;
                using layout = signature_layout<args>;
                constexpr auto scalar_count = layout::scalar_count;
                constexpr bool owner_matches = [] {
                    if constexpr (traits::is_static) return true;
                    else return std::is_base_of_v<typename traits::class_type, Object>;
                }();
                if constexpr (
                    !std::is_same_v<typename traits::return_type, void> ||
                    !owner_matches ||
                    scalar_count != spec.parameter_count
                ) {
                    return false;
                } else {
                    return scalar_parameters_match<Binding::expression, args>(
                        std::make_index_sequence<scalar_count>{}
                    );
                }
            }
        }

        template<class... Bindings>
        struct unique_step_bindings : std::true_type {};

        template<class First, class... Rest>
        struct unique_step_bindings<First, Rest...> : std::bool_constant<
            (((First::kind != Rest::kind) ||
              (First::expression.view() != Rest::expression.view())) && ...) &&
            unique_step_bindings<Rest...>::value
        > {};

        template<class... Bindings>
        inline constexpr bool unique_step_bindings_v =
            unique_step_bindings<Bindings...>::value;

        template<typename Tuple, std::size_t I = 0>
        bool convert_arguments(
            const auto& captures,
            Tuple& values,
            StepContext& context
        ) {
            if constexpr (I == std::tuple_size_v<Tuple>) {
                return true;
            } else {
                using value_type = std::tuple_element_t<I, Tuple>;
                if constexpr (std::is_same_v<value_type, std::string_view>) {
                    std::get<I>(values) = captures[I];
                } else {
                    auto converted = as<value_type>(captures[I]);
                    if (!converted) {
                        std::string message = "argument ";
                        message += std::to_string(I + 1);
                        message += " could not be converted to ";
                        message += scalar_type_name<value_type>();
                        message += ".";
                        context.fail(message);
                        return false;
                    }

                    std::get<I>(values) = std::move(*converted);
                }

                return convert_arguments<Tuple, I + 1>(captures, values, context);
            }
        }

        template<class Binding, class Object, class Tuple, std::size_t... I>
        void invoke_method(
            Object& object,
            const Step& step,
            StepContext& context,
            Tuple& values,
            std::index_sequence<I...>
        ) {
            using traits = member_method_traits<decltype(Binding::method)>;
            using args = typename traits::arguments;
            using layout = signature_layout<args>;

            if constexpr (layout::has_table && layout::has_context) {
                invoke_callable<Binding::method>(
                    object, std::move(std::get<I>(values))..., *step.table, context);
            } else if constexpr (layout::has_table) {
                invoke_callable<Binding::method>(
                    object, std::move(std::get<I>(values))..., *step.table);
            } else if constexpr (layout::has_context) {
                invoke_callable<Binding::method>(
                    object, std::move(std::get<I>(values))..., context);
            } else {
                invoke_callable<Binding::method>(
                    object, std::move(std::get<I>(values))...);
            }
        }

        template<class Object, class Binding>
        void invoke_binding(Object& object, const Step& step, StepContext& context) {
            const auto captures = match_expression<Binding::expression>(step.text);
            if (!captures) {
                context.fail("step text no longer matches the selected definition.");
                return;
            }

            using traits = member_method_traits<decltype(Binding::method)>;
            using args = typename traits::arguments;
            using layout = signature_layout<args>;
            constexpr auto scalar_count = layout::scalar_count;
            using scalar_arguments = tuple_prefix_t<args, layout::scalar_count>;

            if constexpr (layout::has_table) {
                if (!step.table) {
                    context.fail("step definition requires a DataTable attachment.");
                    return;
                }
            } else if (step.table) {
                context.fail("step has an unexpected DataTable attachment.");
                return;
            }

            scalar_arguments values{};
            if (!convert_arguments(*captures, values, context)) return;

            invoke_method<Binding>(
                object,
                step,
                context,
                values,
                std::make_index_sequence<scalar_count>{}
            );
        }
    }

    /**
     * @brief Binds a Gherkin Given step expression to a step callable.
     * @tparam Expression Compile-time step expression.
     * @tparam Method Pointer to a void member or static function that handles the step.
     */
    template<jh::meta::TStr Expression, auto Method>
    struct Given {
        /// @brief Compile-time expression matched against Given step text.
        static constexpr auto expression = Expression;
        /// @brief Callable invoked when the expression matches.
        static constexpr auto method = Method;
        /// @brief Step role associated with this binding.
        static constexpr StepKind kind = StepKind::given;
        static_assert(detail::expression_spec<Expression>.valid,
                      "Invalid Cucumber step expression.");
    };

    /**
     * @brief Binds a Gherkin When step expression to a step callable.
     * @tparam Expression Compile-time step expression.
     * @tparam Method Pointer to a void member or static function that handles the step.
     */
    template<jh::meta::TStr Expression, auto Method>
    struct When {
        /// @brief Compile-time expression matched against When step text.
        static constexpr auto expression = Expression;
        /// @brief Callable invoked when the expression matches.
        static constexpr auto method = Method;
        /// @brief Step role associated with this binding.
        static constexpr StepKind kind = StepKind::when;
        static_assert(detail::expression_spec<Expression>.valid,
                      "Invalid Cucumber step expression.");
    };

    /**
     * @brief Binds a Gherkin Then step expression to a step callable.
     * @tparam Expression Compile-time step expression.
     * @tparam Method Pointer to a void member or static function that handles the step.
     */
    template<jh::meta::TStr Expression, auto Method>
    struct Then {
        /// @brief Compile-time expression matched against Then step text.
        static constexpr auto expression = Expression;
        /// @brief Callable invoked when the expression matches.
        static constexpr auto method = Method;
        /// @brief Step role associated with this binding.
        static constexpr StepKind kind = StepKind::then;
        static_assert(detail::expression_spec<Expression>.valid,
                      "Invalid Cucumber step expression.");
    };

    /// @brief Describes the outcome of dispatching a parsed step.
    enum class dispatch_status {
        /// @brief A matching callable was invoked.
        invoked,
        /// @brief No binding matched the step role and text.
        undefined_step,
        /// @brief A matching expression rejected its captured values or table attachment.
        invalid_arguments
    };

    /// @brief Reports the status returned by a step-definition dispatch.
    struct dispatch_result {
        /// @brief Dispatch outcome.
        dispatch_status status{dispatch_status::undefined_step};
    };

    /// @brief Describes why a step-definition dispatch could not select one binding.
    enum class dispatch_error {
        /// @brief Multiple bindings accept the same step and its arguments.
        ambiguous_step
    };

    /**
     * @brief Defines compile-time step bindings and dispatches matching steps to their callables.
     * @tparam Object Type instantiated to handle each scenario.
     * @tparam Steps One or more Given, When, or Then bindings.
     * Each callable must return void and accept the placeholder values in its expression.
     * It may also accept a trailing <code>const DataTable&amp;</code> followed by a trailing <code>StepContext&amp;</code>.
     * Duplicate expressions for the same step role and incompatible callable signatures are rejected at compile time.
     */
    template<class Object, class... Steps>
    struct StepDefinition final {
        static_assert(sizeof...(Steps) > 0, "StepDefinition requires at least one step.");
        static_assert(detail::unique_step_bindings_v<Steps...>,
                      "StepDefinition cannot bind the same expression more than once for the same step kind.");
        static_assert((detail::binding_signature_valid<Object, Steps>() && ...),
                      "Step callable must be a void member or static function whose arguments match its expression and optional attachments.");

        /// @brief Step object type constructed for each scenario.
        using object_type [[maybe_unused]] = Object;

    private:
        struct registration {
            StepKind kind{StepKind::unknown};
            std::string_view expression{};
            bool (*matches)(const Step&) = nullptr;
            [[maybe_unused]] bool (*arguments_are_valid)(const Step&) = nullptr;
            void (*invoke)(Object&, const Step&, StepContext&) = nullptr;
        };

        template<class Binding>
        static bool matches(const Step& step) {
            return step.kind == Binding::kind &&
                   match_expression<Binding::expression>(step.text).has_value();
        }

        template<class Binding>
        static bool arguments_are_valid(const Step& step) {
            const auto captures = match_expression<Binding::expression>(step.text);
            if (!captures) return false;

            using traits = detail::member_method_traits<decltype(Binding::method)>;
            using args = typename traits::arguments;
            using layout = detail::signature_layout<args>;
            using scalar_arguments = detail::tuple_prefix_t<args, layout::scalar_count>;
            constexpr auto spec = detail::expression_spec<Binding::expression>;
            return layout::has_table == step.table.has_value() &&
                   detail::captured_arguments_are_valid<scalar_arguments>(
                       *captures,
                       std::make_index_sequence<spec.parameter_count>{}
                   );
        }

        template<class Binding>
        static void invoke(Object& object, const Step& step, StepContext& context) {
            detail::invoke_binding<Object, Binding>(object, step, context);
        }

        [[nodiscard]] static const std::array<registration, sizeof...(Steps)>& entries() {
            static constexpr std::array<registration, sizeof...(Steps)> all{{
                registration{
                    Steps::kind,
                    Steps::expression.view(),
                    &matches<Steps>,
                    &arguments_are_valid<Steps>,
                    &invoke<Steps>
                }...
            }};
            return all;
        }

    public:
        /**
         * @brief Selects and invokes the binding that accepts a parsed step.
         * @param object Step object used for a non-static callable.
         * @param step Parsed step to match and dispatch.
         * @param context Receives assertion and conversion failures from the callable.
         * @return The dispatch status, or <code>dispatch_error::ambiguous_step</code> when multiple bindings accept the step.
         */
        [[nodiscard]] static jh::meta::expected<dispatch_result, dispatch_error> dispatch(
            Object& object,
            const Step& step,
            StepContext& context
        ) {
            const registration* selected = nullptr;
            const registration* sole_expression_match = nullptr;
            std::size_t expression_match_count = 0;
            std::size_t matching_count = 0;

            for (const auto& entry : entries()) {
                if (!entry.matches(step)) continue;
                ++expression_match_count;
                sole_expression_match = &entry;
                if (!entry.arguments_are_valid(step)) continue;
                ++matching_count;
                selected = &entry;
            }

            if (matching_count > 1) {
                return jh::meta::unexpected(dispatch_error::ambiguous_step);
            }
            if (matching_count == 0) {
                if (expression_match_count == 0) {
                    return dispatch_result{dispatch_status::undefined_step};
                }
                if (expression_match_count > 1) {
                    return dispatch_result{dispatch_status::invalid_arguments};
                }
                selected = sole_expression_match;
            }

            selected->invoke(object, step, context);
            return dispatch_result{dispatch_status::invoked};
        }
    };
}
