#include "jh/immutable_str.h"

#include <cstring>      // for std::strlen, std::memcpy, std::strcmp
#include <cctype>       // for std::isspace
#include <algorithm>    // for std::max

namespace jh {

    immutable_str::immutable_str(const char* str) {
        init_from_string(str);
    }

    const char* immutable_str::c_str() const noexcept {
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

    bool immutable_str::operator==(const immutable_str& other) const noexcept {
        return std::strcmp(data_.get(), other.data_.get()) == 0;
    }

    std::uint64_t immutable_str::hash() const noexcept {
        return std::hash<std::string_view>{}(std::string_view(data_.get(), size_));
    }

    void immutable_str::init_from_string(const char* input_str) {
        if (!input_str) {
            // Initialize an empty string if input is null
            size_ = 0;
            data_ = std::make_unique<char[]>(1);
            data_[0] = '\0';
            return;
        }

        const char* start = input_str;
        const char* end = input_str + std::strlen(input_str) - 1;

        // If auto_trim is enabled, remove leading and trailing whitespace
        if (auto_trim) {
            while (*start && std::isspace(static_cast<unsigned char>(*start))) {
                ++start;
            }

            // If the input contains only whitespace, treat it as an empty string
            if (*start == '\0') {
                size_ = 0;
                data_ = std::make_unique<char[]>(1);
                data_[0] = '\0';
                return;
            }

            while (end > start && std::isspace(static_cast<unsigned char>(*end))) {
                --end;
            }
        }

        // Compute the final string size
        size_ = end - start + 1;

        // Allocate memory and copy the string
        data_ = std::make_unique<char[]>(size_ + 1);
        std::memcpy(data_.get(), start, size_);
        data_[size_] = '\0';
    }

    std::uint64_t atomic_str_hash::operator()(const std::shared_ptr<immutable_str>& ptr) const noexcept {
        return ptr->hash();
    }

    bool atomic_str_eq::operator()(const atomic_str_ptr& lhs, const atomic_str_ptr& rhs) const noexcept {
        return *lhs == *rhs;
    }

} // namespace jh
