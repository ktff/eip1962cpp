#ifndef H_FROBENIUS
#define H_FROBENIUS

#include "../common.h"
#include "../element.h"
#include "../fp.h"
#include "../field.h"

template <usize N, usize M>
Fp<N> calc_frobenius_factor(Fp<N> const &non_residue, Repr<M> const &base, usize div, std::string const &err)
{
    // NON_RESIDUE**(((base) - 1) / div)
    auto const q_power = base;
    constexpr Repr<N> one = {1};
    auto power = q_power - one;
    Repr<N> const rdiv = {div};
    if (!cbn::is_zero(power % rdiv))
    {
        unknown_parameter_err("Failed to calculate Frobenius coeffs for " + err);
    }
    power = power / rdiv;
    auto const f = non_residue.pow(power);
    return f;
}

#endif