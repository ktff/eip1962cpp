#ifndef H_D_BIG_UNIT
#define H_D_BIG_UNIT

#include "types.h"
#include "slice.h"

class BigUint
{
    // Will do for now
    Vec<u8> vec;

public:
    // TODO
    BigUint(u32 num)
    {
        Vec<u8> v;

        // TODO
        vec = v;
    }

    BigUint(Slice<u8> raw) : vec(raw.to_vec())
    {
        // TODO
    }

    BigUint clone()
    {
        //TODO
    }
    // BigUint &operator=(const BigUint &rhs){};

    bool is_zero()
    {
        //TODO
    }

    bool is_even()
    {
        //TODO
    }

    bool less_than(u8 data)
    {
        // TODO
    }

    usize bits()
    {
        // TODO
    }

    BigUint &operator>>(usize pos)
    {
        //TODO
    }

    BigUint &operator-(BigUint other)
    {
        //TODO
    }
};

#endif