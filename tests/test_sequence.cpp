#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <array>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "jh/conceptual/sequence.h"
#include "jh/pod"

namespace test {
    struct NonTemplateSequence {
        std::vector<int> data{1, 2, 3};

        [[nodiscard]] auto begin() const { return data.begin(); }

        [[nodiscard]] auto end() const { return data.end(); }
    };

    template<typename T>
    struct TemplateSequence {
        std::vector<T> data;

        TemplateSequence(std::initializer_list<T> init) : data(init) {}

        [[nodiscard]] auto begin() const { return data.begin(); }

        [[nodiscard]] auto end() const { return data.end(); }
    };

    struct ConstIterSequence {
        std::vector<int> data{4, 5, 6};

        [[nodiscard]] auto begin() const -> std::vector<int>::const_iterator { return data.begin(); }

        [[nodiscard]] auto end() const -> std::vector<int>::const_iterator { return data.end(); }
    };

    struct MutableIterSequence {
        std::vector<int> data{7, 8, 9};

        auto begin() { return data.begin(); }

        void begin() const = delete;

        auto end() { return data.end(); }

        void end() const = delete;
    };

    struct NoBeginEnd {
    }; // No `begin()` / `end()`

    struct FakeSequence { // Has `begin()` / `end()` but they are not iterators
        [[nodiscard]] static int begin() { return 42; }

        [[nodiscard]] static int end() { return 99; }
    };

    // ---------------------------------------------------------------
    // Mock iterator types for testing
    // ---------------------------------------------------------------
    template<typename T>
    struct DummyInputIter {
        using value_type = T;
        using reference = T &;
        using pointer = T *;
        using difference_type = std::ptrdiff_t;
        using iterator_category [[maybe_unused]] = std::input_iterator_tag;

        T *ptr;

        explicit DummyInputIter(T *p = nullptr) : ptr(p) {}

        reference operator*() const { return *ptr; }

        DummyInputIter &operator++() {
            ++ptr;
            return *this;
        }

        DummyInputIter operator++(int) {
            DummyInputIter tmp(*this);
            ++ptr;
            return tmp;
        }

        bool operator==(const DummyInputIter &o) const { return ptr == o.ptr; }

        bool operator!=(const DummyInputIter &o) const { return ptr != o.ptr; }
    };

    // No (ite - ite); invalid
    template<typename T>
    struct FakeDummyRAIter {

        using value_type = T;
        using reference = T &;
        using pointer = T *;
        using difference_type = std::ptrdiff_t;
        using iterator_category [[maybe_unused]] = std::random_access_iterator_tag;

        T *p;

        explicit FakeDummyRAIter(T *x = nullptr) : p(x) {}

        reference operator*() const { return *p; }

        FakeDummyRAIter &operator++() {
            ++p;
            return *this;
        }

        FakeDummyRAIter operator++(int) {
            FakeDummyRAIter tmp(*this);
            ++p;
            return tmp;
        }

        FakeDummyRAIter &operator--() {
            --p;
            return *this;
        }

        FakeDummyRAIter operator--(int) {
            FakeDummyRAIter tmp(*this);
            --p;
            return tmp;
        }

        FakeDummyRAIter operator+(std::ptrdiff_t n) const { return FakeDummyRAIter(p + n); }

        FakeDummyRAIter operator-(std::ptrdiff_t n) const { return FakeDummyRAIter(p - n); }

        reference operator[](std::ptrdiff_t n) const { return p[n]; }

        bool operator==(const FakeDummyRAIter &o) const { return p == o.p; }

        bool operator!=(const FakeDummyRAIter &o) const { return p != o.p; }

        bool operator<(const FakeDummyRAIter &o) const { return p < o.p; }

        bool operator>(const FakeDummyRAIter &o) const { return p > o.p; }

        bool operator<=(const FakeDummyRAIter &o) const { return p <= o.p; }

        bool operator>=(const FakeDummyRAIter &o) const { return p >= o.p; }
    };

    struct NotIterator {
    }; // intentionally invalid

    // ============================================================================
    // Iterator Edge Case / Misleading Types (should be rejected)
    // ============================================================================

    // Basic Iterator
    struct BasicInputOrOutputIterator {
        BasicInputOrOutputIterator& operator++() { return *this; }
        BasicInputOrOutputIterator operator++(int) { return *this; }
        void operator*() const {}
        bool operator==(const BasicInputOrOutputIterator&) const = default;
    };

