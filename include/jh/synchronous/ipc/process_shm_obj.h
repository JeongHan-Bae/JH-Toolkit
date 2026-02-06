/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file process_shm_obj.h
 * @brief Cross-process shared-memory container for POD-like objects.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::sync::ipc::process_shm_obj</code> exposes a process-visible,
 * named shared-memory region containing a single POD-like object of type <code>T</code>.
 * All participating processes reference the same mapped storage, coordinated through
 * a pair of inter-process mutexes.
 * </p>
 *
 * <h3>Type constraints</h3>
 * <ul>
 *   <li><code>T</code> must satisfy <code>jh::pod::cv_free_pod_like</code>.</li>
 *   <li>This means <code>T</code> is trivially copyable, constructible,
 *       destructible, and has standard layout, <b>without</b> <code>const</code>
 *       or <code>volatile</code> qualification.</li>
 *   <li>This restriction ensures <code>T</code> can be directly memory-mapped
 *       without invoking constructors or destructors.</li>
 * </ul>
 *
 * <h3>Implementation</h3>
 * <ul>
 *   <li><b>POSIX (Linux / BSD / Darwin)</b>:
 *     <ul>
 *       <li>Backed by <code>shm_open()</code> + <code>mmap()</code>.</li>
 *       <li>Permissions determined by <code>JH_PROCESS_MUTEX_SHARED</code> (0644 or 0666).</li>
 *     </ul>
 *   </li>
 *   <li><b>Windows / MSYS2</b>:
 *     <ul>
 *       <li>Backed by <code>CreateFileMapping()</code> + <code>MapViewOfFile()</code>.</li>
 *       <li>Objects reside in <code>Global&bsol;&bsol;</code> namespace for inter-process visibility.</li>
 *       <li><strong>Administrator privilege</strong> required to open or create mappings.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Internal synchronization objects</h3>
 * <ul>
 *   <li><b>Initialization mutex</b>: <code>process_mutex&lt;S&gt;</code> &mdash;
 *       protects one-time initialization of the mapped region.</li>
 *   <li><b>Access mutex</b>: <code>process_mutex&lt;S + ".loc"&gt;</code> &mdash;
 *       protects all concurrent writes and ensures serialization of modifications.</li>
 * </ul>
 *
 * <p>
 * Both mutexes are created in the same namespace as the shared object. Declaring a
 * separate <code>process_mutex&lt;S&gt;</code> or <code>process_mutex&lt;S + ".loc"&gt;</code>
 * externally will conflict with this internal synchronization scheme.
 * </p>
 *
 * <h3>Usage guidelines</h3>
 * <ul>
 *   <li>Call <code>instance()</code> to obtain the singleton mapping.</li>
 *   <li>Use <code>lock()</code> to acquire the access mutex before any write.</li>
 *   <li>Perform reads using <code>flush_acquire()</code> beforehand to guarantee
 *       visibility of the latest writes from other processes.</li>
 *   <li>When writing:
 *     <ul>
 *       <li>Acquire a <code>std::lock_guard</code> (RAII style) on <code>lock()</code>.</li>
 *       <li>Modify the object via <code>ref()</code> or <code>ptr()</code>.</li>
 *       <li><b>Before releasing the lock</b> &mdash; i.e., before the RAII guard leaves scope &mdash;
 *           call <code>flush_release()</code> or <code>flush_seq()</code> to publish the change.</li>
 *     </ul>
 *   </li>
 *   <li>This class generalizes <code>process_counter</code>: it provides the same
 *       memory-sharing model but allows arbitrary POD layouts. Users must therefore
 *       manually manage synchronization scope and visibility fences.</li>
 * </ul>
 *
 * <h3>Unlink semantics</h3>
 * <ul>
 *   <li><b>POSIX</b>: Invokes <code>shm_unlink()</code> for the region, then calls
 *       <code>process_mutex&lt;S&gt;::unlink()</code> and
 *       <code>process_mutex&lt;S + ".loc"&gt;::unlink()</code> to remove both locks.</li>
 *   <li><b>Windows</b>: No explicit unlink; shared objects are destroyed automatically
 *       once all handles are closed.</li>
 *   <li>Operation is idempotent; redundant calls are safe.</li>
 * </ul>
 *
 * <h3>Windows privilege requirement</h3>
 * <p>
 * On Windows, this primitive requires <b>administrator privilege</b> because
 * named shared objects and events are created under <code>Global&bsol;&bsol;</code>.
 * POSIX platforms impose no such restriction.
 * </p>
 */

