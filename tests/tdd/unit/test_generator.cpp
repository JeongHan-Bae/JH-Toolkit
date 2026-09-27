#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <numeric>
#include <random>
#include "jh/generator"

namespace test {
    jh::async::generator<int> range(const int end) {
        for (int i = 0; i < end; ++i) {
            co_yield i;
        }
    }

    jh::async::generator<int> range(const int start, const int end) {
        for (int i = start; i < end; ++i) {
            co_yield i;
        }
    }

    jh::async::generator<int> range(const int start, const int end, const int step) {
        for (int i = start; i < end; i += step) {
            co_yield i;
        }
    }

    jh::async::generator<int, int> countdown(int start) {
        int step = 1; // Default step size if no value is sent
        while (start > 0) {
            volatile int next_step = step;
            step = co_await next_step; // NOLINT
            start -= step;
            co_yield start;
        }
    }
}

// **Simple Test**

namespace test {
void tiny_test_case_1_body(const int selected_section) {
    jh::async::generator<int> my_generator = []() -> jh::async::generator<int> {
        for (int i = 1; i <= 5; ++i) {
            co_yield i;
        }
    }();
    if (selected_section == 1) {
        auto i = 1;
        while (my_generator.next()) {
            jh::test::tiny_test::expect(static_cast<bool>((my_generator.value().value() == i)), "my_generator.value().value() == i");
            ++i;
        }
    }

}
void tiny_test_case_1_section_1() { tiny_test_case_1_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Simple Test / Default">
    : jh::test::tiny_test::test_definition<"Simple Test / Default", &::test::tiny_test_case_1_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 1", jh::test::tiny_test::test<"Simple Test / Default">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 1"> registration_1{};
}


// **Base Case**

namespace test {
void tiny_test_case_2_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000);

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            // Creates a separate test case per iteration
            int start = dist(gen);
            int end = dist(gen);

            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Prevent empty ranges

            auto generator = test::range(start, end);
            int expected = start;

            while (generator.next()) {
                jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == expected)), "generator.value().value() == expected");
                expected++;
            }

            jh::test::tiny_test::expect(static_cast<bool>((expected == end)), "expected == end");
        }
    }

}
void tiny_test_case_2_section_1() { tiny_test_case_2_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Basic Generator Test / section 1">
    : jh::test::tiny_test::test_definition<"Basic Generator Test / section 1", &::test::tiny_test_case_2_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 2", jh::test::tiny_test::test<"Basic Generator Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 2"> registration_2{};
}



// **Edge Case: Empty Range**

namespace test {
void tiny_test_case_3_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000);

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            // Each test case is independent
            const int end = dist(gen);
            const int start = end + dist(gen); // Ensure start >= end

            auto generator = test::range(start, end);

            jh::test::tiny_test::expect_not(static_cast<bool>((generator.next())), "generator.next()"); // Generator should always be empty
        }
    }

}
void tiny_test_case_3_section_1() { tiny_test_case_3_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Empty Generator Test / section 1">
    : jh::test::tiny_test::test_definition<"Empty Generator Test / section 1", &::test::tiny_test_case_3_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 3", jh::test::tiny_test::test<"Empty Generator Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 3"> registration_3{};
}


// **Edge Case: Step**

namespace test {
void tiny_test_case_4_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Step size must be > 0

    constexpr std::uint64_t total_tests = 128;

    for (std::uint64_t i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            const int step = step_dist(gen); // Ensure step > 0

            auto generator = test::range(start, end, step);

            // Compute expected values
            std::vector<int> expected;
            for (int val = start; val < end; val += step) {
                expected.push_back(val);
            }

            // Compare generated sequence with expected
            std::uint64_t index = 0;
            while (generator.next()) {
                jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == expected[index])), "generator.value().value() == expected[index]");
                index++;
            }

            jh::test::tiny_test::expect(static_cast<bool>((index == expected.size())), "index == expected.size()"); // Ensure full iteration
        }
    }

}
void tiny_test_case_4_section_1() { tiny_test_case_4_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Step Generator Test / section 1">
    : jh::test::tiny_test::test_definition<"Step Generator Test / section 1", &::test::tiny_test_case_4_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 4", jh::test::tiny_test::test<"Step Generator Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 4"> registration_4{};
}


