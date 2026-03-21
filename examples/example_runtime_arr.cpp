/**
 * @file example_runtime_arr.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/runtime_arr&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use <code>&lt;jh/runtime_arr&gt;</code>
 * and serves as a reference example for both developers and AI systems
 * learning how to use this library.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/runtime_arr&gt;
 * @endcode
 *
 * <p>
 * All symbols used in this example are exported under the
 * <code>jh::</code> namespace.
 * </p>
 *
 * <p>
 * For complete API documentation, refer to
 * <code>jh/core/runtime_arr.h</code>. The Doxygen comments in that
 * header define the official semantics of the component.
 * </p>
 *
 *
 * <h3>Historical Background and Practical Motivation</h3>
 *
 * <p>
 * The design of <code>jh::runtime_arr</code> originated from
 * real-world attempts to migrate Python-based numerical and
 * data-processing systems into modern C++20.
 * </p>
 *
 * <p>
 * In Python (for example, using NumPy), arrays are typically:
 * </p>
 *
 * <ul>
 *   <li>Allocated with a fixed shape at initialization time.</li>
 *   <li>Contiguously stored.</li>
 *   <li>Non-resizable in ordinary numerical workflows.</li>
 * </ul>
 *
 * <p>
 * Conceptually:
 * </p>
 *
 * @code{.py}
 * data = np.zeros((N,))
 * @endcode
 *
 * <p>
 * The shape is fixed; the container does not grow.
 * </p>
 *
 *
 * <h3>Why Not std::vector?</h3>
 *
 * <p>
 * Although <code>std::vector&lt;T&gt;</code> can be used with a fixed
 * size, it is semantically a resizable container.
 * </p>
 *
 * <p>
 * Even if user code never calls <code>push_back()</code> or
 * <code>resize()</code>, the type system does not encode the
 * non-resizable invariant.
 * </p>
 *
 * <p>
 * In complex industrial systems:
 * </p>
 *
 * <ul>
 *   <li>Call chains are deep.</li>
 *   <li>Business logic is layered.</li>
 *   <li>Containers cross translation-unit boundaries.</li>
 * </ul>
 *
 * <p>
 * Compilers must conservatively assume that a
 * <code>std::vector</code> may change size.
 * </p>
 *
 * <p>
 * Advanced compilers (e.g., modern LLVM/Clang) can sometimes
 * infer non-growth properties through interprocedural analysis.
 * However:
 * </p>
 *
 * <ul>
 *   <li>This increases analysis complexity.</li>
 *   <li>Optimization success depends on inlining depth.</li>
 *   <li>Less aggressive toolchains may fail to derive the invariant.</li>
 * </ul>
 *
 * <p>
 * What was needed was a semantic equivalent of:
 * </p>
 *
 * <ul>
 *   <li>A runtime-sized array.</li>
 *   <li>Heap-allocated.</li>
 *   <li>Never resizable.</li>
 *   <li>Deterministic lifetime.</li>
 * </ul>
 *
 *
 * <h3>Why Not Just std::unique_ptr&lt;T[]&gt;?</h3>
 *
 * <p>
 * The traditional pattern:
 * </p>
 *
 * @code
 * std::unique_ptr&lt;T[]&gt;
 * @endcode
 *
 * <p>
 * provides:
 * </p>
 *
 * <ul>
 *   <li>Heap allocation.</li>
 *   <li>Clear ownership.</li>
 *   <li>No resizing.</li>
 * </ul>
 *
 * <p>
 * But it is not a container:
 * </p>
 *
 * <ul>
 *   <li>No <code>begin()</code>/<code>end()</code>.</li>
 *   <li>No range compatibility.</li>
 *   <li>No allocator policy surface.</li>
 *   <li>No container traits.</li>
 * </ul>
 *
 * <p>
 * <code>jh::runtime_arr</code> encapsulates this model and
 * lifts it to a first-class container abstraction.
 * </p>
 *
 *
 * <h3>Allocator Model</h3>
 *
 * <p>
 * Unlike most containers in the JH Toolkit, the default allocator
 * of <code>runtime_arr</code> is not <code>std::allocator</code>.
 * </p>
 *
 * <p>
 * The default allocator type is:
 * </p>
 *
 * @code
 * jh::typed::monostate
 * // A "monostate" placeholder representing no allocator, logic defined by runtime_arr itself
 * @endcode
 *
 * <p>
 * Under <code>jh::typed::monostate</code>, allocation does not
 * follow the behavior of <code>std::allocator</code>.
 * </p>
 *
 * <p>
 * Instead, a specialized allocation path is used that avoids
 * allocator_traits indirection and simplifies ownership semantics.
 * </p>
 *
 * <p>
 * This design choice reduces template depth and clarifies
 * compiler analysis paths.
 * </p>
 *
 *
 * <h3>POD Acceleration Philosophy</h3>
 *
 * <p>
 * For POD-like types:
 * </p>
 *
 * <ul>
 *   <li>Construction may skip per-element initialization.</li>
 *   <li>Reset operations reduce to <code>std::memset</code>.</li>
 *   <li>No destructor loops are required.</li>
 * </ul>
 *
 * <p>
 * This is achieved using SFINAE and C++20 concepts to prune
 * unnecessary behavior at compile time.
 * </p>
 *
 *
 * <h3>Dual-Mode Build Model</h3>
 *
 * <p>
 * The component supports both:
 * </p>
 *
 * <ul>
 *   <li><code>jh::jh-toolkit</code> — header-only mode.</li>
 *   <li><code>jh::jh-toolkit-static</code> — static-optimized mode.</li>
 * </ul>
 *
 *
 * <h3>Special Case: runtime_arr&lt;bool&gt;</h3>
 *
 * <p>
 * The <code>bool</code> specialization behaves differently
 * from the general <code>runtime_arr&lt;T&gt;</code>.
 * </p>
 *
 * <p>
 * The bit-packed variant:
 * </p>
 *
 * <ul>
 *   <li>Stores elements as packed bits (<code>uint64_t[]</code> backing).</li>
 *   <li>Does not expose <code>data()</code>.</li>
 *   <li>Does not provide <code>std::span</code> access.</li>
 *   <li>Does not model a standard contiguous range.</li>
 *   <li>Does not expose raw pointer iterators.</li>
 * </ul>
 *
 * <p>
 * Access is performed through index-based bit manipulation.
 * Operations such as <code>set()</code> and <code>test()</code>
 * are provided.
 * </p>
 *
 * <p>
 * This behavior is closer to <code>std::vector&lt;bool&gt;</code>
 * or <code>std::bitset</code> than to a normal contiguous container.
 * </p>
 *
 * <p>
 * Because bit manipulation introduces additional instructions,
 * the static build target precompiles this specialization
 * with aggressive optimization flags (e.g., -O3) to mitigate
 * the performance cost.
 * </p>
 *
 * <p>
 * A conjugate variant:
 * </p>
 *
 * @code
 * jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;
 * @endcode
 *
 * <p>
 * uses byte-based storage and behaves like a normal
 * contiguous container.
 * </p>
 *
 *
 * <h3>Design Objective</h3>
 *
 * <p>
 * The overarching objective of <code>runtime_arr</code> is to:
 * </p>
 *
 * <ul>
 *   <li>Encode non-resizable invariants at the type level.</li>
 *   <li>Reduce alias-analysis uncertainty.</li>
 *   <li>Clarify compiler optimization paths.</li>
 *   <li>Provide deterministic heap-based fixed-size storage.</li>
 * </ul>
 *
 * <p>
 * It represents a semantic category distinct from
 * <code>std::vector</code>, modeling a runtime-sized but
 * structurally immutable container.
 * </p>
 *
 */

