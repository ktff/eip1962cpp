#include "repr.h"

// Calculate a - b - borrow, returning the result and modifying
// the borrow value.
u64 sbb(u64 a, u64 b, u64 &borrow)
{
    Repr<2> ar = {a, 1};
    Repr<2> br = {b, 0};
    Repr<2> borrowr = {borrow, 0};

    auto const tmp = (ar - br) - borrowr;
    if (tmp[1] == 0)
    {
        borrow = 1;
    }
    else
    {
        borrow = 0;
    };

    return tmp[0];
}

// Calculate a + b + carry, returning the sum and modifying the
// carry value.
u64 adc(u64 a, u64 b, u64 &carry)
{
    Repr<2> ar = {a, 0};
    Repr<2> br = {b, 0};
    Repr<2> carryr = {carry, 0};
    auto const tmp = ar + br + carryr;

    carry = tmp[1];

    return tmp[0];
}

bool is_zero(std::vector<u64> const &repr)
{
    for (auto it = repr.cbegin(); it != repr.cend(); it++)
    {
        if (*it != 0)
        {
            return false;
        }
    }

    return true;
}

bool is_odd(std::vector<u64> const &repr)
{
    if (repr.size() == 0)
    {
        return false;
    }

    return (repr[0] & 1) == 1;
}

void div2(std::vector<u64> &repr)
{
    auto t = 0;
    for (auto it = repr.rbegin(); it != repr.rend(); it++)
    {
        auto const t2 = *it << 63;
        *it >>= 1;
        *it |= t;
        t = t2;
    }
}

void sub_noborrow(std::vector<u64> &repr, u64 value)
{
    u64 borrow = 0;

    repr[0] = sbb(repr[0], value, borrow);

    auto it = repr.begin();
    it++; //Skip 0
    for (; it != repr.end(); it++)
    {
        *it = sbb(*it, 0, borrow);
    }
}

void add_nocarry(std::vector<u64> &repr, u64 value)
{
    u64 carry = 0;

    repr[0] = adc(repr[0], value, carry);

    auto it = repr.begin();
    it++; //Skip 0
    for (; it != repr.end(); it++)
    {
        *it = adc(*it, 0, carry);
    }
}

// a >= b
// Where a and b are numbers
bool greater_or_equal(std::vector<u64> const &a, std::vector<u64> const &b)
{
    for (auto i = a.size(); i < b.size(); i++)
    {
        if (b[i] > 0)
        {
            return false;
        }
    }
    for (auto i = b.size(); i < a.size(); i++)
    {
        if (a[i] > 0)
        {
            return true;
        }
    }

    for (i32 i = ((i32)min(a.size(), b.size())) - 1; i >= 0; i--)
    {
        if (a[i] > b[i])
        {
            return true;
        }
        else if (a[i] < b[i])
        {
            return false;
        }
    }

    // Equal
    return true;
}

u32 leading_zero(u64 x)
{
    unsigned n = 0;
    if (x == 0)
        return 64;
    constexpr auto top = u64(1) << 63;
    while (x < top)
    {
        n++;
        x <<= 1;
    }
    return n;
}

u32 num_bits(std::vector<u64> const &repr)
{
    u32 bits = LIMB_BITS * repr.size();
    for (auto it = repr.crbegin(); it != repr.crend(); it++)
    {
        auto const limb = *it;
        if (limb == 0)
            bits -= 64;
        else
        {
            bits -= leading_zero(limb);
            break;
        }
    }

    return bits;
}

void left_shift(std::vector<u64> &repr, u64 shift)
{
    auto const num_libs = repr.size();
    if (num_libs == 0)
    {
        return;
    }

    for (usize i = 0; i < (num_libs - 1); i++)
    {
        repr[i] = (repr[i] >> shift) | (repr[i + 1] << (64 - shift));
    }
    repr[num_libs - 1] = repr[num_libs - 1] >> shift;
}