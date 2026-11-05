#include "jh/immutable_str"

#include <string>

void invalid_factory_input(const std::string &text) {
    (void)jh::make_atomic_slot(text);
    (void)jh::make_atomic_slot<false>(text);
}
