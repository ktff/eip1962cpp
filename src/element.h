#ifndef H_ELEMENT
#define H_ELEMENT

#include "repr.h"

template <class E>
class Element
{
public:
    template <class C>
    static E one(C const &context);

    template <class C>
    static E zero(C const &context);

    virtual E one() const = 0;

    virtual E zero() const = 0;

    virtual E &self() = 0;

    virtual E const &self() const = 0;

    template <usize N>
    E pow(Repr<N> const &e) const
    {
        auto res = this->one();
        auto found_one = false;

        for (auto it = RevBitIterator(e); it.before();)
        {
            auto i = it.get();
            if (found_one)
            {
                res.square();
            }
            else
            {
                found_one = i;
            }

            if (i)
            {
                res.mul(self());
            }
        }

        return res;
    }

    // Computes the multiplicative inverse of this element, if nonzero.
    virtual Option<E> inverse() const = 0;

    virtual void square() = 0;

    virtual void mul2() = 0;

    virtual void mul(E const &e) = 0;

    virtual void sub(E const &e) = 0;

    virtual void add(E const &e) = 0;

    virtual bool is_zero() const = 0;

    virtual bool operator==(E const &other) const = 0;

    virtual bool operator!=(E const &other) const = 0;
};

#endif