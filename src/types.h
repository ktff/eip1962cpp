#ifndef H_TYPE
#define H_TYPE

#include <variant>
#include <optional>
#include <vector>
#include <cstdint>
#include <string>
#include <tuple>
#include <string>

using namespace std;

// C++ version of Rust u8
typedef std::uint8_t u8;

// C++ version of Rust u16
typedef std::uint16_t u16;

// C++ version of Rust u32
typedef std::uint32_t u32;

// C++ version of Rust u64
typedef std::uint64_t u64;

// C++ version of Rust usize
typedef std::size_t usize;

// C++ shortcut for Rust Result
template <class T, class R>
using Result = std::variant<T, R>;

// C++ shortcut for Rust Option
template <class T>
using Option = std::optional<T>;

// C++ shortcut for Rust Vec
template <class T>
using Vec = std::vector<T>;

// C++ shortcut for Rust Tuples
template <class... Types>
using Tuple = std::tuple<Types...>;

// template <class... Types>
// Tuple<Types...> tuple(Types &&... args)
// {
//     return std::make_tuple(args...);
// }

// TODO: with c++20 transition to char8_t
// C++ shortcut for Rust String
typedef std::basic_string<char> String;

// C++ equivalent of num-bigint BigUint
// Where Rust's version stores integer in u32 form, but computes in u64 form.
// While C++ version computes with stored format, that's why this with u32.
// // TODO: for now is Slice
// template <usize N>
// using BigUint = cbn::big_int<N, u32>;

#define TRY(y, x)                \
                                 \
    auto _try_##y = x;           \
    if (_try_##y.index() == 1)   \
    {                            \
        return get<1>(_try_##y); \
    }                            \
    auto y = get<0>(_try_##y);

#define TRY_TIE(z, y, x)       \
    {                          \
        auto a = x;            \
        if (a.index() == 1)    \
        {                      \
            return get<1>(a);  \
        }                      \
        tie(z, y) = get<0>(a); \
    }

#define TRY_OR(y, x, e)        \
                               \
    auto _try_##y = x;         \
    if (_try_##y.index() == 1) \
    {                          \
        return e;              \
    }                          \
    auto y = get<0>(_try_##y);

// String str_to_owned(const char *s)
// {

// }

// ERRORS included here becouse of recursive dependecy

#endif