// **Convert Generator to Vector**

namespace test {
void tiny_test_case_5_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start & end

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            auto generator = test::range(start, end);

            // Generate expected vector using iota
            std::vector<int> expected(end - start);
            std::iota(expected.begin(), expected.end(), start);

            // Convert generator to vector
            const std::vector<int> generated = to_vector(generator);

            // Compare generated vector with expected vector
            jh::test::tiny_test::expect(static_cast<bool>((generated == expected)), "generated == expected");
        }
    }

}
void tiny_test_case_5_section_1() { tiny_test_case_5_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator to Vector Test / section 1">
    : jh::test::tiny_test::test_definition<"Generator to Vector Test / section 1", &::test::tiny_test_case_5_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 5", jh::test::tiny_test::test<"Generator to Vector Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 5"> registration_5{};
}


// **Convert Generator to deque**

namespace test {
void tiny_test_case_6_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start & end

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            auto generator = test::range(start, end);

            // Generate expected sequence using std::iota
            std::vector<int> expected_vec(end - start);
            std::iota(expected_vec.begin(), expected_vec.end(), start);

            // Convert expected vector to deque for comparison
            std::deque expected_deque(expected_vec.begin(), expected_vec.end());

            // Convert generator to deque
            const std::deque<int> generated_deque = to_deque(generator);

            // Compare generated deque with expected deque
            jh::test::tiny_test::expect(static_cast<bool>((generated_deque == expected_deque)), "generated_deque == expected_deque");
        }
    }

}
void tiny_test_case_6_section_1() { tiny_test_case_6_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator to deque Test / section 1">
    : jh::test::tiny_test::test_definition<"Generator to deque Test / section 1", &::test::tiny_test_case_6_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 6", jh::test::tiny_test::test<"Generator to deque Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 6"> registration_6{};
}


// **Convert Generator with Steps to Vector**

namespace test {
void tiny_test_case_7_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(0, 10000); // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Ensure step > 0

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            const int step = step_dist(gen); // Ensure step > 0

            auto generator = test::range(start, end, step);

            // Generate expected values manually
            std::vector<int> expected;
            for (int val = start; val < end; val += step) {
                expected.push_back(val);
            }

            // Convert generator to vector
            const std::vector<int> generated = to_vector(generator);

            // Compare generated vector with expected vector
            jh::test::tiny_test::expect(static_cast<bool>((generated == expected)), "generated == expected");
        }
    }

}
void tiny_test_case_7_section_1() { tiny_test_case_7_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Step Generator to Vector Test / section 1">
    : jh::test::tiny_test::test_definition<"Step Generator to Vector Test / section 1", &::test::tiny_test_case_7_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 7", jh::test::tiny_test::test<"Step Generator to Vector Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 7"> registration_7{};
}


// **Convert Generator with Steps to deque**

namespace test {
void tiny_test_case_8_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(0, 10000); // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Ensure step > 0

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            const int step = step_dist(gen); // Ensure step > 0

            auto generator = test::range(start, end, step);

            // Generate expected values manually
            std::vector<int> expected_vec;
            for (int val = start; val < end; val += step) {
                expected_vec.push_back(val);
            }

            // Convert expected vector to deque for comparison
            std::deque expected_deque(expected_vec.begin(), expected_vec.end());

            // Convert generator to deque
            const std::deque<int> generated_deque = to_deque(generator);

            // Compare generated deque with expected deque
            jh::test::tiny_test::expect(static_cast<bool>((generated_deque == expected_deque)), "generated_deque == expected_deque");
        }
    }

}
void tiny_test_case_8_section_1() { tiny_test_case_8_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Step Generator to deque Test / section 1">
    : jh::test::tiny_test::test_definition<"Step Generator to deque Test / section 1", &::test::tiny_test_case_8_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 8", jh::test::tiny_test::test<"Step Generator to deque Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 8"> registration_8{};
}


// **Negative Step Range (Should be Empty)**

namespace test {
void tiny_test_case_9_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start and end
    std::uniform_int_distribution step_dist(1, 100); // Positive values, later negated

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const int end = dist(gen); // Random end
            const int start = end + dist(gen); // Ensure start > end
            const int step = -step_dist(gen); // Ensure step < 0

            auto generator = test::range(start, end, step);

