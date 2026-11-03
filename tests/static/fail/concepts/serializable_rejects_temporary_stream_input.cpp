#include "jh/serialize_io/value_conversion.h"

#include <string>

int main() {
    int value = 0;
    jh::serio::deserialize(std::string{"42"}, value);
}
