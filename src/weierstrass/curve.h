#ifndef H_WEIS_CURVE
#define H_WEIS_CURVE

#include "../common.h"

enum CurveType
{
    Generic,
    AIsMinus3,
    AIsZero,
    BIsZero,
};

// E: Element
template <class E>
class WeierstrassCurve
{
    CurveType cty;
    E curve_a;
    E curve_b;

public:
    E const &get_curve_a() const
    {
        return curve_a;
    }

    CurveType get_type() const
    {
        return cty;
    }
};

// E: Element
template <class E>
class CurvePoint
{
    E x;
    E y;
    E z;

public:
    auto operator=(CurvePoint<E> other)
    {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
    }

    // auto operator=(CurvePoint<E> const &other)
    // {
    //     this->x = other.x;
    //     this->y = other.y;
    //     this->z = other.z;
    // }

private:
    void mul2_generic(E const &curve_a)
    {
        if (this->is_zero())
        {
            return;
        }

        // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#doubling-dbl-2007-bl

        // A = X1^2
        auto a = x;
        a.square();

        // B = Y1^2
        auto b = y;
        b.square();

        // C = B^2 = Y1^4
        auto c = b;
        c.square();

        auto z_2 = z;
        z_2.square();

        // D = 2*((X1+B)2-A-C)
        auto d = x;
        d.add(b);
        d.square();
        d.sub(a);
        d.sub(c);
        d.mul2();

        // E = 3*A + curve_a*z^4
        auto e = a;
        e.mul2();
        e.add(a);

        // curve_a*z^4
        auto a_z_4 = z_2;
        a_z_4.square();
        a_z_4.mul(curve_a);

        e.add(a_z_4);

        // T = D^2
        auto t = d;
        t.mul2();

        // F = E^2 - 2*D
        auto f = e;
        f.square();
        f.sub(t);

        this->x = f;

        // Z3 = (Y1+Z1)^2-B-Z^2
        this->z.add(this->y);
        this->z.square();
        this->z.sub(b);
        this->z.sub(z_2);

        // Y3 = E*(D-X3)-8*C
        this->y = d;
        this->y.sub(this->x);
        this->y.mul(e);
        c.mul2();
        c.mul2();
        c.mul2();
        this->y.sub(c);
    }

    void mul2_a_is_zero()
    {
        if (this->is_zero())
        {
            return;
        }

        // Other than the point at infinity, no points on E or E'
        // can mul2 to equal the point at infinity, as y=0 is
        // never true for points on the curve.

        // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#doubling-dbl-2009-l

        // A = X1^2
        auto a = this->x;
        a.square();

        // B = Y1^2
        auto b = this->y;
        b.square();

        // C = B^2
        auto c = b;
        c.square();

        // D = 2*((X1+B)2-A-C)
        auto d = this->x;
        d.add(b);
        d.square();
        d.sub(a);
        d.sub(c);
        d.mul2();

        // E = 3*A
        auto e = a;
        e.mul2();
        e.add(a);

        // F = E^2
        auto f = e;
        f.square();

        // Z3 = 2*Y1*Z1
        this->z.mul(this->y);
        this->z.mul2();

        // X3 = F-2*D
        this->x = f;
        this->x.sub(d);
        this->x.sub(d);

        // Y3 = E*(D-X3)-8*C
        this->y = d;
        this->y.sub(this->x);
        this->y.mul(e);
        c.mul2();
        c.mul2();
        c.mul2();
        this->y.sub(c);
    }

    bool
    is_zero() const
    {
        return z.is_zero();
    }

