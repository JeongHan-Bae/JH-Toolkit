#include "jh/serialize_io/value_conversion.h"

#include <string>
#include <string_view>

struct borrowed_record {
    std::string_view view;

    explicit borrowed_record(const std::string &source) : view(source) {}
    explicit operator std::string() const { return std::string{view}; }
};

int main() {
    auto value = jh::serio::deserialize<borrowed_record>(std::string{"temporary"});
    (void)value;
}