#include <jh/runtime_arr>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

/**
 * @page Runtime-Sized Immutable Matrix Backed by jh::runtime_arr
 *
 * <h3>Overview</h3>
 *
 * This module demonstrates how to construct a runtime-sized,
 * non-resizable matrix abstraction using <code>jh::runtime_arr</code>.
 *
 * The objective is to reproduce the common NumPy-style usage pattern:
 *
 * @code{.py}
 * import numpy as np
 * A = np.zeros((rows, cols))
 * @endcode
 *
 * where:
 *
 * <ul>
 *   <li>The shape is fixed at construction time.</li>
 *   <li>The storage is contiguous.</li>
 *   <li>The container does not grow.</li>
 * </ul>
 *
 * <h3>Design Principles</h3>
 *
 * <h4>Shape Immutability</h4>
 *
 * Unlike <code>std::vector</code>, the matrix backbone
 * (<code>jh::runtime_arr</code>) does not support resizing.
 *
 * The non-growth invariant is encoded at the type level.
 *
 *
 * <h4>Deterministic Contiguous Layout</h4>
 *
 * The matrix uses row-major storage:
 *
 * @code
 * index = r * cols + c
 * @endcode
 *
 * This mirrors NumPy's default C-order layout.
 *
 *
 * <h4>POD Acceleration</h4>
 *
 * For arithmetic types, construction and destruction
 * are simplified, allowing efficient bulk operations.
 *
 *
 * <h3>Conceptual Structure</h3>
 *
 * @code
 * template<typename T>
 * class matrix {
 *     const std::size_t rows_;
 *     const std::size_t cols_;
 *     jh::runtime_arr<T> data_;
 * };
 * @endcode
 *
 *
 * <h3>Industrial Context</h3>
 *
 * This simplified pattern originated from Python-to-C++ migration efforts
 * where fixed-shape numerical buffers were required for:
 *
 * <ul>
 *   <li>Image processing pipelines</li>
 *   <li>Scientific computation</li>
 *   <li>GPU upload staging buffers</li>
 * </ul>
 *
 * The invariant that "this buffer will never grow"
 * simplifies compiler optimization and alias analysis.
 *
 * Here we demonstrate usage examples for <code>runtime_arr</code>.
 * Since it prohibits copying, you must perform a deep copy as follows:
 * <ul>
 *     <li>either construct with (<code>begin()</code>,<code>end()</code>)
 *     iterators from an existing container,</li>
 *     <li>or use <code>jh::runtime_arr&lt;T&gt;b(a.size());</code> combined with
 *         explicit copying via <code>std::copy</code>.</li>
 * </ul>
 *
 */