            jh::test::tiny_test::expect_not(static_cast<bool>((generator.next())), "generator.next()"); // Should always be empty
        }
    }

}
void tiny_test_case_9_section_1() { tiny_test_case_9_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Negative Step Generator Test / section 1">
    : jh::test::tiny_test::test_definition<"Negative Step Generator Test / section 1", &::test::tiny_test_case_9_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 9", jh::test::tiny_test::test<"Negative Step Generator Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 9"> registration_9{};
}


// **Large Step Size (Should Only Yield One Value)**

namespace test {
void tiny_test_case_10_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(0, 10000); // Range for start
    std::uniform_int_distribution range_dist(1, 1000); // Range for end (ensuring it's larger than start)

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const int start = dist(gen);
            const int range = range_dist(gen);
            const int end = start + range; // Ensure end > start

            const int step = end - start + dist(gen) + 1; // Step is guaranteed to be larger than the range

            auto generator = test::range(start, end, step);

            jh::test::tiny_test::expect(static_cast<bool>((generator.next())), "generator.next()"); // Should yield once
            jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == start)), "generator.value().value() == start"); // The First value must be `start`
            jh::test::tiny_test::expect_not(static_cast<bool>((generator.next())), "generator.next()"); // No more values should be yielded
        }
    }

}
void tiny_test_case_10_section_1() { tiny_test_case_10_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Large Step Generator Test / section 1">
    : jh::test::tiny_test::test_definition<"Large Step Generator Test / section 1", &::test::tiny_test_case_10_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 10", jh::test::tiny_test::test<"Large Step Generator Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 10"> registration_10{};
}


// **to_vector with Input Value**

namespace test {
void tiny_test_case_11_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(5, 10000); // Random starting value
    std::uniform_int_distribution step_dist(1, 50); // Ensure positive step

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = start_dist(gen);
            const int step = step_dist(gen);

            auto generator = test::countdown(start);

            // Compute expected values manually
            std::vector<int> expected;
            while (start > 0) {
                start -= step;
                expected.push_back(start);
                if (start <= 0) break; // Stop at 0
            }

            // Convert generator to vector with input step
            const std::vector<int> generated = to_vector(generator, step);

            // Compare generated vector with expected vector
            jh::test::tiny_test::expect(static_cast<bool>((generated == expected)), "generated == expected");
        }
    }

}
void tiny_test_case_11_section_1() { tiny_test_case_11_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator with Single Input / section 1">
    : jh::test::tiny_test::test_definition<"Generator with Single Input / section 1", &::test::tiny_test_case_11_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 11", jh::test::tiny_test::test<"Generator with Single Input / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 11"> registration_11{};
}


// **to_vector with Input Sequence**

namespace test {
void tiny_test_case_12_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(5, 10000); // Random starting value
    std::uniform_int_distribution step_dist(1, 50); // Random step values
    std::uniform_int_distribution step_count_dist(1, 20); // Number of steps

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int start = start_dist(gen);
            const int step_count = step_count_dist(gen);

            // Generate a sequence of random steps
            std::vector<int> steps(step_count);
            for (int &step: steps) {
                step = step_dist(gen);
            }

            auto generator = test::countdown(start);

            // Compute expected values manually
            std::vector<int> expected;
            size_t index = 0;
            while (start > 0 && index < steps.size()) {
                start -= steps[index++];
                expected.push_back(start);
                if (start <= 0) break; // Stop at 0
            }

            // Convert generator to vector with input steps
            const std::vector<int> generated = to_vector(generator, steps);

            // Compare generated vector with expected vector
            jh::test::tiny_test::expect(static_cast<bool>((generated == expected)), "generated == expected");
        }
    }

}
void tiny_test_case_12_section_1() { tiny_test_case_12_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator with Vector Input / section 1">
    : jh::test::tiny_test::test_definition<"Generator with Vector Input / section 1", &::test::tiny_test_case_12_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 12", jh::test::tiny_test::test<"Generator with Vector Input / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 12"> registration_12{};
}



namespace test {
void tiny_test_case_13_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution step_dist(1, 50); // Step values
    std::uniform_int_distribution step_count_dist(1, 20); // Number of steps

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const int step_count = step_count_dist(gen);

