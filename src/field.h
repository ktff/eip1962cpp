#ifndef H_FIELD
#define H_FIELD

#include "types.h"
#include "repr.h"
#include "ctbignum/slicing.hpp"

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
    PrimeField(Repr<N> modulus) : modulus(modulus)
    {
        // Compute -m^-1 mod 2**64 by exponentiating by totient(2**64) - 1
        u64 inv = 1;
        for (auto i = 0; i < 63; i++)
        {
            inv = inv * inv;
            inv = inv * modulus[0];
        }
        inv = (std::numeric_limits<u64>::max() - inv) + 2 + std::numeric_limits<u64>::max();
        mont_inv_ = inv;

        if ((modulus[N - 1] >> (LIMB_BITS - 1)) > 0 || modulus[N - 1] == 0)
        {
            // Rust version returns Err here so should this function.
            unimplemented();
        }
        Repr<N + 1> pow_N_LIMB_BITS = {0};
        pow_N_LIMB_BITS[N] = 1;
        mont_r_ = pow_N_LIMB_BITS % modulus;
    }

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