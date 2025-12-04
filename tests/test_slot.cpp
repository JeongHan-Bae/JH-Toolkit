#include <catch2/catch_all.hpp>

#include "jh/async"
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <variant>
#include <sstream>
#include <tuple>

TEST_CASE("Basic Test") {
    using namespace std::chrono_literals;
    using jh::async::slot;
    using jh::async::slot_hub;
    using jh::async::listener;
    using jh::async::event_signal;

    std::ostringstream out{};
    std::mutex out_mtx;
    auto safe_out = [&](const std::string &s) {
        std::lock_guard guard(out_mtx);
        out << s;
    };

    slot_hub hub(1000ms);
    auto make_slot = [&](listener<int> &aw_int) -> slot {
        safe_out("[slot coro] started\n");

        for (;;) {
            auto v1 = co_await aw_int;
            safe_out("[slot coro] int = " + std::to_string(v1) + "\n");
            co_yield {};
        }
    };

    auto aw_int = hub.make_listener<int>();

    event_signal<int> sig_int;

    sig_int.connect(&aw_int);
    slot s = make_slot(aw_int);

    hub.bind_slot(s);

    s.spawn(); // Manually start the coroutine

    std::thread emit_int([&]{
        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(20ms);
            safe_out("[emit-int] emit(" + std::to_string(i) + ")\n");
            sig_int.emit(i);
        }
    });

    emit_int.join();
    auto log = out.str();

    REQUIRE(log.find("[slot coro] started") != std::string::npos);

    for (int i = 0; i < 5; ++i) {
        std::string emit_tag = "[emit-int] emit(" + std::to_string(i) + ")\n";
        std::string slot_tag = "[slot coro] int = " + std::to_string(i) + "\n";

        auto pos_emit = log.find(emit_tag);
        REQUIRE(pos_emit != std::string::npos);

        auto pos_slot = log.find(slot_tag, pos_emit);
        REQUIRE(pos_slot != std::string::npos);
    }

}

TEST_CASE("Conditional Start Test") {
    using namespace std::chrono_literals;
    using jh::async::slot;
    using jh::async::slot_hub;
    using jh::async::listener;
    using jh::async::event_signal;

    std::ostringstream out{};
    std::mutex out_mtx;
    auto safe_out = [&](const std::string &s) {
        std::lock_guard guard(out_mtx);
        out << s;
    };

    slot_hub hub(1000ms);

    auto make_slot = [&](listener<int> &aw_int) -> slot {
        safe_out("[slot coro] started\n");

        // Wait until first >=5
        for (;;) {
            int v = co_await aw_int;
            if (v < 5) {
                co_yield {};
            } else {
                break;
            }
        }

        // Main loop
        for (;;) {
            int v = co_await aw_int;
            safe_out("[slot coro] int = " + std::to_string(v) + "\n");
            co_yield {};
        }
    };

    auto aw_int = hub.make_listener<int>();
    event_signal<int> sig_int;

    sig_int.connect(&aw_int);
    slot s = make_slot(aw_int);
    hub.bind_slot(s);

    s.spawn(); // start coro

    std::thread emit_int([&] {
        // 0→10
        for (int i = 0; i <= 10; i++) {
            safe_out("[emit-int] emit(" + std::to_string(i) + ")\n");
            sig_int.emit(i);
            std::this_thread::sleep_for(10ms);
        }
        // 9→0
        for (int i = 9; i >= 0; i--) {
            safe_out("[emit-int] emit(" + std::to_string(i) + ")\n");
            sig_int.emit(i);
            std::this_thread::sleep_for(10ms);
        }
    });

    emit_int.join();

    const std::string log = out.str();

    REQUIRE(log.find("[slot coro] started") != std::string::npos);

    std::string tag10 = "[slot coro] int = 10";
    size_t pos_first10 = log.find(tag10);
    REQUIRE(pos_first10 != std::string::npos);

    std::string log_before10 = log.substr(0, pos_first10);

    for (int k = 0; k <= 5; k++) {
        std::string t = "[slot coro] int = " + std::to_string(k);
        REQUIRE(log_before10.find(t) == std::string::npos);
    }

    auto count_occ = [&](const std::string &needle) {
        size_t pos = 0, cnt = 0;
        while ((pos = log.find(needle, pos)) != std::string::npos) {
            cnt++;
            pos += needle.size();
        }
        return cnt;
    };

    for (int k = 0; k <= 5; k++) {
        std::string t = "[slot coro] int = " + std::to_string(k) + "\n";
        REQUIRE(count_occ(t) == 1);
    }

    for (int k = 6; k <= 9; k++) {
        std::string t = "[slot coro] int = " + std::to_string(k) + "\n";
        REQUIRE(count_occ(t) == 2);
    }

    REQUIRE(count_occ("[slot coro] int = 10\n") == 1);
}

