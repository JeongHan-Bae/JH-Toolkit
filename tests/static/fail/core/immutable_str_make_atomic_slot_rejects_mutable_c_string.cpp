#include "jh/immutable_str"

void invalid_factory_input(char *mutable_buffer) {
    (void)jh::make_atomic_slot(mutable_buffer);
    (void)jh::make_atomic_slot<false>(mutable_buffer);
}
