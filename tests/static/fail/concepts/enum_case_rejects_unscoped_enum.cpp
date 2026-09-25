#include "jh/meta"

enum State { idle };

using invalid_case = decltype(jh::meta::enum_case::of<idle>);
