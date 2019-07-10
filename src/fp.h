#ifndef H_FP
#define H_FP

#include "types.h"
#include "element.h"
#include "repr.h"
#include "field.h"

// using namespace cbn;
using namespace cbn::literals;

template <usize N>
class Fp : Element<Fp<N>>
{
    PrimeField<N> const &field;
    Repr<N> repr;

public:
    Fp(Repr<N> repr, PrimeField<N> const &field) : repr(repr), field(field) {}

    Fp(Fp<N> const &other) : Fp(other.repr, other.field) {}

    auto operator=(Fp<N> const &other)
    {
        this->repr = other.repr;
    }

    ~Fp() {}
    // ************** ELEMENT impl ************** //
    template <class C>
    static Fp<N> one(C const &context)
    {
        PrimeField<N> const &field = context;
        return Fp(field.mont_r(), field);
    }

    template <class C>
    static Fp<N> zero(C const &context)
    {
        constexpr Repr<N> zero = {0};
        PrimeField<N> const &field = context;
        return Fp(zero, field);
    }

    void square()
    {
        repr = cbn::montgomery_mul(repr, repr, field.mod(), field.mont_inv());
    }

    void mul2()
    {
        repr = cbn::shift_left(repr, 1) % field.mod();
    }

    void mul(Fp<N> const &e)
    {
        repr = cbn::montgomery_mul(repr, e.repr, field.mod(), field.mont_inv());
    }

    void sub(Fp<N> const &e)
    {
        repr = cbn::mod_sub(repr, e.repr, field.mod());
    }

    void add(Fp<N> const &e)
    {
        repr = cbn::mod_add(repr, e.repr, field.mod());
    }

    bool is_zero() const
    {
        constexpr auto zero = cbn::to_big_int(0_Z);
        return this->repr == zero;
    }

    bool operator==(Fp<N> const &other) const
    {
        return repr == other.repr;
    }

    bool operator!=(Fp<N> const &other) const
    {
        return repr != other.repr;
    }

    // *************** impl ************ //
};

#endif