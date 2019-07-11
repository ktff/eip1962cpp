#ifndef H_FP2
#define H_FP2

#include "../types.h"
#include "../element.h"
#include "../fp.h"
#include "../field.h"

// using namespace cbn;
using namespace cbn::literals;

template <usize N>
class FieldExtension2 : public PrimeField<N>
{
    Fp<N> _non_residue;

public:
    FieldExtension2(Fp<N> non_residue, PrimeField<N> const &field) : PrimeField<N>(field), _non_residue(non_residue) {}

    auto non_residue() const
    {
        return _non_residue;
    }
};

template <usize N>
class Fp2 : public Element<Fp2<N>>
{
    FieldExtension2<N> const &field;
    Fp<N> c0, c1;

public:
    Fp2(Fp<N> c0, Fp<N> c1, FieldExtension2<N> const &field) : c0(c0), c1(c1), field(field) {}

    auto operator=(Fp2<N> const &other)
    {
        c0 = other.c0;
        c1 = other.c1;
    }

    template <class C>
    static Fp2<N> one(C const &context)
    {
        FieldExtension2<N> const &field = context;
        return Fp2<N>(Fp<N>::one(context), Fp<N>::zero(context), field);
    }

    template <class C>
    static Fp2<N> zero(C const &context)
    {
        FieldExtension2<N> const &field = context;
        return Fp2<N>(Fp<N>::zero(context), Fp<N>::zero(context), field);
    }

    Fp2<N> one() const
    {
        return Fp2::one(field);
    }

    Fp2<N> zero() const
    {
        return Fp2::zero(field);
    }

    Fp2<N> &self()
    {
        return *this;
    }

    Fp2<N> const &self() const
    {
        return *this;
    }

    Option<Fp2<N>> inverse() const
    {
        unimplemented();
    }

    void square()
    {
        // v0 = c0 - c1
        auto v0 = c0;
        v0.sub(c1);
        // v3 = c0 - beta * c1
        auto v3 = c0;
        auto t0 = c1;
        t0.mul(field.non_residue());
        v3.sub(t0);
        // v2 = c0 * c1
        auto v2 = c0;
        v2.mul(c1);

        // v0 = (v0 * v3) + v2
        v0.mul(v3);
        v0.add(v2);

        c1 = v2;
        c1.mul2();
        c0 = v0;
        v2.mul(field.non_residue());
        c0.add(v2);
    }

    void mul2()
    {
        c0.mul2();
        c1.mul2();
    }

    void mul(Fp2<N> const &other)
    {
        auto v0 = c0;
        v0.mul(other.c0);
        auto v1 = c1;
        v1.mul(other.c1);

        c1.add(c0);
        auto t0 = other.c0;
        t0.add(other.c1);
        c1.mul(t0);
        c1.sub(v0);
        c1.sub(v1);
        c0 = v0;
        v1.mul(field.non_residue());
        c0.add(v1);
    }

    void sub(Fp2<N> const &e)
    {
        c0.sub(e.c0);
        c1.sub(e.c1);
    }

    void add(Fp2<N> const &e)
    {
        c0.add(e.c0);
        c1.add(e.c1);
    }

    bool is_zero() const
    {
        return c0.is_zero() && c1.is_zero();
    }

    bool operator==(Fp2<N> const &other) const
    {
        return c0 == other.c0 && c1 == other.c1;
    }

    bool operator!=(Fp2<N> const &other) const
    {
        return !(*this == other);
    }

private:
};

#endif