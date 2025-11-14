#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

#include <sstream>
#include <vector>
#include <string>

#include "jh/async"

TEST_CASE("Basic Order") {
    std::ostringstream out;

    auto test_basic_order = [&](int id) -> jh::async::fiber {
        out << "[basic] fiber " << id << " A\n";
        co_await jh::async::resume_tag;

        out << "[basic] fiber " << id << " B\n";
        co_await jh::async::resume_tag;

        out << "[basic] fiber " << id << " C\n";
        co_await jh::async::resume_tag;

        out << "[basic] fiber " << id << " finished\n";
    };

    std::vector<jh::async::fiber> fs;
    fs.emplace_back(test_basic_order(1));
    fs.emplace_back(test_basic_order(2));
    fs.emplace_back(test_basic_order(3));

    for (auto &f: fs) f.resume();

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (auto &f: fs) {
            if (!f.done()) {
                f.resume();
                all_done = false;
            }
        }
    }

    std::string expected =
            "[basic] fiber 1 A\n"
            "[basic] fiber 2 A\n"
            "[basic] fiber 3 A\n"
            "[basic] fiber 1 B\n"
            "[basic] fiber 2 B\n"
            "[basic] fiber 3 B\n"
            "[basic] fiber 1 C\n"
            "[basic] fiber 2 C\n"
            "[basic] fiber 3 C\n"
            "[basic] fiber 1 finished\n"
            "[basic] fiber 2 finished\n"
            "[basic] fiber 3 finished\n";

    REQUIRE(out.str() == expected);
}

TEST_CASE("Early End") {
    std::ostringstream out;

    auto test_early_end = [&](int x) -> jh::async::fiber {
        int k = x;

        for (int i = 0; i < 5; ++i) {
            x >>= 1;
            out << "[early] step " << i << ", x=" << x << "(" << k << ")\n";

            if (x == 0) {
                out << "[early] x reached 0, early terminate(" << k << ")\n";
                co_return;
            }
            co_await jh::async::resume_tag;
        }
        out << "[early] finished normally\n";
    };

    jh::async::fiber f1 = test_early_end(32);
    jh::async::fiber f2 = test_early_end(7);

    std::vector<jh::async::fiber *> fs = {&f1, &f2};

    for (auto *fp: fs) fp->resume();

    bool done = false;
    while (!done) {
        done = true;
        for (auto *fp: fs) {
            if (!fp->done()) {
                done = false;
                fp->resume();
            }
        }
    }

    std::string expected =
            "[early] step 0, x=16(32)\n"
            "[early] step 0, x=3(7)\n"
            "[early] step 1, x=8(32)\n"
            "[early] step 1, x=1(7)\n"
            "[early] step 2, x=4(32)\n"
            "[early] step 2, x=0(7)\n"
            "[early] x reached 0, early terminate(7)\n"
            "[early] step 3, x=2(32)\n"
            "[early] step 4, x=1(32)\n"
            "[early] finished normally\n";

    REQUIRE(out.str() == expected);
}

TEST_CASE("Move Semantics") {
    std::ostringstream out;

    auto test_move = [&]() -> jh::async::fiber {
        out << "[move] A\n";
        co_await jh::async::resume_tag;

        out << "[move] B\n";
        co_await jh::async::resume_tag;

        out << "[move] done\n";
    };

    jh::async::fiber f = test_move();
    f.resume();

    jh::async::fiber f2 = std::move(f);
    f2.resume();
    f2.resume();

    std::string expected =
            "[move] A\n"
            "[move] B\n"
            "[move] done\n";

    REQUIRE(out.str() == expected);
}

TEST_CASE("Multi Step") {
    std::ostringstream out;

    auto test_multi = [&](int id) -> jh::async::fiber {
        for (int i = 0; i < 3; ++i) {
            out << "[multi] fiber " << id << " step " << i << "\n";
            co_await jh::async::resume_tag;
        }
    };

    std::vector<jh::async::fiber> fs;
    fs.emplace_back(test_multi(1));
    fs.emplace_back(test_multi(2));

    for (auto &f: fs) f.resume();

    bool done = false;
    while (!done) {
        done = true;
        for (auto &f: fs) {
            if (!f.done()) {
                done = false;
                f.resume();
            }
        }
    }

    std::string expected =
            "[multi] fiber 1 step 0\n"
            "[multi] fiber 2 step 0\n"
            "[multi] fiber 1 step 1\n"
            "[multi] fiber 2 step 1\n"
            "[multi] fiber 1 step 2\n"
            "[multi] fiber 2 step 2\n";

    REQUIRE(out.str() == expected);
}

TEST_CASE("Lambda Fiber") {
    std::ostringstream out;

    auto f1 = [&]() -> jh::async::fiber {
        out << "[lambda] A\n";
        co_await jh::async::resume_tag;

        out << "[lambda] B\n";
        co_await jh::async::resume_tag;

        out << "[lambda] C done\n";
    }();

    auto f2 = [&]() -> jh::async::fiber {
        for (int i = 0; i < 3; ++i) {
            out << "[lambda] loop " << i << "\n";
            co_await jh::async::resume_tag;
        }
        out << "[lambda] finished\n";
    }();

    std::vector<jh::async::fiber *> fs = {&f1, &f2};

    for (auto *fp: fs) fp->resume();

    bool done = false;
    while (!done) {
        done = true;
        for (auto *fp: fs) {
            if (!fp->done()) {
                done = false;
                fp->resume();
            }
        }
    }

    std::string expected =
            "[lambda] A\n"
            "[lambda] loop 0\n"
            "[lambda] B\n"
            "[lambda] loop 1\n"
            "[lambda] C done\n"
            "[lambda] loop 2\n"
            "[lambda] finished\n";

    REQUIRE(out.str() == expected);
}
