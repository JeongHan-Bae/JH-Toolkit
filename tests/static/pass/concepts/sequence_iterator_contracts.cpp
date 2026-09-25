#include <array>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <initializer_list>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

// STL Sequences Recognition
void sequence_contract_1()
{
    static_assert(jh::concepts::is_sequence<std::vector<int>>);
    static_assert(jh::concepts::is_sequence<std::list<double>>);
    static_assert(jh::concepts::is_sequence<std::deque<char>>);
    static_assert(jh::concepts::is_sequence<std::set<float>>);
    static_assert(jh::concepts::is_sequence<std::unordered_set<std::string>>);
    static_assert(jh::concepts::is_sequence<std::array<int, 5>>);
    static_assert(jh::concepts::is_sequence<std::forward_list<int>>);
    static_assert(jh::concepts::is_sequence<std::map<int, int>>);
    static_assert(jh::concepts::is_sequence<std::unordered_map<int, int>>);

}

// Extracting Sequence Value Types
void sequence_contract_2()
{
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::vector<int>>, int>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::array<const double, 3>>, double>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::deque<char>>, char>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::set<int>>, int>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::array<float, 10>>, float>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::map<int, double>>, std::pair<const int, double>>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::unordered_map<std::string, float>>, std::pair<const std::string, float>>);

}

// Non-Sequences Should Fail
void sequence_contract_3()
{
    static_assert(!(jh::concepts::is_sequence<int>));
    static_assert(!(jh::concepts::is_sequence<double>));
    static_assert(!(jh::concepts::is_sequence<char *>));
    static_assert(!(jh::concepts::is_sequence<std::tuple<int, double, std::string>>));
    static_assert(!(jh::concepts::is_sequence<std::optional<int>>));
    static_assert(!(jh::concepts::is_sequence<test::NoBeginEnd>));
    static_assert(!(jh::concepts::is_sequence<test::FakeSequence>));

}

// Handling Modifiers in Sequences
void sequence_contract_4()
{
    static_assert(jh::concepts::is_sequence<const std::vector<int>>);
    static_assert(jh::concepts::is_sequence<std::list<double>>);
    static_assert(jh::concepts::is_sequence<const std::deque<char>>);

    static_assert(std::is_same_v<jh::concepts::sequence_value_t<const std::vector<int>>, int>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<std::list<double>>, double>);

}

// Custom Non-Template Sequence
void sequence_contract_5()
{
    static_assert(jh::concepts::is_sequence<test::NonTemplateSequence>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<test::NonTemplateSequence>, int>);

}

// Custom Template Sequence
void sequence_contract_6()
{
    static_assert(jh::concepts::is_sequence<test::TemplateSequence<int>>);
    static_assert(jh::concepts::is_sequence<test::TemplateSequence<std::string>>);

    static_assert(std::is_same_v<jh::concepts::sequence_value_t<test::TemplateSequence<int>>, int>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<test::TemplateSequence<std::string>>, std::string>);

}

// Custom ConstIterSequence
void sequence_contract_7()
{
    static_assert(jh::concepts::is_sequence<test::ConstIterSequence>);
    static_assert(std::is_same_v<jh::concepts::sequence_value_t<test::ConstIterSequence>, int>);

}

// Mutable Iterator Sequence
void sequence_contract_8()
{
    static_assert(!(jh::concepts::is_sequence<test::MutableIterSequence>));

}

// Iterator Concept: is_iterator recognition
void sequence_contract_10()
{
    static_assert(jh::concepts::is_iterator<int *>);
    static_assert(jh::concepts::is_iterator<const double *>);
    static_assert(jh::concepts::is_iterator<std::vector<int>::iterator>);
    static_assert(jh::concepts::is_iterator<std::list<float>::iterator>);
    static_assert(jh::concepts::is_iterator<std::set<std::string>::iterator>);
    static_assert(jh::concepts::is_iterator<test::DummyInputIter < int>>);
    static_assert(!(jh::concepts::is_iterator<int>));
    static_assert(!(jh::concepts::is_iterator<test::NotIterator>));

}

