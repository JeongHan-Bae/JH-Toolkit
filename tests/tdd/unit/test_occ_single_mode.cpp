#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"
#include "jh/concurrent/occ_box.h"
#include <atomic>
#include <latch>
#include <thread>

namespace test {
void occ_single_mode() {
    using jh::test::tiny_test::expect;
    jh::conc::occ_box<int> box(0);
    std::latch both_copied(2), release_writers(1);
    std::atomic<int> callback_calls{0};
    auto increment = [&] {
        box.write([&](int &value) {
            if (callback_calls.fetch_add(1) < 2) {
                both_copied.count_down();
                release_writers.wait();
            }
            ++value;
        });
    };
    std::thread first_writer(increment), second_writer(increment);
    both_copied.wait();
    release_writers.count_down();
    first_writer.join();
    second_writer.join();
    expect(box.read([](const int &v) { return v; }) == 2, "both contended writes commit");
    expect(box.get_version() == 2, "each commit increments the version");
    const auto &source = box;
    jh::conc::occ_box<int> copy(source);
    jh::conc::occ_box<int> moved(std::move(box));
    expect(copy.read([](const int &v) { return v; }) == 2, "copy preserves state");
    expect(moved.read([](const int &v) { return v; }) == 2, "move preserves state");
    expect(box.read([](const int &v) { return v; }) == 2, "move leaves source state");
}
}
template<>
struct jh::test::tiny_test::test<"occ single mode">
    : jh::test::tiny_test::test_definition<"occ single mode", &::test::occ_single_mode> {};
template<>
struct jh::test::tiny_test::session<"occ single mode module">
    : jh::test::tiny_test::session_definition<"occ single mode module", jh::test::tiny_test::test<"occ single mode">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"occ single mode module"> registration{};
}
