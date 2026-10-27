/**
 * @file pattern_matcher_controller_steps.hpp
 * @brief Controller registration scenario backed by the Core pattern matcher.
 */

#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "jh/pattern_matcher"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd::controller_sim {
    using JsonObject = std::map<std::string, std::string>;

    struct Request {
        std::string content_type;
        std::variant<std::monostate, JsonObject, std::string> entity;
    };

    struct Response {
        int status = 200;
        std::string content_type = "application/json";
        JsonObject json;
    };

    inline void reject(Response& response, int status, std::string_view reason) {
        response.status = status;
        response.json["error"] = std::string(reason);
    }

    using Captures = jh::runtime_arr<std::string_view>;
    using Handler = void (*)(const Request&, Response&, const Captures&);

    struct NumericRule {
        [[maybe_unused]] static constexpr std::size_t rank = 10;

        [[nodiscard]] std::pair<bool, std::size_t> match(std::string_view input) const {
            const auto end = input.find('/');
            const auto length = end == std::string_view::npos ? input.size() : end;
            if (length == 0) return {false, 0};
            for (std::size_t i = 0; i < length; ++i) {
                if (input[i] < '0' || input[i] > '9') return {false, 0};
            }
            return {true, length};
        }
    };

    struct UuidRule {
        [[maybe_unused]] static constexpr std::size_t rank = 20;

        static bool hex(char value) {
            return (value >= '0' && value <= '9') ||
                   (value >= 'a' && value <= 'f') ||
                   (value >= 'A' && value <= 'F');
        }

        [[nodiscard]] std::pair<bool, std::size_t> match(std::string_view input) const {
            if (input.size() < 36 || (input.size() > 36 && input[36] != '/')) return {false, 0};
            for (std::size_t i = 0; i < 36; ++i) {
                if (i == 8 || i == 13 || i == 18 || i == 23) {
                    if (input[i] != '-') return {false, 0};
                } else if (!hex(input[i])) {
                    return {false, 0};
                }
            }
            return {true, 36};
        }
    };

    struct StringRule {
        [[maybe_unused]] static constexpr std::size_t rank = 30;

        [[nodiscard]] std::pair<bool, std::size_t> match(std::string_view input) const {
            const auto end = input.find('/');
            const auto length = end == std::string_view::npos ? input.size() : end;
            return {length > 0, length};
        }
    };

    using Matcher = jh::pattern_matcher<Handler, 512, NumericRule{}, UuidRule{}, StringRule{}>;
    using Builder = jh::pattern_matcher_builder<Handler, 512, NumericRule{}, UuidRule{}, StringRule{}>;

    template<jh::meta::TStr Path, jh::meta::TStr... Kinds>
    struct Url {
        static consteval bool valid() {
            constexpr auto text = Path.view();
            if (!text.starts_with('/')) return false;
            std::size_t found = 0;
            for (std::size_t i = 0; i < text.size(); ++i) {
                if (text[i] != ':') continue;
                if (i == 0 || text[i - 1] != '/' || i + 1 == text.size() || text[i + 1] == '/') {
                    return false;
                }
                ++found;
            }
            return found == sizeof...(Kinds);
        }

        static_assert(valid(), "URL wildcard count must equal the number of rule kinds");
        static_assert(((Kinds.view() == "numeric" || Kinds.view() == "str" ||
                        Kinds.view() == "uuid") && ...), "Unknown URL wildcard kind");

        static typename Builder::pattern_type pattern(std::string_view prefix) {
            const std::array<std::string_view, sizeof...(Kinds)> kinds{Kinds.view()...};
            std::string full(prefix);
            full += Path.view();
            typename Builder::pattern_type result;
            std::size_t begin = 0;
            std::size_t kind_index = 0;
            for (;;) {
                const auto colon = full.find(':', begin);
                if (colon == std::string::npos) break;
                const auto kind = kinds[kind_index++];
                typename Builder::rule_variant selected;
                if (kind == "numeric") selected = jh::pattern::rule_tag<NumericRule{}>{};
                else if (kind == "uuid") selected = jh::pattern::rule_tag<UuidRule{}>{};
                else selected = jh::pattern::rule_tag<StringRule{}>{};
                result.emplace_back(full.substr(begin, colon - begin), selected);
                const auto slash = full.find('/', colon);
                begin = slash == std::string::npos ? full.size() : slash;
            }
            result.emplace_back(full.substr(begin), jh::pattern::none{});
            return result;
        }
    };

    template<auto Method, jh::meta::TStr Verb, class UrlSpec>
    struct Route {
        static constexpr auto method = Method;
        static constexpr auto verb = Verb;
        using url [[maybe_unused]] = UrlSpec;
    };

    template<class... Entries>
    struct Routes {};

    template<class Controller, jh::meta::TStr Prefix, class RouteList>
    struct Registry;

    template<class Controller, jh::meta::TStr Prefix, class... Entries>
    struct Registry<Controller, Prefix, Routes<Entries...>> {
        static void register_into(Builder& get, Builder& post) {
            static_assert(Prefix.view().starts_with("/api/"));
            (add<Entries>(get, post), ...);
        }

    private:
        template<class Entry>
        static void add(Builder& get, Builder& post) {
            using Method = std::remove_cv_t<decltype(Entry::method)>;
            static_assert(std::same_as<Method, Handler>,
                          "Controller handler must be a static void function with the required arguments");
            static_assert(Entry::verb.view() == "GET" || Entry::verb.view() == "POST",
                          "Only GET and POST are supported in this scenario");

            typename Builder::entry pattern{Entry::url::pattern(Prefix.view()), Entry::method};
            if constexpr (Entry::verb.view() == "GET") get.insert(std::move(pattern));
            else post.insert(std::move(pattern));
        }
    };

    struct AuthController {
        static void login(const Request& request, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            const auto* json = std::get_if<JsonObject>(&request.entity);
            if (request.content_type != "application/json" || !json) {
                reject(response, 415, "unsupported-media-type");
                return;
            }
            if (!json->contains("password")) { reject(response, 400, "missing-password"); return; }
            response.json["user"] = captures[0];
            response.json["authenticated"] = "true";
        }

        static void status(const Request&, Response& response, const Captures& captures) {
            if (!captures.empty()) { reject(response, 400, "capture-count"); return; }
            response.json["status"] = "ready";
        }
    };

    struct CatalogController {
        static void order(const Request&, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            response.json["order_id"] = captures[0];
            response.json["route"] = "numeric";
        }

        static void order_by_name(const Request&, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            response.json["order_id"] = captures[0];
            response.json["route"] = "string";
        }

        static void item(const Request&, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            response.json["item"] = captures[0];
        }

        static void featured_item(const Request&, Response& response, const Captures& captures) {
            if (!captures.empty()) { reject(response, 400, "capture-count"); return; }
            response.json["item"] = "special";
            response.json["featured"] = "true";
        }

        static void note(const Request& request, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            const auto* text = std::get_if<std::string>(&request.entity);
            if (request.content_type != "text/plain" || !text) {
                reject(response, 415, "unsupported-media-type");
                return;
            }
            response.json["note_id"] = captures[0];
            response.json["text"] = *text;
        }
    };

    struct UserController {
        static void user(const Request&, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            response.json["user_id"] = captures[0];
        }

        static void user_details(const Request&, Response& response, const Captures& captures) {
            if (captures.size() != 1) { reject(response, 400, "capture-count"); return; }
            response.json["detail_id"] = captures[0];
        }

        static void profile(const Request&, Response& response, const Captures& captures) {
            if (captures.size() != 2) { reject(response, 400, "capture-count"); return; }
            response.json["name"] = captures[0];
            response.json["number"] = captures[1];
        }
    };

    using AuthRegistration = Registry<AuthController, "/api/auth", Routes<
        Route<&AuthController::login, "POST", Url<"/login/:user", "str">>,
        Route<&AuthController::status, "GET", Url<"/status">>
    >>;
    using CatalogRegistration = Registry<CatalogController, "/api/catalog", Routes<
        Route<&CatalogController::order, "GET", Url<"/orders/:number", "numeric">>,
        Route<&CatalogController::order_by_name, "GET", Url<"/orders/:name", "str">>,
        Route<&CatalogController::item, "GET", Url<"/items/:name", "str">>,
        Route<&CatalogController::featured_item, "GET", Url<"/items/special">>,
        Route<&CatalogController::note, "POST", Url<"/notes/:number", "numeric">>
    >>;
    using UserRegistration = Registry<UserController, "/api/users", Routes<
        Route<&UserController::user, "GET", Url<"/:id", "uuid">>,
        Route<&UserController::user_details, "GET", Url<"/:id/details", "uuid">>,
        Route<&UserController::profile, "GET", Url<"/profile/:name/:number", "str", "numeric">>
    >>;

    class DemoRouter {
        std::optional<Matcher> get_;
        std::optional<Matcher> post_;

        DemoRouter() {
            Builder get;
            Builder post;
            AuthRegistration::register_into(get, post);
            CatalogRegistration::register_into(get, post);
            UserRegistration::register_into(get, post);
            get_.emplace(std::move(get).build());
            post_.emplace(std::move(post).build());
        }

    public:
        static const DemoRouter& instance() {
            static const DemoRouter router;
            return router;
        }

        [[nodiscard]] Response dispatch(std::string_view url, std::string_view verb, const Request& request) const {
            Response response;
            const Matcher* matcher = verb == "GET" ? &*get_ :
                                     verb == "POST" ? &*post_ : nullptr;
            if (!matcher) {
                reject(response, 405, "method-not-allowed");
                return response;
            }
            auto found = matcher->match(url);
            if (!found) {
                reject(response, 404, "not-found");
                return response;
            }
            found.value().second(request, response, found.value().first);
            return response;
        }
    };
}