    // Fake iterator: defines value_type but no dereference
    struct FakeIterNoDeref {
        using value_type = int;
        FakeIterNoDeref& operator++() { return *this; }
        FakeIterNoDeref operator++(int) { return *this; }
    };

    // Fake iterator: dereference returns unrelated type
    struct FakeIterTypeMissmatch {
        using value_type = int;
        using reference = std::string;
        int operator++() { return 0; } // not returning iterator reference
        std::string operator*() const { return "bad"; } // incompatible with value_type
    };

    // Fake output iterator: can assign but not increment
    struct FakeOutputNoInc {
        using value_type = int;
        int storage{};
        int& operator*() { return storage; }
        void operator=(int v) { storage = v; }
        // missing operator++
    };

    // Fake output iterator: increment works, but assignment fails
    struct FakeOutputNoAssign {
        FakeOutputNoAssign& operator++() { return *this; }
        FakeOutputNoAssign operator++(int) { return *this; }
        int operator*() const { return 42; } // not assignable
    };

    // Fake sequence: has begin()/end(), but begin() returns invalid iterator
    struct FakeIterSequence {
        static int begin() { return 1; }
        static int end() { return 2; }
    };

    template <typename T, typename = void>
    constexpr bool can_deduce_iterator_v = false;

    template <typename T>
    constexpr bool can_deduce_iterator_v<T, std::void_t<typename jh::concepts::detail::iterator_resolver<T>::type>> = true;

    // ============================================================================
    // True Random Access Iterator, Not STL standard
    // ============================================================================

    template<typename T>
    struct TrueRAIter {


        T* p = nullptr;

        TrueRAIter() = default;
        explicit TrueRAIter(T* ptr) : p(ptr) {}

        // Dereference
        T& operator*() const noexcept { return *p; }
        T* operator->() const noexcept { return p; }

        // Pre/post increment
        TrueRAIter& operator++() noexcept {
            ++p;
            return *this;
        }
        TrueRAIter operator++(int) noexcept {
            TrueRAIter tmp(*this);
            ++p;
            return tmp;
        }

        // Pre/post decrement
        TrueRAIter& operator--() noexcept {
            --p;
            return *this;
        }
        TrueRAIter operator--(int) noexcept {
            TrueRAIter tmp(*this);
            --p;
            return tmp;
        }

        // Arithmetic
        TrueRAIter& operator+=(std::ptrdiff_t n) noexcept {
            p += n;
            return *this;
        }
        TrueRAIter& operator-=(std::ptrdiff_t n) noexcept {
            p -= n;
            return *this;
        }
        TrueRAIter operator+(std::ptrdiff_t n) const noexcept { return TrueRAIter(p + n); }
        friend TrueRAIter operator+(std::ptrdiff_t n, const TrueRAIter& it) noexcept { return TrueRAIter(it.p + n); }
        TrueRAIter operator-(std::ptrdiff_t n) const noexcept { return TrueRAIter(p - n); }

        std::ptrdiff_t operator-(const TrueRAIter& other) const noexcept { return p - other.p; }

        // Element access
        T& operator[](std::ptrdiff_t n) const noexcept { return p[n]; }

        // Comparison
        bool operator==(const TrueRAIter& other) const noexcept { return p == other.p; }
        bool operator!=(const TrueRAIter& other) const noexcept { return p != other.p; }
        bool operator<(const TrueRAIter& other) const noexcept { return p < other.p; }
        bool operator>(const TrueRAIter& other) const noexcept { return p > other.p; }
        bool operator<=(const TrueRAIter& other) const noexcept { return p <= other.p; }
        bool operator>=(const TrueRAIter& other) const noexcept { return p >= other.p; }
    };
} // namespace test

// Recognizing STL Sequences
TEST_CASE("STL Sequences Recognition") {
    REQUIRE(jh::concepts::is_sequence<std::vector<int>>);
    REQUIRE(jh::concepts::is_sequence<std::list<double>>);
    REQUIRE(jh::concepts::is_sequence<std::deque<char>>);
    REQUIRE(jh::concepts::is_sequence<std::set<float>>);
    REQUIRE(jh::concepts::is_sequence<std::unordered_set<std::string>>);
    REQUIRE(jh::concepts::is_sequence<std::array<int, 5>>);
    REQUIRE(jh::concepts::is_sequence<std::forward_list<int>>);
    REQUIRE(jh::concepts::is_sequence<std::map<int, int>>);
    REQUIRE(jh::concepts::is_sequence<std::unordered_map<int, int>>);
}

