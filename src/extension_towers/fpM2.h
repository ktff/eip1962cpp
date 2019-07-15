#ifndef H_FPM2
#define H_FPM2

#include "../common.h"
#include "../element.h"
#include "fp2.h"
#include "fp3.h"
#include "../field.h"

using namespace cbn::literals;

template <class F, class E, usize N>
class FpM2 : public Element<FpM2<F, E, N>>
{

public:
    E const &field;
    F c0, c1;

    FpM2(F c0, F c1, E const &field) : field(field), c0(c0), c1(c1)
    {
    }

    auto operator=(FpM2<F, E, N> const &other)
    {
        c0 = other.c0;
        c1 = other.c1;
    }

    FpM2<F, E, N> cyclotomic_exp(std::vector<u64> const &exp) const
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

    void frobenius_map(usize power)
    {
        field.frobenius_map(*this, power);
    }

    // ************************* ELEMENT impl ********************************* //

    template <class C>
    static FpM2<F, E, N> one(C const &context)
    {
        E const &field = context;
        return FpM2<F, E, N>(F::one(context), F::zero(context), field);
    }

    template <class C>
    static FpM2<F, E, N> zero(C const &context)
    {
        E const &field = context;
        return FpM2<F, E, N>(F::zero(context), F::zero(context), field);
    }

    FpM2<F, E, N> one() const
    {
        return FpM2::one(field);
    }

    FpM2<F, E, N> zero() const
    {
        return FpM2::zero(field);
    }

    FpM2<F, E, N> &self()
    {
        return *this;
    }

    FpM2<F, E, N> const &self() const
    {
        return *this;
    }

    void serialize(u8 mod_byte_len, std::vector<u8> &data) const
    {
        c0.serialize(mod_byte_len, data);
        c1.serialize(mod_byte_len, data);
    }

    Option<FpM2<F, E, N>> inverse() const
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

                return FpM2(e0, e1, field);
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

    void mul(FpM2<F, E, N> const &other)
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

    void sub(FpM2<F, E, N> const &e)
    {
        c0.sub(e.c0);
        c1.sub(e.c1);
    }

    void add(FpM2<F, E, N> const &e)
    {
        c0.add(e.c0);
        c1.add(e.c1);
    }

    void negate()
    {
        c0.negate();
        c1.negate();
    }

    bool is_zero() const
    {
        return c0.is_zero() && c1.is_zero();
    }

    bool operator==(FpM2<F, E, N> const &other) const
    {
        return c0 == other.c0 && c1 == other.c1;
    }

    bool operator!=(FpM2<F, E, N> const &other) const
    {
        return !(*this == other);
    }
};

#endif
