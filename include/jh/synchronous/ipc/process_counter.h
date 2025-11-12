/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * \endverbatim
 */
/**
 * @file process_counter.h (synchronous/ipc)
 * @brief Cross-process shared integer counter implemented via named shared memory.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::sync::ipc::process_counter</code> provides a process-visible 64-bit integer
 * stored in OS-level shared memory and synchronized by a per-instance
 * <code>process_mutex&lt;S + ".loc"&gt;</code>.
 * It behaves as a globally accessible atomic counter with read–modify–write semantics
 * enforced through inter-process locking.
 * </p>
 *
 * <h3>Implementation</h3>
 * <ul>
 *   <li><b>POSIX (Linux / BSD / Darwin)</b>:
 *     <ul>
 *       <li>Backed by <code>shm_open</code> + <code>mmap</code>.</li>
 *       <li>Permissions determined by <code>JH_PROCESS_MUTEX_SHARED</code> (0644 or 0666).</li>
 *       <li>No special privileges required.</li>
 *     </ul>
 *   </li>
 *   <li><b>Windows / MSYS2</b>:
 *     <ul>
 *       <li>Backed by <code>CreateFileMapping</code> + <code>MapViewOfFile</code>.</li>
 *       <li>Objects are created under the <code>Global&bsol;&bsol;</code> namespace for inter-process visibility.</li>
 *       <li><strong>Administrator privilege</strong> is required to create or open
 *           <code>Global&bsol;&bsol;</code> objects.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Naming rules</h3>
 * <ul>
 *   <li>Allowed characters: <code>[A-Za-z0-9_.-]</code>.</li>
 *   <li>No leading slash (<code>'/'</code>); it is added automatically when required by the OS.</li>
 *   <li>Name length is <strong>platform-dependent</strong>:
 *     <ul>
 *       <li><b>FreeBSD / Darwin</b>: 30 characters (POSIX limit 31 bytes including '/').</li>
 *       <li><b>Linux / Windows</b>: 128 characters (portable engineering limit).</li>
 *     </ul>
 *   </li>
 *   <li>The limit is verified at compile time using
 *       <code>jh::sync::ipc::limits::valid_object_name&lt;S, limits::max_name_length - 4&gt;</code>,
 *       where <code>-4</code> reserves space for the <code>".loc"</code> suffix used by its mutex.</li>
 * </ul>
 *
 * <h3>unlink semantics</h3>
 * <ul>
 *   <li><b>POSIX</b>: calls <code>shm_unlink()</code>; existing mappings remain valid
 *       until unmapped by all processes. The operation is idempotent.</li>
 *   <li><b>Windows</b>: no explicit unlink; shared memory is destroyed automatically
 *       when the last handle closes.</li>
 * </ul>
 *
 * <h4>Windows privilege requirement</h4>
 * <p>
 * On Windows, <code>process_counter</code> requires <b>administrator privilege</b>
 * because shared memory objects are created in the <code>Global&bsol;&bsol;</code> namespace.
 * This does not affect POSIX systems.
 * </p>
 *
 */

#pragma once
#include "jh/metax/t_str.h"
#include "jh/macros/platform.h"
#include "jh/synchronous/ipc/process_mutex.h"
#include "jh/synchronous/ipc/ipc_limits.h"

#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <string>
#include <functional>

#if IS_WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif


namespace jh::sync::ipc {

    using jh::meta::TStr;