#pragma once

#include "jh/metax/t_str.h"
#include "jh/macros/platform.h"
#include "jh/pods/pod_like.h"
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

    /**
     * @brief Cross-process shared-memory container for a single POD-like object.
     *
     * Provides a memory-mapped instance of <code>T</code> visible to all processes
     * sharing the same name <code>S</code>. Synchronization and initialization are
     * managed internally via named process-level mutexes.
     *
     * @tparam S Shared-memory name literal (letters, digits, dot, dash, underscore).
     * @tparam T POD-like type satisfying <code>cv_free_pod_like</code>.
     * @tparam HighPriv Enables privileged operations (e.g. <code>unlink()</code>).
     */
    template <jh::meta::TStr S, jh::pod::cv_free_pod_like T, bool HighPriv = false>
    requires (limits::valid_object_name<S, limits::max_name_length - 4>())
    class process_shm_obj final {
    private:

#if IS_WINDOWS
        static constexpr auto shm_name_  = jh::meta::TStr{"Global\\"} + S;
#else
        static constexpr auto shm_name_  = jh::meta::TStr{"/"} + S;
        static constexpr mode_t shm_mode = JH_PROCESS_MUTEX_SHARED ? 0666 : 0644;
#endif

        using lock_t = process_mutex<S + jh::meta::TStr{".loc"}, HighPriv>;

        struct shm_data final {
            alignas(alignof(T)) T obj;
            bool initialized;
        };

#if IS_WINDOWS
        HANDLE map_ = nullptr;
#else
        int fd_ = -1;
#endif
        shm_data* data_ = nullptr;
        lock_t& lock_;

        process_shm_obj() : lock_(lock_t::instance()) {
#if IS_WINDOWS
            map_ = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                    0, sizeof(shm_data), shm_name_.val());
            if (!map_)
                throw std::runtime_error("process_shm_obj: CreateFileMapping failed (errno=" +
                                         std::to_string(::GetLastError()) + ")");
            void* ptr = ::MapViewOfFile(map_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(shm_data));
            if (!ptr)
                throw std::runtime_error("process_shm_obj: MapViewOfFile failed (errno=" +
                                         std::to_string(::GetLastError()) + ")");
            data_ = static_cast<shm_data*>(ptr);
#else
            fd_ = ::shm_open(shm_name_.val(), O_CREAT | O_RDWR, shm_mode);
            if (fd_ == -1)
                throw std::runtime_error("process_shm_obj: shm_open failed (errno=" + std::to_string(errno) + ")");
            struct stat st{};
            if (::fstat(fd_, &st) == -1)
                throw std::runtime_error("process_shm_obj: fstat failed (errno=" + std::to_string(errno) + ")");
            if (st.st_size < sizeof(shm_data))
                if (::ftruncate(fd_, sizeof(shm_data)) == -1)
                    throw std::runtime_error("process_shm_obj: ftruncate failed (errno=" + std::to_string(errno) + ")");
            void* ptr = ::mmap(nullptr, sizeof(shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            if (ptr == MAP_FAILED)
                throw std::runtime_error("process_shm_obj: mmap failed (errno=" + std::to_string(errno) + ")");
            data_ = static_cast<shm_data*>(ptr);
            ::close(fd_);
#endif
            // initialization guard
            auto& init_guard = process_mutex<S>::instance();
            std::lock_guard global_lock(init_guard);
            std::lock_guard counter_lock(lock_);
            if (!data_->initialized) {
                data_->obj = T{};
                data_->initialized = true;
            }
        }

        ~process_shm_obj() noexcept {
#if IS_WINDOWS
            if (data_) ::UnmapViewOfFile(data_);
            if (map_) ::CloseHandle(map_);
#else
            if (data_) ::munmap(data_, sizeof(shm_data));
#endif
        }

    public:
        // Disable copy
        process_shm_obj(const process_shm_obj&) = delete;
        process_shm_obj& operator=(const process_shm_obj&) = delete;

        /// @brief Singleton accessor.
        static process_shm_obj& instance() {
            static process_shm_obj inst;
            return inst;
        }

        /**
         * @brief Obtain non-const pointer to the shared object.
         * @return Pointer to the mapped shared instance of <code>T</code>.
         */
        [[nodiscard]] T* ptr() noexcept { return std::launder(&data_->obj); }

        /**
         * @brief Obtain const pointer to the shared object.
         * @return Const pointer to the mapped shared instance.
         */
        [[nodiscard]] const T* ptr() const noexcept { return std::launder(&data_->obj); }

        /**
         * @brief Obtain non-const reference to the shared object.
         * @return Reference to the shared instance.
         */
        [[nodiscard]] T& ref() noexcept { return *std::launder(&data_->obj); }

        /**
         * @brief Obtain const reference to the shared object.
         * @return Const reference to the shared instance.
         */
        [[nodiscard]] const T& ref() const noexcept { return *std::launder(&data_->obj); }

        /**
         * @brief Operator-> convenience accessor.
         * @return Pointer to the shared object.
         */
        [[nodiscard]] T* operator->() noexcept { return ptr(); }

        /**
         * @brief Const Operator-> convenience accessor.
         * @return Const pointer to the shared object.
         */
        [[nodiscard]] const T* operator->() const noexcept { return ptr(); }

        /**
         * @brief Operator* convenience accessor.
         * @return Reference to the shared object.
         */
        [[nodiscard]] T& operator*() noexcept { return ref(); }

        /**
         * @brief Const Operator* convenience accessor.
         * @return Const reference to the shared object.
         */
        [[nodiscard]] const T& operator*() const noexcept { return ref(); }

        /**
         * @brief Accessor for the inter-process mutex protecting this shared object.
         * @return Reference to the internal <code>process_mutex&lt;S + ".loc"&gt;</code>.
         */
        [[nodiscard]] lock_t& lock() noexcept { return lock_; }

        /**
         * @brief Acquire fence ensuring visibility of preceding writes by other processes.
         *
         * Use this before reading from the shared object to guarantee memory ordering.
         */
        [[maybe_unused]] static inline void flush_acquire() noexcept {
            std::atomic_thread_fence(std::memory_order_acquire);
        }

        /**
         * @brief Release fence ensuring visibility of local writes to other processes.
         *
         * Call this before releasing the lock after modifying the shared object.
         */
        [[maybe_unused]] static inline void flush_release() noexcept {
            std::atomic_thread_fence(std::memory_order_release);
        }

        /**
         * @brief Sequential-consistency fence for full memory ordering.
         *
         * Equivalent to <code>std::atomic_thread_fence(std::memory_order_seq_cst)</code>.
         */
        [[maybe_unused]] static inline void flush_seq() noexcept {
            std::atomic_thread_fence(std::memory_order_seq_cst);
        }

        /**
         * @brief Remove the shared-memory region and associated mutexes (POSIX only).
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Calls <code>shm_unlink()</code> for the mapped object name.</li>
         *   <li>Then removes both <code>process_mutex&lt;S&gt;</code> and
         *       <code>process_mutex&lt;S + ".loc"&gt;</code>.</li>
         *   <li>Ignores <code>ENOENT</code> if the object does not exist.</li>
         *   <li>Throws <code>std::runtime_error</code> on other errors.</li>
         * </ul>
         *
         * <h4>Idempotency</h4>
         * <p>
         * The operation is idempotent and safe to call multiple times.
         * Once all processes unmap, the region is destroyed by the OS.
         * </p>
         *
         * <h4>Windows</h4>
         * <p>
         * Windows provides no explicit unlink; shared handles are released automatically
         * when all processes close them.
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