#include <iostream>
#include <iomanip>
#include <concepts>
#include <jh/pod>

namespace example::simulated {

    /* =========================================================
       Concept Constraint
       ========================================================= */

    template<typename T>
    concept MatrixElement =
    jh::pod::pod_like<T> && requires(T a, T b) {
        { a + b } -> std::same_as<T>;
        { a * b } -> std::same_as<T>;
        { a += b } -> std::same_as<T &>;
    };


    /* =========================================================
       Matrix
       ========================================================= */

    template<MatrixElement T>
    class Matrix {
    public:

        /* ================= Constructors ================= */

        Matrix(std::size_t rows, std::size_t cols)
                : rows_(rows),
                  cols_(cols),
                  data_(rows * cols) {
        }

        Matrix(std::initializer_list<std::initializer_list<T>> init)
                : rows_(init.size()),
                  cols_(init.size() ? init.begin()->size() : 0),
                  data_(rows_ * cols_) {
            std::size_t r = 0;

            for (const auto &row: init) {

                if (row.size() != cols_)
                    throw std::runtime_error(
                            "Matrix initializer rows must have equal size");

                std::size_t c = 0;
                for (const auto &value: row) {
                    data_[r * cols_ + c] = value;
                    ++c;
                }

                ++r;
            }
        }

        /* ================= Copy ================= */

        Matrix(const Matrix &other)
                : rows_(other.rows_),
                  cols_(other.cols_),
                  data_(other.data_.begin(), other.data_.end()) {}

        Matrix &operator=(const Matrix &other) {
            if (this == &other)
                return *this;

            if (rows_ != other.rows_ || cols_ != other.cols_)
                throw std::invalid_argument(
                        "Matrix copy assignment requires identical dimensions");

            std::copy(other.raw(),
                      other.raw() + other.size(),
                      raw());

            return *this;
        }

        /* ================= Move ================= */

        Matrix(Matrix &&) noexcept = default;

        Matrix &operator=(Matrix &&) noexcept = default;

        ~Matrix() = default;


        /* ================= Observers ================= */

        [[nodiscard]] std::size_t rows() const noexcept { return rows_; }

        [[nodiscard]] std::size_t cols() const noexcept { return cols_; }

        [[nodiscard]] std::size_t size() const noexcept { return rows_ * cols_; }

        [[nodiscard]] T *raw() noexcept { return data_.data(); }

        [[nodiscard]] const T *raw() const noexcept { return data_.data(); }


        /* ================= Element Access ================= */

        T &operator()(std::size_t r, std::size_t c) {
            return data_[r * cols_ + c];
        }

        const T &operator()(std::size_t r, std::size_t c) const {
            return data_[r * cols_ + c];
        }


        /* ================= Addition ================= */

        [[nodiscard]] Matrix operator+(const Matrix &other) const {
            if (rows_ != other.rows_ || cols_ != other.cols_)
                throw std::invalid_argument(
                        "Matrix dimensions must match for addition");

            Matrix result(rows_, cols_);

            std::transform(raw(),
                           raw() + size(),
                           other.raw(),
                           result.raw(),
                           [](T a, T b) { return a + b; });

            return result;
        }

        Matrix &operator+=(const Matrix &other) {
            if (rows_ != other.rows_ || cols_ != other.cols_)
                throw std::invalid_argument(
                        "Matrix dimensions must match for addition");

            std::transform(raw(),
                           raw() + size(),
                           other.raw(),
                           raw(),
                           [](T a, T b) { return a + b; });

            return *this;
        }


        /* ================= Multiplication ================= */

        Matrix operator*(const Matrix &other) const {
            if (cols_ != other.rows_)
                throw std::invalid_argument(
                        "Matrix dimensions incompatible for multiplication");

            Matrix result(rows_, other.cols_);

            for (std::size_t i = 0; i < rows_; ++i)
                for (std::size_t k = 0; k < cols_; ++k)
                    for (std::size_t j = 0; j < other.cols_; ++j)
                        result(i, j) +=
                                (*this)(i, k) * other(k, j);

            return result;
        }

        /* ================= Fast Power ================= */

        [[nodiscard]] Matrix pow(std::size_t exponent) const {
            if (rows_ != cols_)
                throw std::invalid_argument(
                        "Matrix exponentiation requires square matrix");

            Matrix result = identity(rows_);
            Matrix base(*this);  // deep copy

            while (exponent > 0) {

                if (exponent & 1)
                    result = result * base;

                base = base * base;
                exponent >>= 1;
            }

            return result;
        }


        /* ================= Identity ================= */

        static Matrix identity(std::size_t n) {
            Matrix I(n, n);

            for (std::size_t i = 0; i < n; ++i)
                I(i, i) = static_cast<T>(1);

            return I;
        }

    private:
        const std::size_t rows_{};
        const std::size_t cols_{};
        jh::runtime_arr<T> data_{};
    };

} // namespace example::simulated

