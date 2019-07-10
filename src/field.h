#ifndef H_FIELD
#define H_FIELD

#include "types.h"
#include "repr.h"

template <usize N>
class PrimeField
{
    // u64 mont_power;
    // u64 modulus_bits;
    Repr<N> modulus;
    Repr<N> mont_r_;
    // Repr<N> mont_r2;
    u64 mont_inv_;

public:
    
    Repr<N> mod() const
    {
        return modulus;
    }

    Repr<N> mont_r() const
    {
        return mont_r_;
    }

    // Montgomery parametare for multiplication
    u64 mont_inv() const
    {
        return mont_inv_;
    }
};

#endif