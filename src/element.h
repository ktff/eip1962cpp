#ifndef H_ELEMENT
#define H_ELEMENT

template <class E>
class Element
{
public:
    template <class C>
    static E one(C const &context);

    template <class C>
    static E zero(C const &context);

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