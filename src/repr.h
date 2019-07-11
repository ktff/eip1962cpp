#ifndef H_REPR
#define H_REPR

#include "types.h"
#include "ctbignum/ctbignum.hpp"

const static usize LIMB_BITS = sizeof(u64) * 8;

template <usize N>
using Repr = cbn::big_int<N>;

namespace cbn
{
template <usize N>
bool is_even(Repr<N> const &repr)
{
    return (repr[0] & 0x1) == 0;
}

template <usize N>
Repr<N> mul2(Repr<N> repr)
{
    return cbn::detail::first<N>(cbn::shift_left(repr, 1));
}

template <usize N>
Repr<N> div2(Repr<N> repr)
{
    return cbn::shift_right(repr, 1);
}

template <usize N>
bool is_zero(Repr<N> const &repr)
{
    constexpr Repr<N> zero = {0};
    return repr == zero;
}
} // namespace cbn

template <usize N>
class RevBitIterator
{
    Repr<N> const &repr;
    usize at;

public:
    RevBitIterator(Repr<N> const &repr) : repr(repr), at(N * LIMB_BITS) {}

    bool get() const
    {
        auto i = at / LIMB_BITS;
        auto off = at - (i * LIMB_BITS);
        return (repr[i] >> off) & 0x1;
    }

    /// True if moved
    bool before()
    {
        if (at > 0)
        {
            at--;
            return true;
        }
        else
        {
            return false;
        }
    }
};

#endif