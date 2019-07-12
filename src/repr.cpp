#include "repr.h"

// a >= b
// Where a and b are numbers
bool greater_or_equal_dyn(std::vector<u64> const &a, std::vector<u64> const &b)
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