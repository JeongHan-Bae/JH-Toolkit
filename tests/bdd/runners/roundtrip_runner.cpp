#include "base64_huffman_roundtrip_steps.hpp"
#include "base64_roundtrips_steps.hpp"
#include "bitflags_roundtrip_steps.hpp"
#include "huffman_roundtrip_steps.hpp"
#include "mutable_enumeration_steps.hpp"
#include "pod_payload_roundtrip_steps.hpp"
#include "runtime_arr_bool_roundtrip_steps.hpp"
#include "uri_binary_roundtrip_steps.hpp"
#include "uri_roundtrips_steps.hpp"
#include "uri_safe_text_steps.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace {
    template <typename Definition>
    bool run_feature(const std::string_view feature_name)
    {
        const auto result = jh::test::cucumber::run_file<Definition>(
            std::filesystem::path{JH_TOOLKIT_BDD_FEATURE_DIR} / feature_name);
        if (!result) {
            std::cerr << "BDD feature has ambiguous step definitions: " << feature_name << '\n';
            return false;
        }

        std::cout << jh::test::cucumber::format_report(*result);
        return result->passed();
    }
}

int main()
{
    bool passed = true;
    passed = run_feature<jh_toolkit_bdd::Base64Definition>("base64_roundtrips.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::UriDefinition>("uri_roundtrips.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::HuffmanDefinition>("huffman_roundtrip.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::Base64HuffmanDefinition>("base64_huffman_roundtrip.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::PodPayloadDefinition>("pod_payload_roundtrip.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::RuntimeArrBoolDefinition>("runtime_arr_bool_roundtrip.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::BitflagsDefinition>("bitflags_roundtrip.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::MutableEnumerationDefinition>("mutable_enumeration.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::UriBinaryRoundtripDefinition>("uri_binary_roundtrip.feature") && passed;
    passed = run_feature<jh_toolkit_bdd::UriSafeTextDefinition>("uri_safe_text.feature") && passed;
    return passed ? 0 : 1;
}
