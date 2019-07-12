#ifndef H_MULTIEXP
#define H_MULTIEXP

#include "weierstrass/curve.h"
#include "common.h"

template <class E>
CurvePoint<E> peepinger(std::vector<std::tuple<CurvePoint<E>, std::vector<u64>>> pairs, WeierstrassCurve<E> const &curve)
{
    u32 c;
    if (pairs.size() < 32)
    {
        c = 3;
    }
    else
    {
        c = ceil(log((double)pairs.size()));
    };

    std::vector<CurvePoint<E>> windows;
    std::vector<CurvePoint<E>> buckets;

    u64 mask = (u64(1) << c) - u64(1);
    auto cur = 0;
    auto const num_bits = num_bits(curve.subgroup_order());
    auto const zero_point = CurvePoint<E>::zero(curve);

    while (cur <= num_bits)
    {
        auto acc = zero_point;

        buckets.resize(0);
        buckets.resize((1 << c) - 1, zero_point);

        for (auto it = pairs.begin(); it != pairs.end(); it++)
        {
            CurvePoint<E> const &g = std::get<0>(*it);
            std::vector<u64> &s = std::get<1>(*it);
            usize const index = s[0] & mask;

            if (index != 0)
            {
                buckets[index - 1].add_mixed(g);
            }

            left_shift(s, c);
        }

        auto running_sum = zero_point;
        for (auto it = buckets.crbegin(); it != buckets.crend(); it++)
        {
            running_sum.add(*it);
            acc.add(running_sum);
        }

        windows.push_back(acc);

        cur += c;
    }

    auto acc = zero_point;

    for (auto it = windows.crbegin(); it != windows.crend(); it++)
    {
        for (auto i = 0; i < c; i++)
        {
            acc.mul2();
        }

        acc.add_assign(*it);
    }

    return acc;
}

#endif