namespace example {

    void example_matrix_runtime_arr() {

        using simulated::Matrix;

        std::cout << "\n===== Runtime-sized immutable matrix demo =====\n\n";

        /* =========================
           1. Matrix Multiplication
           ========================= */

        Matrix<double> A{
                {1, 2, 3},
                {4, 5, 6}
        };

        Matrix<double> B{
                {7,  8},
                {9,  10},
                {11, 12}
        };

        auto C = A * B;

        std::cout << "A (2x3) * B (3x2) = C (2x2)\n\n";

        for (std::size_t r = 0; r < C.rows(); ++r) {
            for (std::size_t c = 0; c < C.cols(); ++c)
                std::cout << std::setw(8) << C(r, c);
            std::cout << "\n";
        }

        /* =========================
           2. Matrix Addition
           ========================= */

        Matrix<double> D{
                {1, 1},
                {1, 1}
        };

        auto E = C + D;

        std::cout << "\nC + D = E\n\n";

        for (std::size_t r = 0; r < E.rows(); ++r) {
            for (std::size_t c = 0; c < E.cols(); ++c)
                std::cout << std::setw(8) << E(r, c);
            std::cout << "\n";
        }

        /* =========================
           3. Square Matrix Fast Power
           ========================= */

        Matrix<unsigned long long> F{
                {1, 1},
                {1, 0}
        };

        std::size_t exponent = 20;

        auto G = F.pow(exponent);

        std::cout << "\nFibonacci matrix ^ " << exponent << "\n\n";

        for (std::size_t r = 0; r < G.rows(); ++r) {
            for (std::size_t c = 0; c < G.cols(); ++c)
                std::cout << std::setw(8) << G(r, c);
            std::cout << "\n";
        }

        std::cout << "\nF(" << exponent << ") = "
                  << G(0, 1) << "\n";

        std::cout << "\n===== end matrix demo =====\n";
    }

}

/**
 * @page runtime_arr_basic_usage Recommended API Usage Patterns
 *
 * <h3>Overview</h3>
 *
 * The following examples demonstrate common and recommended ways to use
 * <code>jh::runtime_arr</code> directly as a container.
 *
 * These examples focus on practical usage patterns that mirror
 * <code>std::vector</code> initialization semantics while respecting
 * the structural immutability of <code>runtime_arr</code>.
 *
 * <h3>Key Differences from std::vector</h3>
 *
 * <ul>
 *   <li>The size is fixed at construction.</li>
 *   <li>Copying is intentionally disabled.</li>
 *   <li>Moving is supported.</li>
 *   <li>Elements remain mutable.</li>
 * </ul>
 *
 * Initialization syntax is intentionally designed to feel familiar
 * to users of <code>std::vector</code>.
 *
 */

#include <numeric>

namespace example {

    void example_runtime_arr_basic_usage() {
        std::cout << "\n===== runtime_arr basic usage =====\n\n";

        /* =====================================================
           1. Size-based construction
           ===================================================== */

        jh::runtime_arr<int> a(5);

        std::cout << "size-based construction:\n";

        for (std::size_t i = 0; i < a.size(); ++i)
            a[i] = static_cast<int>(i * 10);

        for (auto v: a)
            std::cout << v << " ";

        std::cout << "\n\n";


        /* =====================================================
           2. Initializer-list construction
           (same syntax as std::vector)
           ===================================================== */

        jh::runtime_arr<int> b{1, 2, 3, 4, 5};

        std::cout << "initializer list construction:\n";

        for (auto v: b)
            std::cout << v << " ";

        std::cout << "\n\n";


        /* =====================================================
           3. Single-element initialization
           ===================================================== */

        jh::runtime_arr<int> single{42};

        std::cout << "single element:\n";
        std::cout << single[0] << "\n\n";


        /* =====================================================
           4. Construction from std::vector (move)
           ===================================================== */

        std::vector<int> vec{10, 20, 30, 40};

        jh::runtime_arr<int> c(std::move(vec));

        std::cout << "constructed from std::vector (move):\n";

        for (auto v: c)
            std::cout << v << " ";

        std::cout << "\n\n";


        /* =====================================================
           5. Move conversion back to std::vector
           ===================================================== */
        /**
         * <h4>Move conversion to <code>std::vector</code></h4>
         *
         * Because the conversion operator is declared as:
         *
         * @code
         * operator std::vector<T>() &&
         * @endcode
         *
         * the conversion is only available for rvalues. This enforces
         * explicit ownership transfer and prevents accidental copying.
         *
         * <b>Correct usage:</b>
         *
         * @code
         * jh::runtime_arr&lt;int&gt; c1{1,2,3,4,5};
         * jh::runtime_arr&lt;int&gt; c2{1,2,3,4,5};
         *
         * std::vector&lt;int&gt; vec2{std::move(c1)};
         *
         * auto vec3 = std::vector&lt;int&gt;{std::move(c2)};
         * @endcode
         *
         * <b>Incorrect usage (will not compile as intended):</b>
         *
         * @code
         * jh::runtime_arr&lt;int&gt; c{1,2,3,4,5};
         *
         * std::vector&lt;int&gt; vec = std::move(c);
         * // ERROR: copy-initialization does not use this conversion as intended
         * @endcode
         *
         * In practice, prefer direct construction:
         *
         * @code
         * std::vector&lt;int&gt; vec{std::move(c)};
         * @endcode
         */
        std::vector<int> vec2{std::move(c)};

        std::cout << "moved back into std::vector:\n";

        for (auto v: vec2)
            std::cout << v << " ";

        std::cout << "\n\n";


        /* =====================================================
           6. Explicit deep copy pattern
           ===================================================== */

        jh::runtime_arr<int> src{1, 2, 3, 4, 5};

        jh::runtime_arr<int> dst(src.size());

        std::copy(src.begin(), src.end(), dst.begin());

        std::cout << "manual deep copy:\n";

        for (auto v: dst)
            std::cout << v << " ";

        std::cout << "\n\n";


        /* =====================================================
           7. Range algorithm compatibility
           ===================================================== */

        jh::runtime_arr<int> r{1, 2, 3, 4, 5};

        int sum = std::accumulate(r.begin(), r.end(), 0);

        std::cout << "sum via std::algorithm = "
                  << sum << "\n\n";


        /* =====================================================
           8. POD fast initialization
           ===================================================== */

        jh::runtime_arr<int> fast(
                1024,
                jh::runtime_arr<int>::uninitialized
        );

        for (std::size_t i = 0; i < fast.size(); ++i)
            fast[i] = static_cast<int>(i);

        std::cout << "POD uninitialized allocation filled manually\n";

        std::cout << "\n===== end runtime_arr usage =====\n";
    }

}

