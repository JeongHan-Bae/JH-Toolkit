#include "jh/runtime_arr"

int main()
{
    jh::runtime_arr<bool> bits(8);
    return static_cast<int>(bits.data() != nullptr);
}
