/**
* Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
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
 */


#include "jh/immutable_str.h"

#include <cstring>      // for std::strlen, std::memcpy, std::strcmp
#include <cctype>       // for std::isspace
#include <algorithm>    // for std::max

namespace jh {
    immutable_str::immutable_str(const char *str) {
        init_from_string(str);
    }

    immutable_str::immutable_str(const std::string_view sv, std::mutex &mtx) {
        std::lock_guard lock(mtx);
        if (std::strlen(sv.data()) != sv.size()) {
            throw std::logic_error("jh::immutable_str does not support string views containing embedded null characters.");
        }
        init_from_string(sv.data());
    }

    const char *immutable_str::c_str() const noexcept {
        return data_.get();
    }

    std::string immutable_str::str() const {
        return {data_.get(), size_};
    }

    std::string_view immutable_str::view() const noexcept {
        return {data_.get(), size_};
    }

    uint64_t immutable_str::size() const noexcept {
        return size_;
    }

    bool immutable_str::operator==(const immutable_str &other) const noexcept {
        return std::strcmp(data_.get(), other.data_.get()) == 0;
    }

    std::uint64_t immutable_str::hash() const noexcept {
        std::call_once(hash_flag_, [this] {
            hash_.emplace(std::hash<std::string_view>{}(std::string_view(data_.get(), size_)));
        });
        return hash_.value();
    }

    void immutable_str::init_from_string(const char *input_str) {
        if (!input_str) {
            // Initialize an empty string if input is null
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        const char *end = input_str + std::strlen(input_str) - 1;

        // If auto_trim is enabled, remove leading and trailing whitespace
        if (auto_trim.load()) {
            while (*start && std::isspace(static_cast<unsigned char>(*start))) {
                ++start;
            }

            // If the input contains only whitespace, treat it as an empty string
            if (*start == '\0') {
                size_ = 0;
                auto data_array_ = std::make_unique<char[]>(1);
                data_array_[0] = '\0';
                data_ = std::move(data_array_);
                return;
            }

            while (end > start && std::isspace(static_cast<unsigned char>(*end))) {
                --end;
            }
        }

        // Compute the final string size
        size_ = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size_ + 1);
        std::memcpy(data_array_.get(), start, size_);
        data_array_[size_] = '\0';
        data_ = std::move(data_array_);
    }

    std::uint64_t atomic_str_hash::operator()(const std::shared_ptr<immutable_str> &ptr) const noexcept {
        return ptr ? ptr->hash() : 0;
    }

    bool atomic_str_eq::operator()(const atomic_str_ptr &lhs, const atomic_str_ptr &rhs) const noexcept {
        if (!lhs || !rhs) return false;
        return *lhs == *rhs;
    }
} // namespace jh