/**
 * @page runtime_arr_static_hash_index Static Hash Index Built from runtime_arr
 *
 * <h3>Overview</h3>
 *
 * In many real-world systems, key-value data is collected dynamically during
 * initialization. A typical approach uses:
 *
 * @code
 * std::unordered_map<K, V>
 * @endcode
 *
 * However, once initialization completes the dataset often becomes immutable.
 *
 * At that point we may prefer:
 *
 * <ul>
 *   <li>No bucket structure</li>
 *   <li>No rehashing</li>
 *   <li>No resizing</li>
 *   <li>Predictable contiguous memory</li>
 *   <li>Fast read-only lookup</li>
 * </ul>
 *
 * A practical approach is to convert the hash table into a compact
 * static lookup structure.
 *
 *
 * <h3>Construction Strategy</h3>
 *
 * Step 1 — Move the unordered_map contents into data_,
 * a <code>jh::runtime_arr&lt;std::pair&lt;K, V&gt;&gt;</code>:
 *
 * Step 2 — Build an index table containing:
 *
 * @code
 * entry { full_hash_of_key, index }
 * @endcode
 *
 * Step 3 — Sort the index table by `(hash, index)`:
 *
 * @code
 * std::stable_sort(entries.begin(), entries.end());
 * @endcode
 *
 * Step 4 — Move the sorted index into a <code>jh::runtime_arr&lt;entry&gt;</code>.
 *
 *
 * <h3>Lookup Algorithm</h3>
 *
 * Lookup becomes:
 *
 * @code
 * hash(key)
 * ↓
 * lower_bound(entries, (hash,0))
 * ↓
 * scan forward while hashes match
 * @endcode
 *
 * This yields complexity:
 *
 * @code
 * O(log N + k)
 * @endcode
 *
 * where <code>k</code> is the number of hash collisions.
 *
 *
 * <h3>Why This Can Be Faster Than std::unordered_map</h3>
 *
 * <ul>
 *   <li>No bucket indirection</li>
 *   <li>Fully contiguous memory</li>
 *   <li>Better cache locality</li>
 *   <li>No rehash operations</li>
 * </ul>
 *
 * In many workloads (configuration tables, registries, DSL interpreters,
 * static dictionaries) this approach performs significantly better than
 * <code>std::unordered_map</code>.
 */

#include <unordered_map>
#include <vector>
#include <jh/concepts> // for jh::hash deduction
#include <algorithm>
#include <string>

namespace example::simulated {

    template<typename K, typename V>
    class StaticHashIndex {

        struct entry {
            std::size_t hash;
            std::size_t index;

            auto operator<=>(const entry &) const noexcept = default;
        };

    public:
        StaticHashIndex(std::unordered_map<K, V> &&map) :
                data_{
                        std::make_move_iterator(map.begin()),
                        std::make_move_iterator(map.end())
                } {

            std::vector<entry> idx;
            idx.reserve(data_.size());

            std::hash<K> hasher;

            for (std::size_t i = 0; i < data_.size(); ++i)
                idx.emplace_back(hasher(data_[i].first), i);

            std::stable_sort(idx.begin(), idx.end());

            entries_ = jh::runtime_arr<entry>(std::move(idx));
        }

