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
 * This mirrors NumPy’s default C-order layout.
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
 *     <li>either opt for reference copy <code>auto& b = a</code></li>
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
                  data_(other.size()) {
            std::copy(other.raw(),
                      other.raw() + other.size(),
                      raw());
        }

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

int main() {
    example::example_matrix_runtime_arr();
    return 0;
}
