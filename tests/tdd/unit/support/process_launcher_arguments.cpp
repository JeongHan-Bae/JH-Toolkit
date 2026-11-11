#include <string_view>

int main(const int argc, const char *const argv[]) {
    if (argc != 4) return 1;
    if (argv[1] != nullptr &&
        argv[2] != nullptr &&
        argv[3] != nullptr &&
        std::string_view{argv[1]} == "alpha" &&
        std::string_view{argv[2]} == "two words" &&
        std::string_view{argv[3]}.empty()) {
        return 0;
    }
    return 2;
}
