#include <iostream>
#include <string_view>
#include "jh/async"
#include "jh/immutable_str"
#include "jh/serio"


void hello_async() {
    auto make_fib = [](std::string_view& sv, std::ostream& out) -> jh::async::fiber {
        for (const auto c : sv) {
            out << c;
            co_await jh::async::resume_tag;
        }
    };
    std::string_view sv_fib = "Hello, Async Fiber!\n";
    auto fib = make_fib(sv_fib, std::cout);
    while (!fib.done()) {
        fib.resume();
    }
    auto make_gen = [](std::string_view& sv) -> jh::async::generator<char> {
        for (const auto c : sv) {
            co_yield c;
        }
    };
    std::string_view sv_gen = "Hello, Async Generator!\n";
    auto gen = make_gen(sv_gen);
    while (gen.next()) {
        std::cout << gen.value().value();
    }
}

void hello_immutable_str() {
    auto pool = jh::observe_pool<jh::immutable_str>();
    const auto str = pool.acquire("Hello, Immutable String!\n");
    std::cout << str->view() << std::endl;
}

void hello_serio() {
    using jh::serio::huff_algo;
    using HUF = jh::serio::huffman<"demo", huff_algo::huff256_canonical>;

    std::ostringstream out_stream(std::ios::binary);
    HUF::compress(out_stream, "Hello, Serialization IO!\n");

    const std::string binary = out_stream.str();

    std::istringstream in_stream(binary, std::ios::binary);
    std::string recovered = HUF::decompress(in_stream);
    std::cout << recovered << std::endl;
}

int main() {
    hello_async();
    hello_immutable_str();
    hello_serio();
    return 0;
}