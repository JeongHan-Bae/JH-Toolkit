#include <memory>
#include "jh/generator"

using invalid = jh::async::generator_range<std::unique_ptr<int>>;