        [[nodiscard]] const V *find(const K &key) const {
            jh::hash<K> hasher{};
            // jh::hash is an auto-deducer that deduces hasher type for K
            // The priority of jh::hash deduction is:
            // std::hash<K> > hash(key) from ADL > key.hash() member function, else fails to compile
            // This is more flexible for user-defined or third-party types that may not have
            // std::hash specializations but do have custom hash functions available via ADL or member functions.
            // introduced from <jh/concepts> header
            std::size_t h = hasher(key);

            entry probe{h, 0};

            auto it = std::lower_bound(
                    entries_.begin(),
                    entries_.end(),
                    probe
            );

            while (it != entries_.end() && it->hash == h) {

                const auto &kv = data_[it->index];

                if (kv.first == key)
                    return &kv.second;

                ++it;
            }

            return nullptr;
        }

    private:

        jh::runtime_arr<std::pair<K, V>> data_{};
        jh::runtime_arr<entry> entries_{};
    };

} // namespace example::simulated


namespace example {

    void example_static_hash_index_runtime_arr() {
        using simulated::StaticHashIndex;

        std::cout << "\n===== static hash index demo =====\n\n";

        std::unordered_map<std::string, int> table;

        table.emplace("apple", 3);
        table.emplace("banana", 7);
        table.emplace("pear", 5);
        table.emplace("orange", 9);

        const StaticHashIndex<std::string, int> index(std::move(table));

        const std::string queries[] = {
                "apple",
                "pear",
                "orange",
                "grape"
        };

        for (const auto &key: queries) {

            const int *value = index.find(key);

            if (value)
                std::cout << key << " → " << *value << "\n";
            else
                std::cout << key << " → (not found)\n";
        }

        std::cout << "\n===== end static hash index demo =====\n";
    }

}

/**
 * @page runtime_arr_ranges_interop Interoperability with std::ranges and std::span
 *
 * <h3>Overview</h3>
 *
 * This example demonstrates how <code>jh::runtime_arr&lt;T&gt;</code>
 * interacts with C++20 <code>std::ranges</code> algorithms and
 * with <code>std::span</code>.
 *
 * <p>
 * The generic <code>runtime_arr&lt;T&gt;</code> is an owning fixed-size container.
 * It is not a view and it is intentionally non-copyable.
 * Therefore, when view-like slicing behavior is needed,
 * the canonical bridge is:
 * </p>
 *
 * @code
 * arr.as_span()
 * @endcode
 *
 * <p>
 * which produces a lightweight non-owning <code>std::span&lt;T&gt;</code>
 * over the same storage.
 * </p>
 *
 * <h3>What This Example Shows</h3>
 *
 * <ul>
 *   <li>Direct use of <code>runtime_arr&lt;T&gt;</code> with
 *       <code>std::ranges</code> algorithms.</li>
 *   <li>Use of <code>.as_span()</code> as a range/view bridge.</li>
 *   <li>Use of <code>std::span::subspan()</code> for slicing.</li>
 *   <li>Propagation of writes performed through span-based subranges.</li>
 * </ul>
 *
 * <h3>Important Note</h3>
 *
 * <p>
 * Since <code>runtime_arr</code> is move-only and not itself a view,
 * many view-oriented interactions are most naturally expressed
 * through <code>std::span</code>.
 * In practice, <code>std::span</code> is the recommended adapter
 * for this purpose.
 * </p>
 */

#include <ranges>
#include <span>
#include <algorithm>

namespace example {