// Iterator Concept: input_iterator
void sequence_contract_11()
{
    static_assert(jh::concepts::input_iterator<int *>);
    static_assert(jh::concepts::input_iterator<std::vector<int>::iterator>);
    static_assert(jh::concepts::input_iterator<test::DummyInputIter < int>>);
    static_assert(!(jh::concepts::input_iterator<int>));
    static_assert(!(jh::concepts::input_iterator<test::NotIterator>));

}

// Iterator Concept: output_iterator
void sequence_contract_12()
{
    static_assert(jh::concepts::output_iterator<int *, int>);
    static_assert(jh::concepts::output_iterator<test::DummyInputIter < int>, int >);
    static_assert(!(jh::concepts::output_iterator<const int *, int>));

}

// Iterator Concept: forward_iterator
void sequence_contract_13()
{
    static_assert(jh::concepts::forward_iterator<std::vector<int>::iterator>);
    static_assert(jh::concepts::forward_iterator<int *>);
    static_assert(!(jh::concepts::forward_iterator<int>));
    static_assert(!(jh::concepts::forward_iterator<test::NotIterator>));

}

// Iterator Concept: bidirectional_iterator
void sequence_contract_14()
{
    static_assert(jh::concepts::bidirectional_iterator<std::list<int>::iterator>);
    static_assert(jh::concepts::bidirectional_iterator<std::set<int>::iterator>);
    static_assert(jh::concepts::bidirectional_iterator<test::FakeDummyRAIter < int>>);
    static_assert(!(jh::concepts::bidirectional_iterator<test::DummyInputIter < int>>));
    static_assert(!(jh::concepts::bidirectional_iterator<int>));

}

// Iterator Concept: random_access_iterator
void sequence_contract_15()
{
    static_assert(jh::concepts::random_access_iterator<int *>);
    static_assert(jh::concepts::random_access_iterator<std::vector<int>::iterator>);
    static_assert(!(jh::concepts::random_access_iterator<test::FakeDummyRAIter < int>>));
    static_assert(!(jh::concepts::random_access_iterator<std::list<int>::iterator>));
    static_assert(!(jh::concepts::random_access_iterator<test::DummyInputIter < int>>));

}

// Iterator deduction via jh::concepts::iterator_t
void sequence_contract_16()
{
    using it_vec = jh::concepts::iterator_t<std::vector<int>>;
    using it_arr = jh::concepts::iterator_t<int[5]>;
    using it_ptr = jh::concepts::iterator_t<int *>;
    using it_set = jh::concepts::iterator_t<std::set<int>>;

    static_assert(std::is_same_v<it_vec, std::vector<int>::iterator>);
    static_assert(std::is_same_v<it_arr, int *>);
    static_assert(std::is_same_v<it_ptr, int *>);
    static_assert(std::is_same_v<it_set, std::set<int>::iterator>);

}

// Iterator deduces from array, pointer, and sequence-like
void sequence_contract_17()
{
    int arr[3] = {1, 2, 3};
    using it_arr = jh::concepts::iterator_t<decltype(arr)>;
    static_assert(std::is_same_v<it_arr, int *>);
    static_assert(jh::concepts::is_iterator<it_arr>);

    std::vector<int> v = {1, 2, 3};
    using it_vec = jh::concepts::iterator_t<decltype(v)>;
    static_assert(std::is_same_v<it_vec, std::vector<int>::iterator>);
    static_assert(jh::concepts::input_iterator<it_vec>);

}

// Iterator rejection: structurally similar but invalid
void sequence_contract_18()
{
    using namespace test;

    // BasicInputOrOutputIterator: basic iterator but can do nothing
    static_assert(jh::concepts::is_iterator<BasicInputOrOutputIterator>);
    static_assert(!(jh::concepts::input_iterator<BasicInputOrOutputIterator>));
    static_assert(!(jh::concepts::output_iterator<BasicInputOrOutputIterator, int>));

    // FakeIterNoDeref: defines value_type but not dereferenceable
    static_assert(!(jh::concepts::is_iterator<FakeIterNoDeref>));
    static_assert(!(jh::concepts::input_iterator<FakeIterNoDeref>));

    // FakeIterTypeMissmatch: invalid type mismatch on dereference
    static_assert(!(jh::concepts::is_iterator<FakeIterTypeMissmatch>));
    static_assert(!(jh::concepts::input_iterator<FakeIterTypeMissmatch>));
    static_assert(!(jh::concepts::output_iterator<FakeIterTypeMissmatch, int>));

    // FakeOutputNoInc: no ++ operators
    static_assert(!(jh::concepts::output_iterator<FakeOutputNoInc, int>));

    // FakeOutputNoAssign: ++ works but cannot assign
    static_assert(!(jh::concepts::output_iterator<FakeOutputNoAssign, int>));

}

