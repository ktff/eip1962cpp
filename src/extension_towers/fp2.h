#ifndef H_FP2
#define H_FP2

#include "../common.h"
#include "../fp.h"

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
class Fp2
{
};

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
class Extension2
{
public:
    Extension2(Fp<E, F> non_residue)
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