// Extracting Sequence Value Types
TEST_CASE("Extracting Sequence Value Types") {
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::vector<int>>, int>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::array<const double, 3>>, double>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::deque<char>>, char>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::set<int>>, int>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::array<float, 10>>, float>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::map<int, double>>, std::pair<const int, double>>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::unordered_map<std::string, float>>, std::pair<const std::string, float>>);
}

TEST_CASE("Non-Sequences Should Fail") {
    REQUIRE_FALSE(jh::concepts::is_sequence<int>);
    REQUIRE_FALSE(jh::concepts::is_sequence<double>);
    REQUIRE_FALSE(jh::concepts::is_sequence<char *>);
    REQUIRE_FALSE(jh::concepts::is_sequence<std::tuple<int, double, std::string>>);
    REQUIRE_FALSE(jh::concepts::is_sequence<std::optional<int>>);
    REQUIRE_FALSE(jh::concepts::is_sequence<test::NoBeginEnd>);
    REQUIRE_FALSE(jh::concepts::is_sequence<test::FakeSequence>);
}

// Handling `const` and `mutable`
TEST_CASE("Handling Modifiers in Sequences") {
    REQUIRE(jh::concepts::is_sequence<const std::vector<int>>);
    REQUIRE(jh::concepts::is_sequence<std::list<double>>);
    REQUIRE(jh::concepts::is_sequence<const std::deque<char>>);

    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<const std::vector<int>>, int>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<std::list<double>>, double>);
}

// Custom Sequence Recognition
TEST_CASE("Custom Non-Template Sequence") {
    REQUIRE(jh::concepts::is_sequence<test::NonTemplateSequence>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<test::NonTemplateSequence>, int>);
}

// Custom Template Sequence Recognition
TEST_CASE("Custom Template Sequence") {
    REQUIRE(jh::concepts::is_sequence<test::TemplateSequence<int>>);
    REQUIRE(jh::concepts::is_sequence<test::TemplateSequence<std::string>>);

    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<test::TemplateSequence<int>>, int>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<test::TemplateSequence<std::string>>, std::string>);
}

// Custom Sequence with `const begin()`
TEST_CASE("Custom ConstIterSequence") {
    REQUIRE(jh::concepts::is_sequence<test::ConstIterSequence>);
    REQUIRE(std::is_same_v<jh::concepts::sequence_value_t<test::ConstIterSequence>, int>);
}

// Custom Sequence with Mutable Iterators (Should NOT be a sequence)
TEST_CASE("Mutable Iterator Sequence") {
    REQUIRE_FALSE(jh::concepts::is_sequence<test::MutableIterSequence>);
}


TEST_CASE("Sequence to Range") {
    constexpr jh::pod::array<int, 3> vec = {1, 2, 3}; // sequence, not a range
    auto range_ = jh::to_range(vec);
    auto it = range_.begin();

    std::ranges::for_each(range_, [&](const int a) {
        REQUIRE(a == *it);
        ++it;
    });
}

TEST_CASE("Iterator Concept: is_iterator recognition") {
    REQUIRE(jh::concepts::is_iterator<int *>);
    REQUIRE(jh::concepts::is_iterator<const double *>);
    REQUIRE(jh::concepts::is_iterator<std::vector<int>::iterator>);
    REQUIRE(jh::concepts::is_iterator<std::list<float>::iterator>);
    REQUIRE(jh::concepts::is_iterator<std::set<std::string>::iterator>);
    REQUIRE(jh::concepts::is_iterator<test::DummyInputIter < int>>);
    REQUIRE_FALSE(jh::concepts::is_iterator<int>);
    REQUIRE_FALSE(jh::concepts::is_iterator<test::NotIterator>);
}

TEST_CASE("Iterator Concept: input_iterator") {
    REQUIRE(jh::concepts::input_iterator<int *>);
    REQUIRE(jh::concepts::input_iterator<std::vector<int>::iterator>);
    REQUIRE(jh::concepts::input_iterator<test::DummyInputIter < int>>);
    REQUIRE_FALSE(jh::concepts::input_iterator<int>);
    REQUIRE_FALSE(jh::concepts::input_iterator<test::NotIterator>);
}