namespace jh_toolkit_bdd {
    class PatternMatcherControllerSteps final {
        const controller_sim::DemoRouter* router_ = nullptr;
        controller_sim::Request request_;
        controller_sim::Response response_;

    public:
        void given_registered_controller() { router_ = &controller_sim::DemoRouter::instance(); }

        void given_empty_entity() { request_ = {}; }

        void given_json_entity(std::string_view key, std::string_view value) {
            request_.content_type = "application/json";
            request_.entity = controller_sim::JsonObject{{std::string(key), std::string(value)}};
        }

        void given_text_entity(std::string_view value) {
            request_.content_type = "text/plain";
            request_.entity = std::string(value);
        }

        void dispatch_request(std::string_view verb, std::string_view url) {
            response_ = router_->dispatch(url, verb, request_);
        }

        void invoke_login_without_captures() {
            controller_sim::AuthController::login(request_, response_, controller_sim::Captures{});
        }

        void response_status_is(std::int64_t expected, jh::test::cucumber::StepContext& context) const {
            context.expect(response_.status == expected, "controller response status differs");
        }

        void response_media_type_is(std::string_view expected,
                                    jh::test::cucumber::StepContext& context) const {
            context.expect(response_.content_type == expected, "controller response media type differs");
        }

        void response_json_field_is(std::string_view key, std::string_view expected,
                                    jh::test::cucumber::StepContext& context) const {
            const auto it = response_.json.find(std::string(key));
            context.expect(it != response_.json.end() && it->second == expected,
                           "controller JSON response field differs");
        }
    };