    /**
     * @brief Cross-process integer counter stored in shared memory (POSIX / Win32).
     *
     * Provides process-visible integer storage with lock-protected modification.
     * Uses OS-named shared memory (POSIX shm or Win32 file mapping) and a dedicated
     * <code>process_mutex&lt;S + ".loc"&gt;</code> for consistency.
     *
     * <h4>Design goals</h4>
     * <ul>
     *   <li>Safe across processes and threads.</li>
     *   <li>Consistent behavior between POSIX (<code>shm_open</code>) and Windows (<code>CreateFileMapping</code>).</li>
     *   <li>Lock-protected read–modify–write semantics.</li>
     * </ul>
     *
     * <h4>Read semantics</h4>
     * <ul>
     *   <li><code>load()</code> — lightweight, relaxed read.
     *       Does <b>not</b> guarantee visibility of concurrent updates,
     *       but is sufficient when reading slightly stale values is harmless
     *       (e.g. in <code>shared_process_mutex</code> to check reader counts).</li>
     *   <li><code>load_strong()</code> — sequentially consistent read
     *       (full memory fence). Suitable for transactional reads
     *       requiring visibility of all preceding writes in any process.</li>
     *   <li><code>load_force()</code> — acquires the internal mutex before reading.
     *       Ensures full synchronization with all writers; used when
     *       strict atomicity or serialized state inspection is required.</li>
     * </ul>
     *
     * <h4>Write semantics</h4>
     * <ul>
     *   <li>All modification methods (<code>store</code>, <code>fetch_add</code>,
     *       <code>fetch_sub</code>, <code>fetch_apply</code>) acquire the same mutex
     *       to ensure atomicity across processes.</li>
     *   <li>Operations are linearized globally within the named counter scope.</li>
     * </ul>
     *
     * <h4>Semantic clarification</h4>
     * <p>
     * The API intentionally mimics <code>std::atomic&lt;uint64_t&gt;</code>,
     * but this class is <strong>not an atomic type</strong> in the C++ memory model.
     * On non-Linux systems, there is no standard facility for <em>memory-mapped atomics</em>;
     * therefore, atomicity across processes must be provided explicitly via
     * a lightweight inter-process mutex.
     * </p>
     * <p>
     * From a design standpoint, <code>process_counter</code> remains a
     * <strong>primitive</strong> rather than a composite abstraction —
     * it encapsulates synchronization internally and exposes a simple
     * read–modify–write interface suitable for inter-process coordination.
     * </p>
     *
     * <h4>Internal synchronization objects</h4>
     * <ul>
     *   <li><b>Main mutex</b>: <code>process_mutex&lt;S + ".loc"&gt;</code> — protects all
     *       read–modify–write operations on the counter value.</li>
     *   <li><b>Initialization mutex</b>: <code>process_mutex&lt;S&gt;</code> — guards the
     *       one-time initialization of the shared memory region (ensures that
     *       <code>initialized</code> flag and <code>value</code> are safely set
     *       by the first process that creates the object).</li>
     * </ul>
     *
     * <p>
     * Both mutexes are created in the same namespace as the counter, which means
     * that if a user manually defines a <code>process_mutex&lt;S&gt;</code> or
     * <code>process_mutex&lt;S + ".loc"&gt;</code> elsewhere, it will conflict
     * with the internal synchronization of <code>process_counter&lt;S&gt;</code>.
     * Therefore, avoid declaring any <code>process_mutex</code> with the same
     * template literal <code>S</code> or its <code>".loc"</code> suffix.
     * </p>
     *
     * <h4>Usage notes</h4>
     * <ul>
     *   <li>Acts as a globally shared 64-bit counter.</li>
     *   <li>All accesses within the same template literal <code>S</code>
     *       map to the same shared-memory region across all processes.</li>
     *   <li>The counter is lazily initialized once per namespace;
     *       subsequent accesses reuse the same shared mapping.</li>
     * </ul>
     */
    template <TStr S, bool HighPriv = false>
    requires (limits::valid_object_name<S, limits::max_name_length - 4>())
    class process_counter final {
    private:

#if IS_WINDOWS
        static constexpr auto shm_name_  = jh::meta::t_str{"Global\\"} + S;
#else
        static constexpr auto shm_name_  = jh::meta::t_str{"/"} + S;
        static constexpr mode_t shm_mode = JH_PROCESS_MUTEX_SHARED ? 0666 : 0644;
#endif

        using lock_t = process_mutex<S + jh::meta::t_str{".loc"}, HighPriv>;

        struct counter_data {
            alignas(alignof(std::uint64_t)) std::uint64_t value;
            bool initialized;
        };

#if IS_WINDOWS
        HANDLE map_ = nullptr;
#else
        int fd_ = -1;
#endif
        counter_data* data_ = nullptr;
        lock_t& lock_;