            // Generate a sequence of random steps
            std::vector<int> steps(step_count);
            for (int &step: steps) {
                step = step_dist(gen);
            }

            // Compute total sum as the starting value
            int sum = std::accumulate(steps.begin(), steps.end(), 0);

            auto generator = test::countdown(sum);
            size_t index = 0;

            while (generator.next()) {
                jh::test::tiny_test::expect(static_cast<bool>((index < steps.size())), "index < steps.size()"); // Ensure we don't access out of bounds

                const int decrement = steps[index++];
                if (!generator.send(decrement)) break; // Stop if generator ends
                sum -= decrement;

                jh::test::tiny_test::expect(static_cast<bool>((generator.value() == sum)), "generator.value() == sum");
            }

            jh::test::tiny_test::expect(static_cast<bool>((sum == 0)), "sum == 0"); // Ensure countdown fully decremented to 0
        }
    }

}
void tiny_test_case_13_section_1() { tiny_test_case_13_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator with 'Send' Step by Step / section 1">
    : jh::test::tiny_test::test_definition<"Generator with 'Send' Step by Step / section 1", &::test::tiny_test_case_13_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 13", jh::test::tiny_test::test<"Generator with 'Send' Step by Step / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 13"> registration_13{};
}



namespace test {
void tiny_test_case_14_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution step_dist(1, 50); // Step values
    std::uniform_int_distribution step_count_dist(1, 20); // Number of steps

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const int step_count = step_count_dist(gen);

            // Generate a sequence of random steps
            std::vector<int> steps(step_count);
            for (int &step: steps) {
                step = step_dist(gen);
            }

            // Compute total sum as the starting value
            int sum = std::accumulate(steps.begin(), steps.end(), 0);

            auto generator = test::countdown(sum);
            size_t index = 0;

            while (index < steps.size() && generator.send_ite(steps[index])) {
                sum -= steps[index];

                jh::test::tiny_test::expect(static_cast<bool>((generator.value() == sum)), "generator.value() == sum");
                ++index;
            }

            jh::test::tiny_test::expect(static_cast<bool>((sum == 0)), "sum == 0"); // Ensure countdown fully decremented to 0
        }
    }

}
void tiny_test_case_14_section_1() { tiny_test_case_14_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator with 'Send_Ite' Step by Step / section 1">
    : jh::test::tiny_test::test_definition<"Generator with 'Send_Ite' Step by Step / section 1", &::test::tiny_test_case_14_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 14", jh::test::tiny_test::test<"Generator with 'Send_Ite' Step by Step / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 14"> registration_14{};
}



namespace test {
void tiny_test_case_15_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution size_dist(1, 100); // deque size
    std::uniform_int_distribution value_dist(-10000, 10000); // Random values

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const int size = size_dist(gen);

            // Generate a random deque
            std::deque<int> original_deque;
            for (int j = 0; j < size; ++j) {
                original_deque.push_back(value_dist(gen));
            }

            // Convert deque to generator
            auto generator = jh::async::make_generator(original_deque);

            // Convert generator back to deque
            const std::deque<int> generated_deque = to_deque(generator);

            // Ensure the final deque matches the original deque
            jh::test::tiny_test::expect(static_cast<bool>((generated_deque == original_deque)), "generated_deque == original_deque");
        }
    }

}
void tiny_test_case_15_section_1() { tiny_test_case_15_body(1); }
}
template<>
struct jh::test::tiny_test::test<"deque -> jh::generator -> deque Equivalence Test / section 1">
    : jh::test::tiny_test::test_definition<"deque -> jh::generator -> deque Equivalence Test / section 1", &::test::tiny_test_case_15_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 15", jh::test::tiny_test::test<"deque -> jh::generator -> deque Equivalence Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 15"> registration_15{};
}




namespace test {
void tiny_test_case_16_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(-100, 100); // Random start value
    std::uniform_int_distribution length_dist(1, 100); // Random length of the range

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            // Generate a random range [start, start + length)
            const int start = start_dist(gen);
            const int length = length_dist(gen);
            const int end = start + length;

            // Create a generator for the range
            auto generator = test::range(start, end);

            // Iterate over the generator and validate values
            int expected_value = start;
            for (const auto a: generator) {
                jh::test::tiny_test::expect(static_cast<bool>((a == expected_value)), "a == expected_value");
                ++expected_value;
            }

