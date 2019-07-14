#ifndef H_WNAF
#define H_WNAF

#include "../common.h"
#include "../repr.h"

std::vector<i64> into_ternary_wnaf(std::vector<u64> const &repr)
{
    std::vector<i64> res;

    if (repr.size() == 0)
    {
        return res;
    }

    std::vector<u64> e = repr;

    constexpr u64 WINDOW = u64(1);
    constexpr u64 MIDPOINT = u64(1) << WINDOW;
    constexpr i64 MIDPOINT_I64 = MIDPOINT;
    constexpr u64 MASK = u64(1) << (WINDOW + 1);

    while (!is_zero(e))
    {
        i64 z = 0;
        if (is_odd(e))
        {
            z = MIDPOINT_I64 - (i64(e[0] % MASK));
            if (z >= 0)
            {
                sub_noborrow(e, z);
            }
            else
            {
                add_nocarry(e, (-z));
            }
        }
        res.push_back(z);
        div2(e);
    }

    return res;
}

#endif