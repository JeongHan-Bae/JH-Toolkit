#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"
#include "jh/atomic/shared_ptr.h"
#include "jh/atomic/weak_ptr.h"
#include "jh/immutable_str"
#include <atomic>
#include <array>
#include <latch>
#include <mutex>
#include <string_view>
#include <thread>

namespace test {
void atomic_smart_ptr_behavior() {
    using jh::test::tiny_test::expect;
    jh::atomic::shared_ptr<int> slot;
    expect(!slot.load(), "default shared slot is empty");
    auto first = std::make_shared<int>(1);
    slot.store(first);
    expect(slot.load() == first, "shared store/load");
    auto second = std::make_shared<int>(2);
    expect(slot.exchange(second) == first, "shared exchange");
    auto wrong = first;
    expect(!slot.compare_exchange_strong(wrong, first, std::memory_order_acq_rel,
                                         std::memory_order_acquire), "failed shared CAS");
    expect(wrong == second, "CAS updates expected");
    expect(slot.compare_exchange_strong(wrong, first), "successful shared CAS");
    auto owner = std::make_shared<std::pair<int,int>>(1, 2);
    std::shared_ptr<int> alias(owner, &owner->second);
    jh::atomic::shared_ptr<int> aliased(alias);
    expect(aliased.load().get() == &owner->second, "alias pointer retained");

    jh::atomic::weak_ptr<int> observer(first);
    expect(observer.load().lock() == first, "weak snapshot locks");
    expect(observer.exchange(second).lock() == first, "weak exchange");
    observer.store(std::weak_ptr<int>{});
    expect(observer.load().expired(), "empty weak slot");
    auto transient = std::make_shared<int>(5);
    observer.store(transient);
    transient.reset();
    expect(observer.load().expired(), "expired weak slot");

    struct tracked {
        int value;
        std::atomic<int> *destroyed;
        tracked(int v, std::atomic<int> &count) : value(v), destroyed(&count) {}
        ~tracked() { destroyed->fetch_add(1); }
    };
    std::atomic<int> destroyed{0};
    auto owned = std::make_shared<tracked>(7, destroyed);
    jh::atomic::shared_ptr<tracked> concurrent(owned);
    std::latch snapshot_loaded(1), release_snapshot(1);
    bool snapshot_valid = false;
    std::thread reader([&] {
        auto snapshot = concurrent.load();
        snapshot_loaded.count_down();
        release_snapshot.wait();
        snapshot_valid = snapshot && snapshot->value == 7;
    });
    snapshot_loaded.wait();
    concurrent.store(nullptr);
    owned.reset();
    const bool retained_before_release = destroyed.load() == 0;
    release_snapshot.count_down();
    reader.join();
    expect(retained_before_release, "loaded snapshot retains ownership after store");
    expect(snapshot_valid && destroyed.load() == 1, "snapshot releases old object after use");

    jh::atomic::shared_ptr<int> cas_slot(std::make_shared<int>(0));
    std::latch both_loaded(2), start_cas(1);
    std::array<bool, 2> succeeded{};
    std::array<std::shared_ptr<int>, 2> observed{};
    auto contender = [&](std::size_t index) {
        observed[index] = cas_slot.load();
        both_loaded.count_down();
        start_cas.wait();
        succeeded[index] = cas_slot.compare_exchange_strong(
            observed[index], std::make_shared<int>(static_cast<int>(index) + 1),
            std::memory_order_acq_rel, std::memory_order_acquire);
    };
    std::thread left_contender(contender, 0), right_contender(contender, 1);
    both_loaded.wait();
    start_cas.count_down();
    left_contender.join();
    right_contender.join();
    expect(succeeded[0] != succeeded[1], "one of two CAS contenders succeeds");
    expect(observed[succeeded[0] ? 1 : 0] == cas_slot.load(),
           "failed contender observes committed handle");

    auto observed_owner = std::make_shared<int>(7);
    jh::atomic::weak_ptr<int> concurrent_weak(observed_owner);
    std::latch weak_loaded(1), release_weak(1);
    bool weak_expired = false;
    std::thread weak_reader([&] {
        auto snapshot = concurrent_weak.load();
        weak_loaded.count_down();
        release_weak.wait();
        weak_expired = snapshot.expired() && !snapshot.lock();
    });
    weak_loaded.wait();
    concurrent_weak.store(std::weak_ptr<int>{});
    observed_owner.reset();
    release_weak.count_down();
    weak_reader.join();
    expect(weak_expired && concurrent_weak.load().expired(),
           "weak snapshot expires after last owner is removed");

    auto str = jh::make_atomic(" hello ");
    jh::atomic_str_slot str_slot(str);
    expect(str_slot.load() == str, "immutable string atomic slot");
    jh::atomic_weak_str_slot weak_str_slot(str);
    expect(weak_str_slot.load().lock() == str, "immutable string weak slot");
    auto created_slot = jh::make_atomic_slot("created");
    expect(created_slot.load()->view() == "created", "atomic string factory");
    std::mutex source_mutex;
    auto copied_slot = jh::safe_from_slot(std::string_view("copied"), source_mutex);
    expect(copied_slot.load()->view() == "copied", "locked atomic string factory");
}
}
template<>
struct jh::test::tiny_test::test<"atomic smart pointer behavior">
    : jh::test::tiny_test::test_definition<"atomic smart pointer behavior", &::test::atomic_smart_ptr_behavior> {};
template<>
struct jh::test::tiny_test::session<"atomic smart pointer module">
    : jh::test::tiny_test::session_definition<
          "atomic smart pointer module", jh::test::tiny_test::test<"atomic smart pointer behavior">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"atomic smart pointer module"> registration{};
}