TEST_CASE("Iterator Concept: output_iterator") {
    REQUIRE(jh::concepts::output_iterator<int *, int>);
    REQUIRE(jh::concepts::output_iterator<test::DummyInputIter < int>, int >);
    REQUIRE_FALSE(jh::concepts::output_iterator<const int *, int>);
}

TEST_CASE("Iterator Concept: forward_iterator") {
    REQUIRE(jh::concepts::forward_iterator<std::vector<int>::iterator>);
    REQUIRE(jh::concepts::forward_iterator<int *>);
    REQUIRE_FALSE(jh::concepts::forward_iterator<int>);
    REQUIRE_FALSE(jh::concepts::forward_iterator<test::NotIterator>);
}

TEST_CASE("Iterator Concept: bidirectional_iterator") {
    REQUIRE(jh::concepts::bidirectional_iterator<std::list<int>::iterator>);
    REQUIRE(jh::concepts::bidirectional_iterator<std::set<int>::iterator>);
    REQUIRE(jh::concepts::bidirectional_iterator<test::FakeDummyRAIter < int>>);
    REQUIRE_FALSE(jh::concepts::bidirectional_iterator<test::DummyInputIter < int>>);
    REQUIRE_FALSE(jh::concepts::bidirectional_iterator<int>);
}

TEST_CASE("Iterator Concept: random_access_iterator") {
    REQUIRE(jh::concepts::random_access_iterator<int *>);
    REQUIRE(jh::concepts::random_access_iterator<std::vector<int>::iterator>);
    REQUIRE_FALSE(jh::concepts::random_access_iterator<test::FakeDummyRAIter < int>>);
    REQUIRE_FALSE(jh::concepts::random_access_iterator<std::list<int>::iterator>);
    REQUIRE_FALSE(jh::concepts::random_access_iterator<test::DummyInputIter < int>>);
}

TEST_CASE("Iterator deduction via jh::concepts::iterator_t") {
    using it_vec = jh::concepts::iterator_t<std::vector<int>>;
    using it_arr = jh::concepts::iterator_t<int[5]>;
    using it_ptr = jh::concepts::iterator_t<int *>;
    using it_set = jh::concepts::iterator_t<std::set<int>>;

    STATIC_REQUIRE(std::is_same_v<it_vec, std::vector<int>::iterator>);
    STATIC_REQUIRE(std::is_same_v<it_arr, int *>);
    STATIC_REQUIRE(std::is_same_v<it_ptr, int *>);
    STATIC_REQUIRE(std::is_same_v<it_set, std::set<int>::iterator>);
}

TEST_CASE("Iterator deduces from array, pointer, and sequence-like") {
    int arr[3] = {1, 2, 3};
    using it_arr = jh::concepts::iterator_t<decltype(arr)>;
    STATIC_REQUIRE(std::is_same_v<it_arr, int *>);
    STATIC_REQUIRE(jh::concepts::is_iterator<it_arr>);

    std::vector<int> v = {1, 2, 3};
    using it_vec = jh::concepts::iterator_t<decltype(v)>;
    STATIC_REQUIRE(std::is_same_v<it_vec, std::vector<int>::iterator>);
    STATIC_REQUIRE(jh::concepts::input_iterator<it_vec>);
}

// ---------------------------------------------------------------------------
// Concept rejection tests
// ---------------------------------------------------------------------------

TEST_CASE("Iterator rejection: structurally similar but invalid") {
    using namespace test;

    // BasicInputOrOutputIterator: basic iterator but can do nothing
    REQUIRE(jh::concepts::is_iterator<BasicInputOrOutputIterator>);
    REQUIRE_FALSE(jh::concepts::input_iterator<BasicInputOrOutputIterator>);
    REQUIRE_FALSE(jh::concepts::output_iterator<BasicInputOrOutputIterator, int>);

    // FakeIterNoDeref: defines value_type but not dereferenceable
    REQUIRE_FALSE(jh::concepts::is_iterator<FakeIterNoDeref>);
    REQUIRE_FALSE(jh::concepts::input_iterator<FakeIterNoDeref>);

    // FakeIterTypeMissmatch: invalid type mismatch on dereference
    REQUIRE_FALSE(jh::concepts::is_iterator<FakeIterTypeMissmatch>);
    REQUIRE_FALSE(jh::concepts::input_iterator<FakeIterTypeMissmatch>);
    REQUIRE_FALSE(jh::concepts::output_iterator<FakeIterTypeMissmatch, int>);

    // FakeOutputNoInc: no ++ operators
    REQUIRE_FALSE(jh::concepts::output_iterator<FakeOutputNoInc, int>);

    // FakeOutputNoAssign: ++ works but cannot assign
    REQUIRE_FALSE(jh::concepts::output_iterator<FakeOutputNoAssign, int>);
}