    void mul2(CurveType cty, E const &curve_a)
    {
        switch (cty)
        {
        case CurveType::Generic:
            this->mul2_generic(curve_a);
            break;

        case CurveType::AIsZero:
            this->mul2_a_is_zero();
            break;

        default:
            unimplemented();
        }
    }

public:
    template <class C>
    void add(CurvePoint<E> const &b, WeierstrassCurve<E> const &wc, C const &context)
    {
        if (this->is_zero())
        {
            *this = b;
            return;
        }
        else if (b.is_zero())
        {
            return;
        }

        if (b.z == E::one(context))
        {
            this->add_mixed(b, wc, context);
            return;
        }

        // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#addition-add-2007-bl

        // Z1Z1 = Z1^2
        auto z1z1 = this->z;
        z1z1.square();

        // Z2Z2 = Z2^2
        auto z2z2 = b.z;
        z2z2.square();

        // U1 = X1*Z2Z2
        auto u1 = this->x;
        u1.mul(z2z2);

        // U2 = X2*Z1Z1
        auto u2 = b.x;
        u2.mul(z1z1);

        // S1 = Y1*Z2*Z2Z2
        auto s1 = this->y;
        s1.mul(b.z);
        s1.mul(z2z2);

        // S2 = Y2*Z1*Z1Z1
        auto s2 = b.y;
        s2.mul(this->z);
        s2.mul(z1z1);

        if (u1 == u2 && s1 == s2)
        {
            // The two points are equal, so we mul2.
            this->mul2(wc.get_type(), wc.get_curve_a());
        }
        else
        {
            // If we're adding -a and a together, this->z becomes zero as H becomes zero.
            if (u1 == u2)
            {
                this->x = E::zero(context);
                this->y = E::one(context);
                this->z = E::zero(context);
                return;
            }

            // H = U2-U1
            auto h = u2;
            h.sub(u1);

            // I = (2*H)^2
            auto i = h;
            i.mul2();
            i.square();

            // J = H*I
            auto j = h;
            j.mul(i);

            // r = 2*(S2-S1)
            auto r = s2;
            r.sub(s1);
            r.mul2();

            // V = U1*I
            auto v = u1;
            v.mul(i);

            // X3 = r^2 - J - 2*V
            this->x = r;
            this->x.square();
            this->x.sub(j);
            this->x.sub(v);
            this->x.sub(v);

            // Y3 = r*(V - X3) - 2*S1*J
            this->y = v;
            this->y.sub(this->x);
            this->y.mul(r);
            s1.mul(j); // S1 = S1 * J * 2
            s1.mul2();
            this->y.sub(s1);

            // Z3 = ((Z1+Z2)^2 - Z1Z1 - Z2Z2)*H
            this->z.add(b.z);
            this->z.square();
            this->z.sub(z1z1);
            this->z.sub(z2z2);
            this->z.mul(h);
        }
    }

private:
    template <class C>
    void add_mixed(CurvePoint<E> const &b, WeierstrassCurve<E> const &wc, C const &context)
    {
        if (b.is_zero())
        {
            return;
        }

        if (this->is_zero())
        {
            *this = b;
            return;
        }

        if (b.z != E::one(context))
        {
            this->add(b, wc, context);
            return;
        }

        // http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#addition-madd-2007-bl

        // Z1Z1 = Z1^2
        auto z1z1 = this->z;
        z1z1.square();

        // U2 = X2*Z1Z1
        auto u2 = b.x;
        u2.mul(z1z1);

        // S2 = Y2*Z1*Z1Z1
        auto s2 = b.y;
        s2.mul(this->z);
        s2.mul(z1z1);

        if (this->x == u2 && this->y == s2)
        {
            // The two points are equal, so we mul2.
            this->mul2(wc.get_type(), wc.get_curve_a());
        }
        else
        {
            // If we're adding -a and a together, this->z becomes zero as H becomes zero.

            // H = U2-X1
            auto h = u2;
            h.sub(this->x);

            // HH = H^2
            auto hh = h;
            hh.square();

            // I = 4*HH
            auto i = hh;
            i.mul2();
            i.mul2();

            // J = H*I
            auto j = h;
            j.mul(i);

            // r = 2*(S2-Y1)
            auto r = s2;
            r.sub(this->y);
            r.mul2();

            // V = X1*I
            auto v = this->x;
            v.mul(i);

            // X3 = r^2 - J - 2*V
            this->x = r;
            this->x.square();
            this->x.sub(j);
            this->x.sub(v);
            this->x.sub(v);

            // Y3 = r*(V-X3)-2*Y1*J
            j.mul(this->y); // J = 2*Y1*J
            j.mul2();
            this->y = v;
            this->y.sub(this->x);
            this->y.mul(r);
            this->y.sub(j);

            // Z3 = (Z1+H)^2-Z1Z1-HH
            this->z.add(h);
            this->z.square();
            this->z.sub(z1z1);
            this->z.sub(hh);
        }
    }
};

#endif