            // Ensure iteration covered all expected values
            jh::test::tiny_test::expect(static_cast<bool>((expected_value == end)), "expected_value == end");
        }
    }

}
void tiny_test_case_16_section_1() { tiny_test_case_16_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Ranged-For Loop test / section 1">
    : jh::test::tiny_test::test_definition<"Ranged-For Loop test / section 1", &::test::tiny_test_case_16_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 16", jh::test::tiny_test::test<"Ranged-For Loop test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 16"> registration_16{};
}



namespace test {
void tiny_test_case_17_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(-100, 100); // Random start value
    std::uniform_int_distribution length_dist(1, 100); // Random length of the range

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            // Generate a random range [start, start + length)
            const int start = start_dist(gen);
            const int length = length_dist(gen);
            const int end = start + length;

            // Create a generator for the range
            auto range_ = jh::to_range([&] { return test::range(start, end); });
            // Iterate over the generator and validate values
            int expected_value = start;
            std::ranges::for_each(range_, [&](const int a) {
                jh::test::tiny_test::expect(static_cast<bool>((a == expected_value)), "a == expected_value");
                ++expected_value;
            });

            // Ensure iteration covered all expected values
            jh::test::tiny_test::expect(static_cast<bool>((expected_value == end)), "expected_value == end");
        }
    }

}
void tiny_test_case_17_section_1() { tiny_test_case_17_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Ranged-For Range Loop test / section 1">
    : jh::test::tiny_test::test_definition<"Ranged-For Range Loop test / section 1", &::test::tiny_test_case_17_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 17", jh::test::tiny_test::test<"Ranged-For Range Loop test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 17"> registration_17{};
}



namespace test {
void tiny_test_case_18_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(-100, 100); // Random start value
    std::uniform_int_distribution length_dist(1, 100); // Random length of the range

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            // Generate a random range [start, start + length)
            const int start = start_dist(gen);
            const int length = length_dist(gen);
            const int end = start + length;

            // Create a generator for the range
            auto generator = test::range(start, end);

            // Iterate over the generator and validate values
            int expected_value = start;
            for (auto iter = generator.begin(); iter != jh::async::generator<int, jh::typed::monostate>::end(); ++iter) {
                jh::test::tiny_test::expect(static_cast<bool>((*iter == expected_value)), "*iter == expected_value");
                ++expected_value;
            }

            // Ensure iteration covered all expected values
            jh::test::tiny_test::expect(static_cast<bool>((expected_value == end)), "expected_value == end");
        }
    }

}
void tiny_test_case_18_section_1() { tiny_test_case_18_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Iterator For Loop test / section 1">
    : jh::test::tiny_test::test_definition<"Iterator For Loop test / section 1", &::test::tiny_test_case_18_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 18">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 18", jh::test::tiny_test::test<"Iterator For Loop test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 18"> registration_18{};
}



namespace test {
void tiny_test_case_19() {
    auto generator = test::range(10); // Create a simple generator

    // 1️⃣ Manually advance the generator to confirm initial state
    generator.next();
    auto init_val = generator.value().value();

    // 2️⃣ Create iterator **after** generator has been advanced
    auto iter = generator.begin();

    // 3️⃣ Iterator should return the next value, NOT the `init_val`
    jh::test::tiny_test::expect(static_cast<bool>((iter != generator.end())), "iter != generator.end()");

    auto iter_val = *iter;

    // Ensure iterator value is different from the initial generator value
    jh::test::tiny_test::expect(static_cast<bool>((iter_val != init_val)), "iter_val != init_val");

    // 4️⃣ Ensure generator has been advanced to the iterator's value
    jh::test::tiny_test::expect(static_cast<bool>((generator.value() == iter_val)), "generator.value() == iter_val");

    // 5️⃣ Calling *iter again does NOT advance the generator further
    jh::test::tiny_test::expect(static_cast<bool>((*iter == iter_val)), "*iter == iter_val");
    jh::test::tiny_test::expect(static_cast<bool>((generator.value() == iter_val)), "generator.value() == iter_val");

}
}
template<>
struct jh::test::tiny_test::test<"Generator Iterator Consumption Test">
    : jh::test::tiny_test::test_definition<"Generator Iterator Consumption Test", &::test::tiny_test_case_19> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 19">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 19", jh::test::tiny_test::test<"Generator Iterator Consumption Test">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 19"> registration_19{};
}