// Sequence rejection: fake begin() types
void sequence_contract_19()
{
    static_assert(!(jh::concepts::is_sequence<test::FakeIterSequence>));

}

// iterator_t deduction coverage
void sequence_contract_20()
{
    using namespace test;

    // STL containers
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::vector<int>>, std::vector<int>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::list<double>>, std::list<double>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::deque<char>>, std::deque<char>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::set<int>>, std::set<int>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::unordered_set<std::string>>, std::unordered_set<std::string>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::map<int, int>>, std::map<int, int>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::unordered_map<std::string, int>>, std::unordered_map<std::string, int>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::forward_list<int>>, std::forward_list<int>::iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<std::array<int, 5>>, int*>); // std::array::begin returns T*

    // POD arrays and pointers
    static_assert(std::is_same_v<jh::concepts::iterator_t<int[3]>, int*>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<const int[3]>, const int*>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<int*>, int*>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<const double*>, const double*>);

    // Custom sequence-like types
    static_assert(std::is_same_v<jh::concepts::iterator_t<NonTemplateSequence>, std::vector<int>::const_iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<TemplateSequence<float>>, std::vector<float>::const_iterator>);
    static_assert(std::is_same_v<jh::concepts::iterator_t<ConstIterSequence>, std::vector<int>::const_iterator>);

    // jh::pod::array acts like array
    static_assert(std::is_same_v<jh::concepts::iterator_t<const jh::pod::array<int, 3>>, const int*>);

    // MutableIterSequence - no const begin(), but still a begin
    constexpr bool has_iter_mutable = requires { typename jh::concepts::iterator_t<test::MutableIterSequence>; };
    static_assert(has_iter_mutable);

    // NoBeginEnd and FakeSequence - must not be deducible
    static_assert(!test::can_deduce_iterator_v<test::NoBeginEnd>);
    static_assert(!test::can_deduce_iterator_v<test::FakeSequence>);

    // Builtin scalar / non-iterable types
    static_assert(!test::can_deduce_iterator_v<int>);
    static_assert(!test::can_deduce_iterator_v<void>);
    static_assert(!test::can_deduce_iterator_v<std::tuple<int, double>>);

    // Cross-check jh::concepts::is_iterator consistency with iterator_t
    static_assert(jh::concepts::is_iterator<jh::concepts::iterator_t<std::vector<int>>>);
    static_assert(jh::concepts::input_iterator<jh::concepts::iterator_t<std::list<int>>>);
    static_assert(jh::concepts::bidirectional_iterator<jh::concepts::iterator_t<std::set<int>>>);
    static_assert(jh::concepts::random_access_iterator<jh::concepts::iterator_t<std::deque<int>>>);
    static_assert(jh::concepts::input_iterator<jh::concepts::iterator_t<TemplateSequence<int>>>);
    static_assert(jh::concepts::is_iterator<jh::concepts::iterator_t<jh::pod::array<int, 3>>>);

}

// True-RA-Iterator deduction coverage
void sequence_contract_21()
{
    using namespace test;

    static_assert(jh::concepts::is_iterator<TrueRAIter<int>>);
    static_assert(jh::concepts::input_iterator<TrueRAIter<int>>);
    static_assert(jh::concepts::forward_iterator<TrueRAIter<int>>);
    static_assert(jh::concepts::bidirectional_iterator<TrueRAIter<int>>);
    static_assert(jh::concepts::random_access_iterator<TrueRAIter<int>>);

}
