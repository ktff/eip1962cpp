#ifndef H_FP4
#define H_FP4

#include "../common.h"
#include "../element.h"
#include "fp2.h"
#include "fp3.h"
#include "../field.h"

using namespace cbn::literals;

template <usize N>
class FieldExtension2over2 : public FieldExtension2<N>
{

public:
    FieldExtension2over2(FieldExtension2<N> const &field) : FieldExtension2<N>(field) {}

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
};

template <usize N>
class Fp4 : public Element<Fp4<N>>
{
    FieldExtension2over2<N> const &field;
    std::array<Fp<N>, 4> frobenius_coeffs_c1;

public:
    Fp2<N> c0, c1;

    Fp4(Fp2<N> c0, Fp2<N> c1, FieldExtension2over2<N> const &field) : field(field), frobenius_coeffs_c1({Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field), Fp<N>::zero(field)}), c0(c0), c1(c1)
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

    auto operator=(Fp4<N> const &other)
    {
        c0 = other.c0;
        c1 = other.c1;
    }

    Fp4<N> cyclotomic_exp(std::vector<u64> const &exp) const
    {
        auto res = one();
        auto self_inverse = *this;
        self_inverse.conjugate();

        auto found_nonzero = false;
        auto const naf = into_ternary_wnaf(exp);

        for (auto it = naf.crbegin(); it != naf.crend(); it++)
        {
            auto const value = *it;
            if (found_nonzero)
            {
                res.square();
            }

            if (value != 0)
            {
                found_nonzero = true;

                if (value > 0)
                {
                    res.mul(*this);
                }
                else
                {
                    res.mul(self_inverse);
                }
            }
        }

        return res;
    }

    void conjugate()
    {
        c1.negate();
    }

    // ************************* ELEMENT impl ********************************* //

    template <class C>
    static Fp4<N> one(C const &context)
    {
        FieldExtension2over2<N> const &field = context;
        return Fp4<N>(Fp2<N>::one(context), Fp2<N>::zero(context), field);
    }

    template <class C>
    static Fp4<N> zero(C const &context)
    {
        FieldExtension2over2<N> const &field = context;
        return Fp4<N>(Fp2<N>::zero(context), Fp2<N>::zero(context), field);
    }

    Fp4<N> one() const
    {
        return Fp4::one(field);
    }

    Fp4<N> zero() const
    {
        return Fp4::zero(field);
    }

    Fp4<N> &self()
    {
        return *this;
    }

    Fp4<N> const &self() const
    {
        return *this;
    }

    void serialize(u8 mod_byte_len, std::vector<u8> &data) const
    {
        c0.serialize(mod_byte_len, data);
        c1.serialize(mod_byte_len, data);
    }

    Option<Fp4<N>> inverse() const
    {
        if (is_zero())
        {
            return {};
        }
        else
        {
            // Guide to Pairing-based Cryptography, Algorithm 5.19.
            // v0 = c0.square()
            auto v0 = c0;
            v0.square();
            // v1 = c1.square()
            auto v1 = c1;
            v1.square();
            // v0 = v0 - beta * v1
            auto v1_by_nonresidue = v1;
            field.mul_by_nonresidue(v1_by_nonresidue);
            v0.sub(v1_by_nonresidue);
            auto o = v0.inverse();
            if (o)
            {
                auto v1 = o.value();
                auto e0 = c0;
                e0.mul(v1);
                auto e1 = c1;
                e1.mul(v1);
                e1.negate();

                return Fp4(e0, e1, field);
            }
            else
            {
                return {};
            }
        }
    }

    void square()
    {

        auto ab_add = c0;
        ab_add.add(c1);
        auto ab_mul = c0;
        ab_mul.mul(c1);

        auto t0 = c1;
        field.mul_by_nonresidue(t0);
        t0.add(c0);

        auto t1 = ab_mul;
        field.mul_by_nonresidue(t1);

        auto e0 = ab_add;
        e0.mul(t0);
        e0.sub(ab_mul);
        e0.sub(t1);

        auto e1 = ab_mul;
        e1.mul2();

        c0 = e0;
        c1 = e1;
    }

    void mul2()
    {
        c0.mul2();
        c1.mul2();
    }

    void mul(Fp4<N> const &other)
    {
        auto const a0 = c0;
        auto const b0 = c1;
        auto const a1 = other.c0;
        auto const b1 = other.c1;

        auto a0a1 = a0;
        a0a1.mul(a1);
        auto b0b1 = b0;
        b0b1.mul(b1);
        auto t0 = b0b1;
        field.mul_by_nonresidue(t0);

        auto e0 = a0a1;
        e0.add(t0);
        auto e1 = a0;
        e1.add(b0);

        auto t1 = a1;
        t1.add(b1);

        e1.mul(t1);
        e1.sub(a0a1);
        e1.sub(b0b1);

        c0 = e0;
        c1 = e1;
    }

    void sub(Fp4<N> const &e)
    {
        c0.sub(e.c0);
        c1.sub(e.c1);
    }

    void add(Fp4<N> const &e)
    {
        c0.add(e.c0);
        c1.add(e.c1);
    }

    void negate()
    {
        c0.negate();
        c1.negate();
    }

    void frobenius_map(usize power)
    {
        if (power != 1 && power != 2)
        {
            unreachable(stringf("can not reach power %u", power));
        }
        c0.frobenius_map(power);
        c1.frobenius_map(power);
        c1.mul_by_fp(field.frobenius_coeffs_c1[power % 4]);
    }

    bool is_zero() const
    {
        return c0.is_zero() && c1.is_zero();
    }

    bool operator==(Fp4<N> const &other) const
    {
        return c0 == other.c0 && c1 == other.c1;
    }

    bool operator!=(Fp4<N> const &other) const
    {
        return !(*this == other);
    }
};

#endif