TEST_CASE("Multi-Signal Single-Listener Test") {
    using namespace std::chrono_literals;

    using jh::async::slot;
    using jh::async::slot_hub;
    using jh::async::listener;
    using jh::async::event_signal;

    using Event = std::tuple<size_t, int>;   // <signal_id, value>

    std::ostringstream out{};
    std::mutex out_mtx;
    auto safe_out = [&](const std::string &s) {
        std::lock_guard guard(out_mtx);
        out << s;
    };

    slot_hub hub(1000ms);

    // multi-signal listener
    auto ml = hub.make_listener<Event>();


    // slot
    auto make_slot = [&](listener<Event>& ml) -> slot {
        safe_out("[slot coro] started\n");

        for (;;) {
            auto [idx, v] = co_await ml;

            if (idx == 0)
                safe_out("[slot coro] int1 = " + std::to_string(v)+ "\n");
            else if (idx == 1)
                safe_out("[slot coro] int2 = " + std::to_string(v)+ "\n");

            co_yield {};
        }
    };

    slot s = make_slot(ml);
    hub.bind_slot(s);

    s.spawn();

    // thread 1: emits 0..4 every 10ms
    std::thread t1([&] {
        event_signal<Event> sig1;
        sig1.connect(&ml);
        for (int i = 0; i < 5; i++) {
            safe_out("[emit-1] emit(" + std::to_string(i)+ ")\n");
            sig1.emit(0, i);
            std::this_thread::sleep_for(10ms);
        }
    });

    // thread 2: emits 10..14 every 20ms
    std::thread t2([&] {
        event_signal<Event> sig2;
        sig2.connect(&ml);
        for (int i = 10; i < 15; i++) {
            safe_out("[emit-2] emit(" + std::to_string(i)+ ")\n");
            sig2.emit(1, i);
            std::this_thread::sleep_for(20ms);
        }
    });

    t1.join();
    t2.join();

    const std::string log = out.str();

    REQUIRE(log.find("[slot coro] started") != std::string::npos);

    // ---- For signal 1 (0..4), expect int1 = N ----
    for (int v = 0; v < 5; v++) {
        std::string emit_tag = "[emit-1] emit(" + std::to_string(v) + ")\n";
        std::string slot_tag = "[slot coro] int1 = " + std::to_string(v) + "\n";
        std::string impossible_slot_tag = "[slot coro] int2 = " + std::to_string(v) + "\n";

        // emit must appear
        auto pos_emit = log.find(emit_tag);
        REQUIRE(pos_emit != std::string::npos);

        // valid slot output must appear AFTER corresponding emit
        auto pos_slot = log.find(slot_tag, pos_emit);
        REQUIRE(pos_slot != std::string::npos);

        // impossible output must *never* appear anywhere
        REQUIRE(log.find(impossible_slot_tag) == std::string::npos);
    }

    // ---- For signal 2 (10..14), expect int2 = N ----
    for (int v = 10; v < 15; v++) {
        std::string emit_tag = "[emit-2] emit(" + std::to_string(v) + ")\n";
        std::string slot_tag = "[slot coro] int2 = " + std::to_string(v) + "\n";
        std::string impossible_slot_tag = "[slot coro] int1 = " + std::to_string(v) + "\n";

        // emit must appear
        auto pos_emit = log.find(emit_tag);
        REQUIRE(pos_emit != std::string::npos);

        // valid slot output must appear AFTER corresponding emit
        auto pos_slot = log.find(slot_tag, pos_emit);
        REQUIRE(pos_slot != std::string::npos);

        // impossible output must *never* appear anywhere
        REQUIRE(log.find(impossible_slot_tag) == std::string::npos);
    }

}

