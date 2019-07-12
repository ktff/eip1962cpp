#ifndef H_REPR
#define H_REPR

#include "common.h"
#include "ctbignum/ctbignum.hpp"

const static usize LIMB_BITS = sizeof(u64) * 8;

template <usize N>
using Repr = cbn::big_int<N>;

// *********************** FUNCTIONS on Repr ******************* //
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

// *********************** FUNCTIONS on std::vector<u64> ******************* //
// a >= b
// Where a and b are numbers
bool greater_or_equal(std::vector<u64> const &a, std::vector<u64> const &b);

// Minimal number of bits necessary to represent number in repr
u32 num_bits(std::vector<u64> const &repr);

void left_shift(std::vector<u64> &repr, u64 shift);

// ********************** HELPER CLASSES ******************* //
// A is required to have to methods: size() and operator[]
template <class A>
class RevBitIterator
{
    A const &repr;
    usize at;

public:
    RevBitIterator(A const &repr) : repr(repr), at(repr.size() * LIMB_BITS) {}

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