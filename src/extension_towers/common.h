#ifndef H_EX_COMMON
#define H_EX_COMMON

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
bool is_non_square(
    Fp<E, F> &element,
    Slice<u64> modulus_minus_one_by_2)
{
    // TODO
}

#endif