    void example_runtime_arr_ranges_interop() {
        std::cout << "\n===== runtime_arr ranges interop =====\n\n";

        /* =====================================================
           1. runtime_arr as input to std::ranges algorithms
           ===================================================== */

        jh::runtime_arr<int> data{1, 2, 3, 4, 5, 6, 7, 8};

        const auto even_count = std::ranges::count_if(
                data,
                [](int x) { return x % 2 == 0; }
        );

        std::cout << "even element count in runtime_arr = "
                  << even_count << "\n";

        const auto found = std::ranges::find(data, 5);

        std::cout << "value 5 "
                  << (found != data.end() ? "found" : "not found")
                  << " in runtime_arr\n\n";


        /* =====================================================
           2. Obtain a span view from runtime_arr
           ===================================================== */

        std::span<int> whole = data.as_span();

        const auto gt_four = std::ranges::count_if(
                whole,
                [](int x) { return x > 4; }
        );

        std::cout << "elements > 4 in span view = "
                  << gt_four << "\n\n";


        /* =====================================================
           3. Slice with subspan()
           ===================================================== */

        std::span<int> middle = whole.subspan(2, 4);

        std::cout << "middle subspan values: ";
        std::ranges::for_each(middle, [](int v) {
            std::cout << v << ' ';
        });
        std::cout << "\n\n";


        /* =====================================================
           4. Modify through subspan using std::ranges::transform
           ===================================================== */

        std::ranges::transform(
                middle,
                middle.begin(),
                [](int x) { return x * 10; }
        );

        std::cout << "runtime_arr after transforming middle subspan: ";
        std::ranges::for_each(data, [](int v) {
            std::cout << v << ' ';
        });
        std::cout << "\n\n";


        /* =====================================================
           5. Build a filtered view from span
           ===================================================== */

        auto even_view = whole | std::views::filter([](int x) {
            return x % 2 == 0;
        });

        std::cout << "even elements through span-based filter view: ";
        for (int v: even_view)
            std::cout << v << ' ';
        std::cout << "\n\n";


        /* =====================================================
           6. Construct runtime_arr from an iterator range
           ===================================================== */

        std::vector<int> source{10, 20, 30, 40, 50};

        jh::runtime_arr<int> copied(source.begin(), source.end());

        std::cout << "copied runtime_arr values: ";
        std::ranges::for_each(copied, [](int v) {
            std::cout << v << ' ';
        });
        std::cout << "\n";

        std::cout << "\n===== end ranges interop demo =====\n";
    }

}
/**
 * @page runtime_arr_allocator_interop Allocator Interoperability
 *
 * <h3>Overview</h3>
 *
 * <code>jh::runtime_arr&lt;T, Alloc&gt;</code> supports allocator-based
 * construction in a manner similar to standard containers.
 *
 * The allocator parameter is optional.
 * When omitted, <code>typed::monostate</code> is used internally,
 * and memory is allocated using:
 *
 * <ul>
 *   <li><code>new[]</code></li>
 *   <li><code>delete[]</code></li>
 * </ul>
 *
 * When a user allocator is supplied, <code>runtime_arr</code>
 * resolves how to use it at compile time.
 *
 * The resolution rule is:
 *
 * <ol>
 *   <li>If the allocator directly provides
 *       <code>allocate(n)</code> and
 *       <code>deallocate(ptr,n)</code>,
 *       it is used directly.</li>
 *
 *   <li>If the allocator follows the STL allocator model,
 *       <code>std::allocator_traits::rebind_alloc&lt;T&gt;</code>
 *       is used automatically.</li>
 * </ol>
 *
 * This allows seamless interoperability with
 * standard allocator types such as:
 *
 * <ul>
 *   <li><code>std::allocator</code></li>
 *   <li><code>std::pmr::polymorphic_allocator</code></li>
 * </ul>
 *
 * Unlike <code>std::vector</code>, the allocator is only used
 * during construction because <code>runtime_arr</code> never
 * performs reallocation or resizing.
 *
 * <h3>Example</h3>
 *
 * See:
 *
 * <ul>
 *   <li><code>example::example_runtime_arr_allocator_interop()</code></li>
 * </ul>
 */

#include <memory_resource>

namespace example {

    void example_runtime_arr_allocator_interop() {

        std::cout << "\n===== runtime_arr allocator interop =====\n\n";

        /* =====================================================
           1. std::allocator
           ===================================================== */

        {
            std::cout << "std::allocator example:\n";

            std::allocator<int> alloc{};

            jh::runtime_arr<int, std::allocator<int>>
                    arr(6, alloc);

            for (std::size_t i = 0; i < arr.size(); ++i)
                arr[i] = static_cast<int>(i * 3);

            for (auto v: arr)
                std::cout << v << " ";

            std::cout << "\n\n";
        }


        /* =====================================================
           2. pmr::polymorphic_allocator
           ===================================================== */

        {
            std::cout << "pmr::polymorphic_allocator example:\n";

            std::byte buffer[1024];

            std::pmr::monotonic_buffer_resource pool{
                    buffer, sizeof(buffer)};

            std::pmr::polymorphic_allocator<int> alloc{&pool};

            jh::runtime_arr<int,
                    std::pmr::polymorphic_allocator<int>>
                    arr(8, alloc);

            for (std::size_t i = 0; i < arr.size(); ++i)
                arr[i] = static_cast<int>(i);

            for (auto v: arr)
                std::cout << v << " ";

            std::cout << "\n\n";
        }


        /* =====================================================
           3. vector → runtime_arr with allocator
           ===================================================== */

        {
            std::cout << "vector → runtime_arr with allocator:\n";

            std::vector<int> src{10, 20, 30, 40};

            std::pmr::monotonic_buffer_resource pool{};

            std::pmr::polymorphic_allocator<int> alloc{&pool};

            jh::runtime_arr<int,
                    std::pmr::polymorphic_allocator<int>>
                    arr(std::move(src), alloc);

            for (auto v: arr)
                std::cout << v << " ";

            std::cout << "\n\n";
        }
        /* =====================================================
           4. iterator range + allocator construction
           ===================================================== */

        {
            std::cout << "iterator range + allocator:\n";

            std::vector<int> source{5, 10, 15, 20, 25};

            std::pmr::monotonic_buffer_resource pool{};

            std::pmr::polymorphic_allocator<int> alloc{&pool};

            jh::runtime_arr<
                    int,
                    std::pmr::polymorphic_allocator<int>
            > arr(source.begin(), source.end(), alloc);

            for (auto v: arr)
                std::cout << v << " ";

            std::cout << "\n\n";
        }

        std::cout << "===== end allocator interop =====\n";
    }

}