namespace test {
void tiny_test_case_20_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution size_dist(1, 100);
    std::uniform_int_distribution value_dist(-10000, 10000);

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const int size = size_dist(gen);

            std::vector<int> original;
            original.reserve(size);
            for (int j = 0; j < size; ++j) {
                original.push_back(value_dist(gen));
            }

            auto range_ = jh::to_range([&] { return jh::async::make_generator(original); });

            // First collection
            std::vector<int> first_pass;
            for (int x: range_) {
                first_pass.push_back(x);
            }
            jh::test::tiny_test::expect(static_cast<bool>((first_pass == original)), "first_pass == original");

            // Second collection
            std::vector<int> second_pass;
            for (int x: range_) {
                second_pass.push_back(x);
            }
            jh::test::tiny_test::expect(static_cast<bool>((second_pass == original)), "second_pass == original");
            jh::test::tiny_test::expect(static_cast<bool>((first_pass == second_pass)), "first_pass == second_pass");
        }
    }

}
void tiny_test_case_20_section_1() { tiny_test_case_20_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Generator to_range repeatable iteration test / section 1">
    : jh::test::tiny_test::test_definition<"Generator to_range repeatable iteration test / section 1", &::test::tiny_test_case_20_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 20">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 20", jh::test::tiny_test::test<"Generator to_range repeatable iteration test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 20"> registration_20{};
}


/// Throw and Catch Tests


namespace test {
void tiny_test_case_21() {
    using namespace jh::async;

    auto generator = []() -> jh::generator<int> {
        co_yield 1;
        throw std::runtime_error("Test exception");
        co_yield 2; // NOLINT has to declare to ensure reentering (never reached)
    }();

    // First OK
    jh::test::tiny_test::expect(static_cast<bool>((generator.next())), "generator.next()");
    jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == 1)), "generator.value().value() == 1");

    // Second -> should throw
    jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(generator.next()); }, "throws std::runtime_error: generator.next()");

}
}
template<>
struct jh::test::tiny_test::test<"Generator throws during execution">
    : jh::test::tiny_test::test_definition<"Generator throws during execution", &::test::tiny_test_case_21> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 21">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 21", jh::test::tiny_test::test<"Generator throws during execution">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 21"> registration_21{};
}



namespace test {
void tiny_test_case_22() {
    using namespace jh::async;

    auto generator = []() -> jh::generator<int> {
        throw std::logic_error("Immediate failure");
        co_return; // NOLINT has to declare to ensure reentering
    }();

    jh::test::tiny_test::expect_throw<std::logic_error>([&]() { (void)(generator.next()); }, "throws std::logic_error: generator.next()");

}
}
template<>
struct jh::test::tiny_test::test<"Generator throws immediately">
    : jh::test::tiny_test::test_definition<"Generator throws immediately", &::test::tiny_test_case_22> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 22">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 22", jh::test::tiny_test::test<"Generator throws immediately">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 22"> registration_22{};
}



