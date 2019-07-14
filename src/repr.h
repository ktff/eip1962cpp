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

// *********************** FUNCTIONS on u64 ******************* //

// Calculate a - b - borrow, returning the result and modifying
// the borrow value.
u64 sbb(u64 a, u64 b, u64 &borrow);

// Calculate a + b + carry, returning the sum and modifying the
// carry value.
u64 adc(u64 a, u64 b, u64 &carry);

// // Calculate a + (b * c) + carry, returning the least significant digit
// // and setting carry to the most significant digit.
// u64 mac_with_carry(u64 a,u64 b,u64 c,u64 &carry)  {
//     auto const tmp = (u128::from(a)) + u128::from(b) * u128::from(c) + u128::from(*carry);

//     *carry = (tmp >> 64) as u64;

//     return tmp;
// }

// *********************** FUNCTIONS on std::vector<u64> ******************* //

bool is_zero(std::vector<u64> const &repr);

bool is_odd(std::vector<u64> const &repr);

void div2(std::vector<u64> &repr);

void sub_noborrow(std::vector<u64> &repr, u64 value);

void add_nocarry(std::vector<u64> &repr, u64 value);

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
    // Skips higher zeros
    RevBitIterator(A const &repr) : repr(repr), at(repr.size() * LIMB_BITS)
    {
        skip_zeros();
    }

    void skip_zeros()
    {
        while (this->before())
        {
            if (this->get())
            {
                at++;
                break;
            }
        }
    }

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