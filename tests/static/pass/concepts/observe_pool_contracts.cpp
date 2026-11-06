#include "jh/pool"

#include <memory>
#include <functional>
#include <shared_mutex>
#include <type_traits>
#include <utility>

struct immovable_observed_value {
    explicit immovable_observed_value(int value) : value(value) {}
    immovable_observed_value(const immovable_observed_value &) = delete;
    immovable_observed_value(immovable_observed_value &&) = delete;
    int value;
    std::size_t hash() const noexcept { return static_cast<std::size_t>(value); }
    bool operator==(const immovable_observed_value &other) const noexcept {
        return value == other.value;
    }
};

using observed_pool_type = jh::observe_pool<immovable_observed_value>;
using observed_handle = observed_pool_type::ptr;
static_assert(!std::is_copy_constructible_v<immovable_observed_value>);
static_assert(!std::is_move_constructible_v<immovable_observed_value>);
static_assert(std::is_same_v<observed_pool_type,
    jh::conc::pointer_pool<immovable_observed_value, jh::hash<immovable_observed_value>,
                           std::equal_to<immovable_observed_value>, std::shared_mutex>>);
static_assert(std::is_same_v<decltype(std::declval<observed_pool_type &>().acquire(1)), observed_handle>);
static_assert(std::is_copy_constructible_v<observed_handle>);
static_assert(std::is_move_constructible_v<observed_handle>);
static_assert(std::is_same_v<decltype(*std::declval<observed_handle &>()), immovable_observed_value &>);
static_assert(!std::is_convertible_v<observed_handle, std::shared_ptr<immovable_observed_value>>);