        process_counter() : lock_(lock_t::instance()) {
#if IS_WINDOWS
            map_ = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                    0, sizeof(counter_data), shm_name_.val());
            if (!map_)
                throw std::runtime_error("process_counter: CreateFileMapping failed (errno=" +
                                         std::to_string(::GetLastError()) + ")");
            void* ptr = ::MapViewOfFile(map_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(counter_data));
            if (!ptr)
                throw std::runtime_error("process_counter: MapViewOfFile failed (errno=" +
                                         std::to_string(::GetLastError()) + ")");
            data_ = static_cast<counter_data*>(ptr);
#else
            fd_ = ::shm_open(shm_name_.val(), O_CREAT | O_RDWR, shm_mode);
            if (fd_ == -1)
                throw std::runtime_error("process_counter: shm_open failed (errno=" + std::to_string(errno) + ")");
            struct stat st{};
            if (::fstat(fd_, &st) == -1)
                throw std::runtime_error("process_counter: fstat failed (errno=" + std::to_string(errno) + ")");
            if (st.st_size < sizeof(counter_data))
                if (::ftruncate(fd_, sizeof(counter_data)) == -1)
                    throw std::runtime_error("process_counter: ftruncate failed (errno=" + std::to_string(errno) + ")");
            void* ptr = ::mmap(nullptr, sizeof(counter_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            if (ptr == MAP_FAILED)
                throw std::runtime_error("process_counter: mmap failed (errno=" + std::to_string(errno) + ")");
            data_ = static_cast<counter_data*>(ptr);
            ::close(fd_);
#endif
            // initialization guard
            auto& init_guard = process_mutex<S>::instance();
            std::lock_guard global_lock(init_guard);
            std::lock_guard counter_lock(lock_);
            if (!data_->initialized) {
                data_->value = 0;
                data_->initialized = true;
            }
        }

        ~process_counter() noexcept {
#if IS_WINDOWS
            if (data_) ::UnmapViewOfFile(data_);
            if (map_) ::CloseHandle(map_);
#else
            if (data_) ::munmap(data_, sizeof(counter_data));
#endif
        }

    public:
        // Disable copy
        process_counter(const process_counter&) = delete;
        process_counter& operator=(const process_counter&) = delete;

        /// @brief Singleton instance.
        static process_counter& instance() {
            static process_counter inst;
            return inst;
        }

        /**
         * @brief Lightweight relaxed load.
         *
         * Performs an acquire-fenced read of the shared value.
         * May return a slightly stale value if another process updates concurrently,
         * but guarantees no tearing or partial reads.
         *
         * <h4>Usage</h4>
         * <ul>
         *   <li>Use in performance-sensitive paths where exact freshness is not required.</li>
         *   <li>Typical use: reader-count check or approximate monitoring.</li>
         * </ul>
         *
         * @return Current (possibly slightly outdated) counter value.
         */
        [[nodiscard]] std::uint64_t load() noexcept {
            std::atomic_thread_fence(std::memory_order_acquire);
            return data_->value;
        }

        /**
         * @brief Strong, sequentially consistent load.
         *
         * Performs a full sequential-consistency fence before reading.
         * Ensures visibility of all preceding writes across processes
         * that use the same shared memory region.
         *
         * <h4>Usage</h4>
         * <ul>
         *   <li>Use when a transactionally accurate view is required.</li>
         *   <li>Guarantees visibility order equivalent to <code>std::atomic&lt;T&gt;::load(memory_order_seq_cst)</code>.</li>
         * </ul>
         *
         * @return The most recent globally visible counter value.
         */
        [[nodiscard]] std::uint64_t load_strong() noexcept {
            std::atomic_thread_fence(std::memory_order_seq_cst);
            return data_->value;
        }

        /**
         * @brief Locked and synchronized load.
         *
         * Acquires the internal process-wide mutex before reading,
         * guaranteeing full serialization with all concurrent writers.
         *
         * <h4>Usage</h4>
         * <ul>
         *   <li>Use for diagnostic or critical-path reads that must reflect
         *       an exact, globally synchronized value.</li>
         *   <li>Typically used for invariant checks or transactional control logic.</li>
         * </ul>
         *
         * @return The exact counter value after synchronization with all writers.
         */
        [[nodiscard]] std::uint64_t load_force() noexcept {
            std::lock_guard guard(lock_);
            std::atomic_thread_fence(std::memory_order_acquire);
            return data_->value;
        }

        /**
         * @brief Atomically replace the counter value.
         *
         * Acquires the internal lock, writes the new value, and enforces
         * release and sequential consistency ordering.
         *
         * @param v New value to store.
         */
        void store(std::uint64_t v) noexcept {
            std::lock_guard guard(lock_);
            std::atomic_thread_fence(std::memory_order_release);
            data_->value = v;
            std::atomic_thread_fence(std::memory_order_seq_cst);
        }

        /**
         * @brief Atomically add to the counter.
         *
         * Increments the counter by <code>delta</code> under lock protection.
         *
         * @param delta Increment amount (default 1).
         * @return Previous counter value before addition.
         */
        std::uint64_t fetch_add(std::uint64_t delta = 1) noexcept {
            std::lock_guard guard(lock_);
            auto old = data_->value;
            data_->value += delta;
            std::atomic_thread_fence(std::memory_order_seq_cst);
            return old;
        }

        /**
         * @brief Atomically subtract from the counter.
         *
         * Decrements the counter by <code>delta</code> under lock protection.
         *
         * @param delta Decrement amount (default 1).
         * @return Previous counter value before subtraction.
         */
        std::uint64_t fetch_sub(std::uint64_t delta = 1) noexcept {
            std::lock_guard guard(lock_);
            auto old = data_->value;
            data_->value -= delta;
            std::atomic_thread_fence(std::memory_order_seq_cst);
            return old;
        }

        /**
         * @brief Apply a custom transformation atomically.
         *
         * Executes <code>func(old_value)</code> under exclusive lock, replaces
         * the stored value with the result, and returns the previous value.
         *
         * @tparam F Callable type with signature <code>uint64_t(uint64_t)</code>.
         * @param func Transformation function.
         * @return The previous counter value before transformation.
         */
        template <typename F>
        requires std::invocable<F, std::uint64_t> &&
                 std::same_as<std::invoke_result_t<F, std::uint64_t>, std::uint64_t>
        std::uint64_t fetch_apply(F&& func) noexcept(noexcept(std::invoke(std::forward<F>(func), std::uint64_t{}))) {
            std::lock_guard guard(lock_);
            auto old = data_->value;
            auto new_v = std::invoke(std::forward<F>(func), old);
            data_->value = new_v;
            std::atomic_thread_fence(std::memory_order_seq_cst);
            return old;
        }

        /**
         * @brief Remove the counter's shared-memory object from the namespace (POSIX only).
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>On POSIX systems, calls <code>shm_unlink()</code> for the counter name,
         *       and then delegates to <code>process_mutex&lt;S&gt;::unlink()</code> and <code>process_mutex&lt;S + ".loc"&gt;::unlink()</code>.</li>
         *   <li>If the object does not exist (<code>errno == ENOENT</code>),
         *       the call is silently ignored.</li>
         *   <li>If unlinking fails for other reasons, a <code>std::runtime_error</code> is thrown.</li>
         * </ul>
         *
         * <h4>Idempotency</h4>
         * <p>
         * The operation is idempotent: calling <code>unlink()</code> multiple times is safe.
         * Once removed, subsequent calls are treated as no-ops.
         * </p>
         *
         * <h4>Windows</h4>
         * <p>
         * On Windows / MSYS2, there is no explicit unlink concept. Shared memory
         * and synchronization handles are destroyed automatically when the last
         * process closes them.
         * </p>
         */
        static void unlink() requires(HighPriv) {
#if IS_WINDOWS
            // no unlink
#else
            if (::shm_unlink(shm_name_.val()) == -1 && errno != ENOENT)
                throw std::runtime_error(
                        "shm_unlink failed for " + std::string{shm_name_.val()} +
                        " (errno=" + std::to_string(errno) + ")");
            process_mutex<S, HighPriv>::unlink();
            lock_t::unlink();
#endif
        }
        /// Disabled if HighPriv == false. Non-privileged variants cannot call unlink().
        static void unlink() requires(!HighPriv) = delete;
    };

} // namespace jh::sync::ipc
