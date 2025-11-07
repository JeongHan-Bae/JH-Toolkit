#pragma once

#include "jh/pods/pod_like.h"
#include "jh/pods/array.h"
#include "jh/pods/bits.h"
#include "jh/pods/bytes_view.h"
#include "jh/pods/span.h"
#include "jh/pods/stringify.h"

namespace jh::concepts {
    using jh::pod::pod_like;
    using jh::pod::cv_free_pod_like;

    using jh::pod::max_pod_array_bytes;
    using jh::pod::max_pod_bitflags_bytes;

    using jh::pod::trivial_bytes;
    using jh::pod::detail::linear_container;

    using jh::pod::streamable;
    using jh::pod::streamable_pod;
}