/**
 * @page runtime_arr_bool_specialization runtime_arr<bool> Specialization
 *
 * <h3>Overview</h3>
 *
 * The type <code>jh::runtime_arr&lt;bool&gt;</code> is a specialized
 * container that stores boolean values in a bit-packed form.
 *
 * Instead of allocating one byte per element, the container
 * compresses elements into a sequence of machine words
 * (<code>uint64_t</code>).
 *
 * This reduces memory usage by a factor of eight compared to
 * a byte-based representation.
 *
 *
 * <h3>Key Characteristics</h3>
 *
 * The bit-packed specialization differs from the generic
 * <code>runtime_arr&lt;T&gt;</code> in several ways:
 *
 * <ul>
 *   <li>Elements are stored as packed bits.</li>
 *   <li>No <code>data()</code> function is provided.</li>
 *   <li>The container does not model a contiguous range.</li>
 *   <li>Iterators return proxy values rather than raw references.</li>
 * </ul>
 *
 * Access to individual elements is provided through:
 *
 * <ul>
 *   <li><code>set(index)</code></li>
 *   <li><code>unset(index)</code></li>
 *   <li><code>test(index)</code></li>
 *   <li><code>operator[]</code></li>
 * </ul>
 *
 * The underlying storage can also be inspected using:
 *
 * <ul>
 *   <li><code>raw_data()</code></li>
 *   <li><code>raw_word_count()</code></li>
 * </ul>
 *
 *
 * <h3>Flat Boolean Variant</h3>
 *
 * Some applications require a standard contiguous container
 * where each element occupies one byte.
 *
 * This behavior can be obtained using:
 *
 * @code
 * jh::runtime_arr<bool, jh::runtime_arr_helper::bool_flat_alloc>
 * @endcode
 *
 * In this configuration the container behaves like a normal
 * contiguous array and supports operations such as
 * <code>as_span()</code>.
 *
 *
 * <h3>Use Cases</h3>
 *
 * Bit-packed boolean arrays are commonly used in:
 *
 * <ul>
 *   <li>visited-node flags in graph algorithms</li>
 *   <li>occupancy grids</li>
 *   <li>filter masks</li>
 *   <li>feature flags</li>
 * </ul>
 *
 *
 * <h3>Example</h3>
 *
 * See:
 *
 * <ul>
 *   <li><code>example::example_runtime_arr_bool()</code></li>
 * </ul>
 */

namespace example {

    void example_runtime_arr_bool() {

        std::cout << "\n===== runtime_arr<bool> examples =====\n\n";

        /* =====================================================
           1. bit-packed runtime_arr<bool>  (default)
           ===================================================== */

        {
            std::cout << "[bit-packed bool array]\n";

            jh::runtime_arr<bool> bits(16);

            // --- set operations ---
            bits.set(1);
            bits.set(3);
            bits.set(5, true);

            // --- unset ---
            bits.unset(3);

            // --- operator[] proxy ---
            bits[7] = true;

            // --- at() with bounds check ---
            bits.at(9) = true;

            // --- read values ---
            std::cout << "values:\n";
            for (std::uint64_t i = 0; i < bits.size(); ++i)
                std::cout << bits.test(i) << " ";

            std::cout << "\n";

            // --- iterator traversal ---
            std::cout << "iterator traversal:\n";
            for (bool v: bits)
                std::cout << v << " ";

            std::cout << "\n";

            // --- raw storage inspection ---
            std::cout << "raw words: " << bits.raw_word_count() << "\n";

            auto *raw = bits.raw_data();
            for (std::uint64_t i = 0; i < bits.raw_word_count(); ++i)
                std::cout << "word[" << i << "] = " << raw[i] << "\n";

            // --- reset ---
            bits.reset_all();

            std::cout << "after reset_all():\n";
            for (bool v: bits)
                std::cout << v << " ";

            std::cout << "\n\n";
        }


        /* =====================================================
           2. flat bool array (no compression)
           ===================================================== */

        {
            std::cout << "[flat bool array]\n";

            using flat_bool_arr =
                    jh::runtime_arr<
                            bool,
                            jh::runtime_arr_helper::bool_flat_alloc
                    >;

            flat_bool_arr arr(10);

            arr[0] = true;
            arr[3] = true;
            arr[5] = true;

            // direct span access
            auto span = arr.as_span();

            std::cout << "values:\n";
            for (bool v: span)
                std::cout << v << " ";

            std::cout << "\n";

            // modify through span
            span[1] = true;

            std::cout << "after span write:\n";
            for (bool v: arr)
                std::cout << v << " ";
            std::cout << "\n";
            // --- reset ---
            arr.reset_all();

            std::cout << "after reset_all():\n";
            for (bool v: arr)
                std::cout << v << " ";

            std::cout << "\n\n";
        }

        std::cout << "===== end runtime_arr<bool> =====\n";
    }

}

int main() {
    example::example_matrix_runtime_arr();
    example::example_runtime_arr_basic_usage();
    example::example_static_hash_index_runtime_arr();
    example::example_runtime_arr_ranges_interop();
    example::example_runtime_arr_allocator_interop();
    example::example_runtime_arr_bool();
    return 0;
}
