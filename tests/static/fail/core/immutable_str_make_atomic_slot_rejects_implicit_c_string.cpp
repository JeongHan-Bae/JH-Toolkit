#include "jh/immutable_str"

struct implicit_c_string {
    operator const char *() const { return "text"; }
};

void invalid_factory_input(const implicit_c_string &text) {
    (void)jh::make_atomic_slot(text);
    (void)jh::make_atomic_slot<false>(text);
}
