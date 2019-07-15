#ifndef H_FP4
#define H_FP4

#include "../common.h"
#include "../element.h"
#include "fp2.h"
#include "fp3.h"
#include "fpM2.h"
#include "../field.h"

using namespace cbn::literals;

template <usize N>
class FieldExtension2over2 : public FieldExtension2<N>
{
public:
    std::array<Fp<N>, 4> frobenius_coeffs_c1;

    FieldExtension2over2(FieldExtension2<N> const &field) : FieldExtension2<N>(field), frobenius_coeffs_c1({Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field)})
    {
        // NON_REDISUE**(((q^0) - 1) / 4)
        auto const f_0 = Fp<N>::one(field);

        // NON_REDISUE**(((q^1) - 1) / 4)
        auto const f_1 = calc_frobenius_factor(field.non_residue(), field.mod(), 4, "Fp4");

        // NON_REDISUE**(((q^2) - 1) / 4)
        auto const f_2 = calc_frobenius_factor(field.non_residue(), field.mod() * field.mod(), 4, "Fp4");

        auto const f_3 = Fp<N>::zero(field);

        std::array<Fp<N>, 4> calc_frobenius_coeffs_c1 = {f_0, f_1, f_2, f_3};
        frobenius_coeffs_c1 = calc_frobenius_coeffs_c1;
    }

    void mul_by_nonresidue(Fp2<N> &el) const
    {
        // IMPORTANT: This only works cause the structure of extension field for Fp6
        // is w^2 - u = 0!
        // take an element in Fp4 as 2 over 2 and multiplity
        // (c0 + c1 * u)*u with u^2 - xi = 0 -> (c1*xi + c0 * u)
        auto e0 = el.c1;
        el.c1 = el.c0;
        FieldExtension2<N>::mul_by_nonresidue(e0);
        el.c0 = e0;
    }

    void frobenius_map(FpM2<Fp2<N>, FieldExtension2over2<N>, N> &fp, usize power) const
    {
        if (power != 1 && power != 2)
        {
            unreachable(stringf("can not reach power %u", power));
        }
        fp.c0.frobenius_map(power);
        fp.c1.frobenius_map(power);
        fp.c1.mul_by_fp(frobenius_coeffs_c1[power % 4]);
    }
};

template <usize N>
using Fp4 = FpM2<Fp2<N>, FieldExtension2over2<N>, N>;
#endif
