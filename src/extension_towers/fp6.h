#ifndef H_FP6
#define H_FP6

#include "../common.h"
#include "../element.h"
#include "fp2.h"
#include "fp3.h"
#include "fpM2.h"
#include "../field.h"

using namespace cbn::literals;

template <usize N>
class FieldExtension2over3 : public FieldExtension3<N>
{
public:
    std::array<Fp<N>, 6> frobenius_coeffs_c1;

    FieldExtension2over3(FieldExtension3<N> const &field) : FieldExtension3<N>(field), frobenius_coeffs_c1({Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field)})
    {
        // NON_REDISUE**(((q^0) - 1) / 6)
        auto const f_0 = Fp<N>::one(field);

        // NON_REDISUE**(((q^1) - 1) / 6)
        auto q_power = field.mod();
        auto const f_1 = calc_frobenius_factor(field.non_residue(), q_power, 6, "Fp6");

        // NON_REDISUE**(((q^2) - 1) / 6)
        q_power = q_power * field.mod();
        auto const f_2 = Fp<N>::zero(field);

        // NON_REDISUE**(((q^3) - 1) / 6)
        q_power = q_power * field.mod();
        auto const f_3 = calc_frobenius_factor(field.non_residue(), q_power, 6, "Fp6");

        auto const f_4 = Fp<N>::zero(field);
        auto const f_5 = Fp<N>::zero(field);

        std::array<Fp<N>, 6> calc_frobenius_coeffs_c1 = {f_0, f_1, f_2, f_3, f_4, f_5};
        frobenius_coeffs_c1 = calc_frobenius_coeffs_c1;
    }

    void mul_by_nonresidue(Fp3<N> &el) const
    {
        // IMPORTANT: This only works cause the structure of extension field for Fp6
        // is w^2 - u = 0!
        // take an element in Fp6 as 2 over 3 and mutplity
        // (c0 + c1 * u)*u with u^2 - xi = 0 -> (c1*xi + c0 * u)
        auto c0 = el.c2;
        el.c2 = el.c1;
        el.c1 = el.c0;
        FieldExtension3<N>::mul_by_nonresidue(c0);
        el.c0 = c0;
    }

    void frobenius_map(FpM2<Fp3<N>, FieldExtension2over3<N>, N> &fp, usize power) const
    {
        if (power != 1 && power != 3)
        {
            unreachable(stringf("can not reach power %u", power));
        }
        fp.c0.frobenius_map(power);
        fp.c1.frobenius_map(power);
        fp.c1.mul_by_fp(frobenius_coeffs_c1[power % 6]);
    }
};

template <usize N>
using Fp6 = FpM2<Fp3<N>, FieldExtension2over3<N>, N>;
#endif
