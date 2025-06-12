#pragma once

#include "pod_like.h"
#include <ostream>
#include <sstream>
#include <iomanip>
#include "../utils/typed.h"
#include "../utils/base64.h"
#include "pair.h"
#include "array.h"
#include "bits.h"
#include "bytes_view.h"
#include "span.h"
#include "string_view.h"
#include "optional.h"

namespace jh::pod {

    template<typename T>
    concept streamable = requires(std::ostream &os, const T &value) {
        { os << value } -> std::same_as<std::ostream &>;
    };

    template<typename T>
    concept streamable_pod =
    jh::pod::pod_like<T> &&
    streamable<T> &&
    !std::is_fundamental_v<T> &&
    !std::is_enum_v<T> &&
    !std::is_pointer_v<T>;

    template<streamable T, uint16_t N>
    requires(!std::is_same_v<T, char> && // forbid printing char arrays
             requires(std::ostream &os, T v) {
                 { os << v }; // T should be printable
             })
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::array<T, N> &arr) {
        os << "[";
        for (uint16_t i = 0; i < N; ++i) {
            if (i != 0)
                os << ", ";
            os << arr[i];
        }
        os << "]";
        return os;
    }

    template<uint16_t N>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::array<char, N> &str) {
        os << '"';  // start escaped JSON string
        for (uint16_t i = 0; i < N && str[i] != '\0'; ++i) {
            char c = str[i];
            switch (c) {
                case '\"':
                    os << "\\\"";
                    break;
                case '\\':
                    os << "\\\\";
                    break;
                case '\b':
                    os << "\\b";
                    break;
                case '\f':
                    os << "\\f";
                    break;
                case '\n':
                    os << "\\n";
                    break;
                case '\r':
                    os << "\\r";
                    break;
                case '\t':
                    os << "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) > 0x7E) {
                        os << "\\u"
                           << std::hex << std::uppercase
                           << std::setw(4) << std::setfill('0')
                           << static_cast<int>(static_cast<unsigned char>(c))
                           << std::dec << std::nouppercase;
                    } else {
                        os << c;
                    }
            }
        }
        os << '"';  // end escaped JSON string
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, jh::typed::monostate &) {
        os << "null";
        return os;
    }

    template<streamable T1, streamable T2>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::pair<T1, T2> &t) {
        os << "{" << t.first << ", " << t.second << "}";
        return os;
    }

    template<streamable T>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::optional<T> &opt) {
        if (opt.has()) {
            os << opt.ref();
        } else {
            os << "nullopt";
        }
        return os;
    }

    template<std::uint16_t N>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::bitflags<N> &flags) {
        auto bytes = jh::pod::to_bytes(flags);
        std::ios_base::fmtflags fmt = os.flags();

        if ((fmt & std::ios_base::basefield) == std::ios_base::hex) {
            // hex mode
            os << "0x'";
            for (int i = bytes.size() - 1; i >= 0; --i) {
                os << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(bytes[i]);
            }
        } else {
            // binary mode
            os << "0b'";
            for (int i = bytes.size() - 1; i >= 0; --i) {
                for (int b = 7; b >= 0; --b)
                    os << ((bytes[i] >> b) & 1);
            }
        }
        os << "'";

        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const jh::pod::bytes_view bv) {
        os << "base64'";
        const auto encoded = jh::utils::base64::encode(reinterpret_cast<const uint8_t *>(bv.data), bv.len);
        os << encoded;
        os << "'";
        return os;
    }

    template<streamable T>
    inline std::ostream &operator<<(std::ostream &os, const span<T> &sp) {
        os << "span<" << typeid(T).name() << ">[";
        for (std::uint64_t i = 0; i < sp.size(); ++i) {
            if (i != 0) os << ", ";
            os << sp[i];
        }
        os << "]";
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const string_view &sv) {
        os << "string_view\"";
        const auto buffer = std::string_view{sv.data, sv.len};
        os << buffer;
        os << "\"";
        return os;
    }

    template<streamable_pod Pod>
    std::string to_string(const Pod& p) {
        std::ostringstream oss;
        oss << p;
        return oss.str();
    }
}

namespace jh::typed{
    inline std::ostream &operator<<(std::ostream &os, const jh::typed::monostate &) {
        os << "null";
        return os;
    }
}