TEST_CASE("Sequence rejection: fake begin() types") {
    REQUIRE_FALSE(jh::concepts::is_sequence<test::FakeIterSequence>);
}

// ============================================================================
// iterator_t deduction coverage and error safety
// ============================================================================

TEST_CASE("iterator_t deduction coverage") {
    using namespace test;

    // STL containers
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::vector<int>>, std::vector<int>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::list<double>>, std::list<double>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::deque<char>>, std::deque<char>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::set<int>>, std::set<int>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::unordered_set<std::string>>, std::unordered_set<std::string>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::map<int, int>>, std::map<int, int>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::unordered_map<std::string, int>>, std::unordered_map<std::string, int>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::forward_list<int>>, std::forward_list<int>::iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<std::array<int, 5>>, int*>); // std::array::begin returns T*

    // POD arrays and pointers
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<int[3]>, int*>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<const int[3]>, const int*>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<int*>, int*>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<const double*>, const double*>);

    // Custom sequence-like types
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<NonTemplateSequence>, std::vector<int>::const_iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<TemplateSequence<float>>, std::vector<float>::const_iterator>);
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<ConstIterSequence>, std::vector<int>::const_iterator>);

    // jh::pod::array acts like array
    STATIC_REQUIRE(std::is_same_v<jh::concepts::iterator_t<const jh::pod::array<int, 3>>, const int*>);

    // MutableIterSequence - no const begin(), but still a begin
    constexpr bool has_iter_mutable = requires { typename jh::concepts::iterator_t<test::MutableIterSequence>; };
    STATIC_REQUIRE(has_iter_mutable);

    // NoBeginEnd and FakeSequence - must not be deducible
    STATIC_REQUIRE(!test::can_deduce_iterator_v<test::NoBeginEnd>);
    STATIC_REQUIRE(!test::can_deduce_iterator_v<test::FakeSequence>);

    // Builtin scalar / non-iterable types
    STATIC_REQUIRE(!test::can_deduce_iterator_v<int>);
    STATIC_REQUIRE(!test::can_deduce_iterator_v<void>);
    STATIC_REQUIRE(!test::can_deduce_iterator_v<std::tuple<int, double>>);

    // Cross-check jh::concepts::is_iterator consistency with iterator_t
    STATIC_REQUIRE(jh::concepts::is_iterator<jh::concepts::iterator_t<std::vector<int>>>);
    STATIC_REQUIRE(jh::concepts::input_iterator<jh::concepts::iterator_t<std::list<int>>>);
    STATIC_REQUIRE(jh::concepts::bidirectional_iterator<jh::concepts::iterator_t<std::set<int>>>);
    STATIC_REQUIRE(jh::concepts::random_access_iterator<jh::concepts::iterator_t<std::deque<int>>>);
    STATIC_REQUIRE(jh::concepts::input_iterator<jh::concepts::iterator_t<TemplateSequence<int>>>);
    STATIC_REQUIRE(jh::concepts::is_iterator<jh::concepts::iterator_t<jh::pod::array<int, 3>>>);
}

TEST_CASE("True-RA-Iterator deduction coverage") {
    using namespace test;

    STATIC_REQUIRE(jh::concepts::is_iterator<TrueRAIter<int>>);
    STATIC_REQUIRE(jh::concepts::input_iterator<TrueRAIter<int>>);
    STATIC_REQUIRE(jh::concepts::forward_iterator<TrueRAIter<int>>);
    STATIC_REQUIRE(jh::concepts::bidirectional_iterator<TrueRAIter<int>>);
    STATIC_REQUIRE(jh::concepts::random_access_iterator<TrueRAIter<int>>);
}
