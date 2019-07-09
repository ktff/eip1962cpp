#ifndef H_FIELD
#define H_FIELD

#include "common.h"

// E: ElementRepr
template <class E>
class PrimeField
{
};

// E: ElementRepr
template <class E>
Result<PrimeField<E>, void> field_from_modulus(BigUint modulus)
{
    // TODO
}

Vec<u64> biguint_to_u64_vec(BigUint v)
{
    // TODO
}

#endif