namespace test {
void tiny_test_case_23() {
    using namespace jh::async;

    auto generator = []() -> jh::generator<int,int> {
        co_yield 0;
        int v = co_await int{};
        if (v == 42)
            throw std::runtime_error("send error");
        co_yield 1;
    }();

    jh::test::tiny_test::expect(static_cast<bool>((generator.next())), "generator.next()");
    jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == 0)), "generator.value().value() == 0");

    [&]() { bool did_not_throw = true; try { generator.send(1); } catch (...) { did_not_throw = false; } jh::test::tiny_test::expect(did_not_throw, "does not throw: generator.send(1)"); }();

    jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == 0)), "generator.value().value() == 0");

    jh::test::tiny_test::expect(static_cast<bool>((generator.next())), "generator.next()");
    jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == 1)), "generator.value().value() == 1");

    auto generator2 = []() -> jh::generator<int,int> {
        co_yield 0;
        int v = co_await int{};
        if (v == 42)
            throw std::runtime_error("send error");
        co_yield 1;
    }();

    jh::test::tiny_test::expect(static_cast<bool>((generator2.next())), "generator2.next()");
    jh::test::tiny_test::expect(static_cast<bool>((generator2.value().value() == 0)), "generator2.value().value() == 0");

    [&]() { bool did_not_throw = true; try { generator2.next(); } catch (...) { did_not_throw = false; } jh::test::tiny_test::expect(did_not_throw, "does not throw: generator2.next()"); }();

    jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(generator2.send(42)); }, "throws std::runtime_error: generator2.send(42)");

    auto generator3 = []() -> jh::generator<int,int> {
        co_yield 0;
        int v = co_await int{};
        if (v == 42)
            throw std::runtime_error("send error");
        co_yield 1;
    }();

    jh::test::tiny_test::expect(static_cast<bool>((generator3.next())), "generator3.next()");

    jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(generator3.send_ite(42)); }, "throws std::runtime_error: generator3.send_ite(42)");

}
}
template<>
struct jh::test::tiny_test::test<"Generator throws on send() and next()">
    : jh::test::tiny_test::test_definition<"Generator throws on send() and next()", &::test::tiny_test_case_23> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 23">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 23", jh::test::tiny_test::test<"Generator throws on send() and next()">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 23"> registration_23{};
}



namespace test {
void tiny_test_case_24() {
    using namespace jh::async;

    auto generator = []() -> jh::generator<int> {
        for (int i = 0; i < 3; ++i)
            co_yield i;
        throw std::runtime_error("end fail");
    }();

    int count = 0;

    try {
        for (int v : generator) {
            jh::test::tiny_test::expect(static_cast<bool>((v == count)), "v == count");
            ++count;
        }
        jh::test::tiny_test::expect(false, ("Exception expected but not thrown"));
    }
    catch (const std::runtime_error& e) {
        jh::test::tiny_test::expect(static_cast<bool>((std::string(e.what()) == "end fail")), "std::string(e.what()) == \"end fail\"");
        jh::test::tiny_test::expect(static_cast<bool>((count == 3)), "count == 3");
    }

}
}
template<>
struct jh::test::tiny_test::test<"Exception inside ranged-for consumption">
    : jh::test::tiny_test::test_definition<"Exception inside ranged-for consumption", &::test::tiny_test_case_24> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 24">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 24", jh::test::tiny_test::test<"Exception inside ranged-for consumption">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 24"> registration_24{};
}



namespace test {
void tiny_test_case_25() {
    using namespace jh::async;

    auto generator = []() -> jh::generator<int> {
        co_yield 1;
        throw std::runtime_error("explode");
    }();

    jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(to_vector(generator)); }, "throws std::runtime_error: to_vector(generator)");

}
}
template<>
struct jh::test::tiny_test::test<"to_vector propagates exceptions">
    : jh::test::tiny_test::test_definition<"to_vector propagates exceptions", &::test::tiny_test_case_25> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 25">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 25", jh::test::tiny_test::test<"to_vector propagates exceptions">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 25"> registration_25{};
}



namespace test {
void tiny_test_case_26() {
    using namespace jh::async;

    static bool cleaned = false;

    struct Cleaner {
        ~Cleaner() { cleaned = true; }
    };

    {
        auto generator = []() -> jh::generator<int> {
            Cleaner c; // local RAII object
            co_yield 1;
            throw std::runtime_error("boom");
        }();

        // First ok
        jh::test::tiny_test::expect(static_cast<bool>((generator.next())), "generator.next()");
        jh::test::tiny_test::expect(static_cast<bool>((generator.value().value() == 1)), "generator.value().value() == 1");

        // second throws
        jh::test::tiny_test::expect_throw([&]() { (void)(generator.next()); }, "throws: generator.next()");
    }

    jh::test::tiny_test::expect(static_cast<bool>((cleaned)), "cleaned");  // RAII must be executed

}
}
template<>
struct jh::test::tiny_test::test<"Generator destructor cleans up after exception">
    : jh::test::tiny_test::test_definition<"Generator destructor cleans up after exception", &::test::tiny_test_case_26> {};
template<>
struct jh::test::tiny_test::session<"test module test_generator case 26">
    : jh::test::tiny_test::session_definition<
          "test module test_generator case 26", jh::test::tiny_test::test<"Generator destructor cleans up after exception">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_generator case 26"> registration_26{};
}