    using PatternMatcherControllerDefinition = jh::test::cucumber::StepDefinition<
        PatternMatcherControllerSteps,
        jh::test::cucumber::Given<
            "three stateless controllers registered once", &PatternMatcherControllerSteps::given_registered_controller>,
        jh::test::cucumber::Given<
            "an empty request entity", &PatternMatcherControllerSteps::given_empty_entity>,
        jh::test::cucumber::Given<
            "a JSON request entity with <string> set to <string>", &PatternMatcherControllerSteps::given_json_entity>,
        jh::test::cucumber::Given<
            "a plain text request entity <string>", &PatternMatcherControllerSteps::given_text_entity>,
        jh::test::cucumber::When<
            "a <string> request for <string> is dispatched", &PatternMatcherControllerSteps::dispatch_request>,
        jh::test::cucumber::When<
            "the login handler receives no captures", &PatternMatcherControllerSteps::invoke_login_without_captures>,
        jh::test::cucumber::Then<
            "the response status is <int>", &PatternMatcherControllerSteps::response_status_is>,
        jh::test::cucumber::Then<
            "the response media type is <string>", &PatternMatcherControllerSteps::response_media_type_is>,
        jh::test::cucumber::Then<
            "the JSON response field <string> is <string>", &PatternMatcherControllerSteps::response_json_field_is>
    >;
}
