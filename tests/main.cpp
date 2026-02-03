#define CATCH_CONFIG_RUNNER

#include <catch2/catch_all.hpp>
#include <cstdlib>
#include <vector>
#include <string>

int main(int argc, char* argv[])
{
    Catch::Session session;

    // Args appended only in CI environment
    std::vector<std::string> extra;
    if (std::getenv("CI")) {
        extra.emplace_back("--benchmark-samples=1");
        extra.emplace_back("--benchmark-resamples=0");
        extra.emplace_back("--benchmark-warmup-time=0");
    }

    // Merge argv + extra
    std::vector<const char*> new_argv;
    new_argv.reserve(argc + extra.size());
    new_argv.emplace_back(argv[0]);
    for (int i = 1; i < argc; ++i) new_argv.emplace_back(argv[i]);
    for (auto& e : extra) new_argv.emplace_back(e.c_str());

    session.applyCommandLine((int)new_argv.size(), new_argv.data());
    return session.run();
}
