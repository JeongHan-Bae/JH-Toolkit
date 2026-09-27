#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

#include "jh/conceptual/sequence.h"
#include "jh/concepts"
#include "jh/generator"
#include "jh/meta"
#include "jh/pod"
#include "jh/synchronous/ipc/process_launcher.h"
#include "jh/synchronous/ipc/shared_process_mutex.h"

namespace {
    using nested_tuple = std::tuple<int, std::tuple<int, int>>;
    using flat_tuple = decltype(jh::meta::tuple_materialize(std::declval<nested_tuple>()));
    static_assert(std::is_same_v<flat_tuple, std::tuple<int, int, int>>);

    struct pod_contract {
        int x;
        float y;
    };
    static_assert(jh::pod::pod_like<pod_contract>);

    using mutable_span = decltype(jh::pod::to_span(std::declval<jh::pod::array<int, 3>&>()).value());
    using const_span = decltype(jh::pod::to_span(std::declval<const jh::pod::array<int, 3>&>()).value());
    static_assert(std::is_same_v<std::remove_cvref_t<mutable_span>, jh::pod::span<int>>);
    static_assert(std::is_same_v<std::remove_cvref_t<const_span>, jh::pod::span<const int>>);

    using reentrant_mutex = jh::sync::ipc::shared_process_mutex<"m">;
    using recursive_mutex = jh::sync::ipc::shared_process_mutex<"m", true>;
    static_assert(jh::concepts::reentrant_mutex<reentrant_mutex>);
    static_assert(jh::concepts::reentrant_mutex<recursive_mutex>);
    static_assert(jh::concepts::reentrance_capable_mutex<reentrant_mutex>);
    static_assert(jh::concepts::reentrance_capable_mutex<recursive_mutex>);

    static_assert(jh::pod::cv_free_pod_like<jh::sync::ipc::process_exit_result>);
#if IS_POSIX
    constexpr auto signaled_exit = jh::sync::ipc::process_exit_error::signal_segmentation_fault;
    constexpr auto core_dumped_exit = static_cast<jh::sync::ipc::process_exit_error>(
        static_cast<std::uint32_t>(SIGSEGV) | jh::sync::ipc::process_exit_core_dump_mask
    );
    static_assert(jh::sync::ipc::process_exit_is_signal(signaled_exit));
    static_assert(jh::sync::ipc::process_exit_signal_number(signaled_exit) == SIGSEGV);
    static_assert(!jh::sync::ipc::process_exit_has_core_dump(signaled_exit));
    static_assert(jh::sync::ipc::process_exit_is_signal(core_dumped_exit));
    static_assert(jh::sync::ipc::process_exit_signal_number(core_dumped_exit) == SIGSEGV);
    static_assert(jh::sync::ipc::process_exit_has_core_dump(core_dumped_exit));
#endif

    using generator_range = decltype(jh::to_range([]() -> jh::async::generator<int> {
        co_yield 0;
    }));
    static_assert(std::ranges::range<generator_range>);
}