TEST_CASE("Different-Type Event Test") {
    using namespace std::chrono_literals;

    using jh::async::slot;
    using jh::async::slot_hub;
    using jh::async::listener;
    using jh::async::event_signal;

    using Payload = std::variant<int, std::string>;
    using Event = std::tuple<size_t, Payload>;  // <signal_id, payload>

    // expected values
    std::vector<int> expected_int1 = {0, 1, 2};
    std::vector<int> expected_int2 = {10, 11, 12};
    std::vector<std::string> expected_str = {"A", "B", "C"};

    // actual collected values
    std::vector<int> vec_int1, vec_int2;
    std::vector<std::string> vec_str;

    std::ostringstream out{};
    std::mutex out_mtx;
    auto safe_out = [&](const std::string &s) {
        std::lock_guard guard(out_mtx);
        out << s;
    };

    slot_hub hub(1000ms);

    auto ml = hub.make_listener<Event>();

    // slot coroutine
    auto make_slot = [&](listener<Event> &ml) -> slot {
        safe_out("[slot coro] started\n");

        for (;;) {
            // Manually unpack to avoid gcc warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            auto ev = co_await ml;
            auto idx = std::get<0>(ev);
            auto payload = std::get<1>(ev);
#pragma GCC diagnostic pop
            if (idx == 0) {
                int v = std::get<int>(payload);
                vec_int1.emplace_back(v);
                safe_out("[slot coro] int1 = " + std::to_string(v) + "\n");
            }
            else if (idx == 1) {
                int v = std::get<int>(payload);
                vec_int2.emplace_back(v);
                safe_out("[slot coro] int2 = " + std::to_string(v) + "\n");
            }
            else if (idx == 2) {
                const std::string &s = std::get<std::string>(payload);
                vec_str.emplace_back(s);
                safe_out("[slot coro] str = " + s + "\n");
            }

            co_yield {};
        }
    };

    slot s = make_slot(ml);
    hub.bind_slot(s);

    s.spawn();

    // emitters
    std::thread t1([&] {
        event_signal<Event> sig;
        sig.connect(&ml);
        for (int v : expected_int1) {
            safe_out("[emit-0] emit(" + std::to_string(v) + ")\n");
            sig.emit(0, v);
            std::this_thread::sleep_for(10ms);
        }
    });

    std::thread t2([&] {
        event_signal<Event> sig;
        sig.connect(&ml);
        for (int v : expected_int2) {
            safe_out("[emit-1] emit(" + std::to_string(v) + ")\n");
            sig.emit(1, v);
            std::this_thread::sleep_for(15ms);
        }
    });

    std::thread t3([&] {
        event_signal<Event> sig;
        sig.connect(&ml);
        for (auto &s : expected_str) {
            safe_out("[emit-2] emit(" + s + ")\n");
            sig.emit(2, s);
            std::this_thread::sleep_for(20ms);
        }
    });

    t1.join();
    t2.join();
    t3.join();

    const std::string log = out.str();

    REQUIRE(log.find("[slot coro] started") != std::string::npos);

    // compare vectors (strict equality)
    REQUIRE(vec_int1 == expected_int1);
    REQUIRE(vec_int2 == expected_int2);
    REQUIRE(vec_str == expected_str);
}

TEST_CASE("Two-Listener Switch Test") {
    using namespace std::chrono_literals;

    using jh::async::slot;
    using jh::async::slot_hub;
    using jh::async::listener;
    using jh::async::event_signal;

    const int STOP_VALUE = 999;

    // expected
    std::vector<int> expected_ints = {1, 2, 3, STOP_VALUE};
    std::vector<std::string> expected_strs = {"A", "B", "C"};

    // actual
    std::vector<int> vec_int;
    std::vector<std::string> vec_str;

    bool switched = false;   // external flag changed by slot

    // --- logging ---
    std::ostringstream out{};
    std::mutex out_mtx;
    auto safe_out = [&](const std::string &s) {
        std::lock_guard guard(out_mtx);
        out << s;
    };

    slot_hub hub(1000ms);

    auto li_int = hub.make_listener<int>();
    auto li_str = hub.make_listener<std::string>();

    auto make_slot = [&](listener<int> &aw_int, listener<std::string> &aw_str) -> slot {
        safe_out("[slot coro] started\n");

        // Phase 1: read ints
        for (;;) {
            int v = co_await aw_int;
            vec_int.emplace_back(v);
            safe_out("[slot coro] int = " + std::to_string(v) + "\n");

            if (v == STOP_VALUE) {
                switched = true;  // inform external
                break;            // leave int loop
            }
            co_yield {};
        }

        // Phase 2: read strings
        for (;;) {
            auto s = co_await aw_str;
            vec_str.emplace_back(s);
            safe_out("[slot coro] str = " + s + "\n");
            co_yield {};
        }
    };

    slot s = make_slot(li_int, li_str);
    hub.bind_slot(s);
    s.spawn();

    // --- emitters ---

    // emit ints 1,2,3,STOP_VALUE
    std::thread t1([&] {
        event_signal<int> sig;
        sig.connect(&li_int);

        for (int v : expected_ints) {
            if (switched) break;  // slot switched, stop sending int

            safe_out("[emit-int] " + std::to_string(v) + "\n");
            sig.emit(v);
            std::this_thread::sleep_for(10ms);
        }
    });

    // after switch, emit strings A,B,C
    std::thread t2([&] {
        event_signal<std::string> sig;
        sig.connect(&li_str);

        // wait a bit until slot sets flag
        while (!switched) {
            std::this_thread::sleep_for(2ms);
        }

        for (auto &sval : expected_strs) {
            safe_out("[emit-str] " + sval + "\n");
            sig.emit(sval);
            std::this_thread::sleep_for(10ms);
        }
    });

    t1.join();
    t2.join();

    const std::string log = out.str();

    REQUIRE(log.find("[slot coro] started") != std::string::npos);

    // vector match
    REQUIRE(vec_int == expected_ints);
    REQUIRE(vec_str == expected_strs);
}
