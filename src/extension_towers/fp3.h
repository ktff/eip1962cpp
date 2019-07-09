#ifndef H_FP3
#define H_FP3

#include "../common.h"
#include "../fp.h"

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
class Fp3
{
};

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
class Extension3
{
public:
    Extension3(Fp<E, F> non_residue)
    {
        // TODO
    }

    bool calculate_frobenius_coeffs(
        BigUint modulus)
    {
        // TODO
    }
};

#endif