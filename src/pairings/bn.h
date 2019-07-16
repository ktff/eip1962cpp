#ifndef H_BN
#define H_BN

#include "../common.h"
#include "../curve.h"
#include "../fp.h"
#include "../extension_towers/fp2.h"
#include "../extension_towers/fp6_3.h"
#include "../extension_towers/fp12.h"

template <usize N>
using ThreePoint = std::tuple<Fp2<N>, Fp2<N>, Fp2<N>>;

template <usize N>
class BNengine
{
    std::vector<u64> u;
    std::vector<u64> six_u_plus_2;
    bool u_is_negative;
    TwistType twist_type;
    WeierstrassCurve<Fp2<N>> const &curve_twist;
    Fp2<N> non_residue_in_p_minus_one_over_2;

public:
    BNengine(std::vector<u64> u,
             std::vector<u64> six_u_plus_2,
             bool u_is_negative,
             TwistType twist_type,
             WeierstrassCurve<Fp2<N>> const &curve_twist,
             Fp2<N> non_residue_in_p_minus_one_over_2) : u(u), six_u_plus_2(six_u_plus_2), u_is_negative(u_is_negative), twist_type(twist_type), curve_twist(curve_twist), non_residue_in_p_minus_one_over_2(non_residue_in_p_minus_one_over_2) {}

    template <class C>
    std::optional<Fp12<N>>
    pair(std::vector<std::tuple<CurvePoint<Fp<N>>, CurvePoint<Fp2<N>>>> const &points, C const &context) const
    {
        if (points.size() == 0)
        {
            return {};
        }
        auto res = miller_loop(points, context);
        return final_exponentiation(res);
    }

private:
    template <class C>
    Fp12<N> miller_loop(std::vector<std::tuple<CurvePoint<Fp<N>>, CurvePoint<Fp2<N>>>> const &points, C const &context) const
    {

        std::vector<CurvePoint<Fp<N>>> g1_references;
        std::vector<std::vector<ThreePoint<N>>> prepared_coeffs;

        for (auto it = points.cbegin(); it != points.cend(); it++)
        {
            auto const p = std::get<0>(*it);
            auto const q = std::get<1>(*it);
            if (!p.is_zero() && !q.is_zero())
            {
                auto coeffs = prepare(q, context);
                prepared_coeffs.push_back(coeffs);
                g1_references.push_back(p);
            }
        }

        auto const n = prepared_coeffs.size();

        std::vector<usize> pc_indexes;
        pc_indexes.resize(n);

        auto f = Fp12<N>::one(context);
        auto it = RevBitIterator(six_u_plus_2);
        it.before(); //skip 1
        for (; it.before();)
        {
            auto const i = it.get();
            f.square();

            for_ell(f, n, g1_references, prepared_coeffs, pc_indexes);

            if (i)
            {
                for_ell(f, n, g1_references, prepared_coeffs, pc_indexes);
            }
        }

        if (u_is_negative)
        {
            f.conjugate();
        }

        for_ell(f, n, g1_references, prepared_coeffs, pc_indexes);
        for_ell(f, n, g1_references, prepared_coeffs, pc_indexes);

        for (auto j = 0; j < n; j++)
        {
            assert(pc_indexes[j] == prepared_coeffs[j].size());
        }

        return f;
    }

    template <class C>
    std::vector<ThreePoint<N>> prepare(CurvePoint<Fp2<N>> const &twist_point, C const &context) const
    {
        assert(twist_point.is_normalized());

        auto two_inv_ = Fp<N>::one(context);
        two_inv_.mul2();
        auto const two_inv = two_inv_.inverse().value();

        std::vector<ThreePoint<N>> ell_coeffs;

        if (twist_point.is_zero())
        {
            return ell_coeffs;
        }

        auto r = CurvePoint<Fp2<N>>(twist_point.get_x(), twist_point.get_y());

        auto it = RevBitIterator(six_u_plus_2);
        it.before(); //skip 1
        for (; it.before();)
        {
            ell_coeffs.push_back(doubling_step(r, two_inv));

            if (it.get())
            {
                ell_coeffs.push_back(addition_step(r, twist_point));
            }
        }

        if (u_is_negative)
        {
            r.negate();
        }

        auto q = twist_point;

        q.x.c1.negate();
        FieldExtension3over2<N> const &field_3_2 = context;
        q.x.mul(field_3_2.frobenius_coeffs_c1[1]);

        q.y.c1.negate();
        q.y.mul(non_residue_in_p_minus_one_over_2);

        ell_coeffs.push_back(addition_step(r, q));

        auto minusq2 = twist_point;
        minusq2.x.mul(field_3_2.frobenius_coeffs_c1[2]);

        ell_coeffs.push_back(addition_step(r, minusq2));

        return ell_coeffs;
    }

    ThreePoint<N> doubling_step(
        CurvePoint<Fp2<N>> &r,
        Fp<N> const &two_inv) const
    {
        // Use adapted formulas from ZEXE instead

        // X*Y/2
        auto a = r.x;
        a.mul(r.y);
        a.mul_by_fp(two_inv);

        // Y^2
        auto b = r.y;
        b.square();

        // Z^2
        auto c = r.z;
        c.square();

        auto e = curve_twist.get_b();

        // 3*Z^2
        auto t0 = c;
        t0.mul2();
        t0.add(c);

        // 3*b*Z^2
        e.mul(t0);

        // 9*b*Z^2
        auto f = e;
        f.mul2();
        f.add(e);

        // (Y^2 + 9*b*Z^2)/2
        auto g = b;
        g.add(f);
        g.mul_by_fp(two_inv);

        // (Y + Z)^2
        auto h = r.y;
        h.add(r.z);
        h.square();

        // (Y^2 + Z^2)
        auto t1 = b;
        t1.add(c);

        // 2*Y*Z
        h.sub(t1);

        // 3*b*Z^2 - Y^2
        auto i = e;
        i.sub(b);

        // X^2
        auto j = r.x;
        j.square();

        // (3*b*Z^2)^2
        auto e_square = e;
        e_square.square();

        // X = (Y^2 - 9*b*Z^2)*X*Y/2
        r.x = b;
        r.x.sub(f);
        r.x.mul(a);

        // 27*b^2*Z^4
        auto e_square_by_3 = e_square;
        e_square_by_3.mul2();
        e_square_by_3.add(e_square);

        // Y = ((Y^2 + 9*b*Z^2)/2)^2 - 27*b^2*Z^4
        r.y = g;
        r.y.square();
        r.y.sub(e_square_by_3);

        // Z = 2*Y^3*Z
        r.z = b;
        r.z.mul(h);

        // 3*X^2
        auto j_by_three = j;
        j_by_three.mul2();
        j_by_three.add(j);

        // - 2*Y*Z
        h.negate();

        // i.mul_by_nonresidue(this->fp6_extension);
        switch (twist_type)
        {
        case M:
            return std::tuple(i, j_by_three, h);
        case D: // (0, 3, 4) = (-2*Y*Z, 3*X^2, 3*b*Z^2 - Y^2)
            return std::tuple(h, j_by_three, i);
        }
    }

    ThreePoint<N> addition_step(
        CurvePoint<Fp2<N>> &r,
        CurvePoint<Fp2<N>> const &q) const
    {
        assert(q.is_normalized());
        // use adapted zexe formulas too instead of ones from pairing crate
        // Capitals are coors of R (homogenious), normals are coordinates of Q (affine)
        // Y - y*Z
        auto theta = q.y;
        theta.mul(r.z);
        theta.negate();
        theta.add(r.y);

        // X - x*Z
        auto lambda = q.x;
        lambda.mul(r.z);
        lambda.negate();
        lambda.add(r.x);

        // Theta^2
        auto c = theta;
        c.square();

        // Lambda^2
        auto d = lambda;
        d.square();

        // Lambda^3
        auto e = lambda;
        e.mul(d);

        // Theta^2 * Z
        auto f = r.z;
        f.mul(c);

        // Lambda^2 * X
        auto g = r.x;
        g.mul(d);

        // Lambda^3 + Theta^2 * Z - 2*Lambda^2 * X
        auto h = g;
        h.mul2();
        h.negate();
        h.add(e);
        h.add(f);

        r.x = lambda;
        r.x.mul(h);

        // (Lambda^2 * X - H)*Theta
        auto t0 = g;
        t0.sub(h);
        t0.mul(theta);

        // Y = (Lambda^2 * X - H)*Theta - Lambda^3 * Y
        r.y.mul(e);
        r.y.negate();
        r.y.add(t0);

        // Z = Lambda^3 * Z
        r.z.mul(e);

        // Lambda*y
        auto t1 = lambda;
        t1.mul(q.y);

        // Theta*x - Lambda*y
        auto j = theta;
        j.mul(q.x);
        j.sub(t1);

        theta.negate();

        // lambda.negate();
        // j.mul_by_nonresidue(this->fp6_extension);
        switch (twist_type)
        {
        case M:
            return std::tuple(j, theta, lambda);
        case D: // (0, 3, 4) = (lambda, -theta, Theta*x - Lambda*y)
            return std::tuple(lambda, theta, j);
        }
    }

    void for_ell(Fp12<N> &f, usize n, std::vector<CurvePoint<Fp<N>>> const &g1_references, std::vector<std::vector<ThreePoint<N>>> const &prepared_coeffs, std::vector<usize> &pc_indexes) const
    {
        for (auto j = 0; j < n; j++)
        {
            auto const p = g1_references[j];
            auto const coeffs = prepared_coeffs[j][pc_indexes[j]];
            pc_indexes[j]++;
            ell(f, coeffs, p);
        }
    }

    void ell(
        Fp12<N> &f,
        ThreePoint<N> const &coeffs,
        CurvePoint<Fp<N>> const &p) const
    {
        assert(p.is_normalized());
        auto c0 = std::get<0>(coeffs);
        auto c1 = std::get<1>(coeffs);
        auto c2 = std::get<2>(coeffs);

        switch (twist_type)
        {
        case M:
        {
            c2.mul_by_fp(p.y);
            c1.mul_by_fp(p.x);
            f.mul_by_014(c0, c1, c2);
            break;
        }
        case D:
        {
            c0.mul_by_fp(p.y);
            c1.mul_by_fp(p.x);
            f.mul_by_034(c0, c1, c2);
            break;
        }
        }
    }

    std::optional<Fp12<N>> final_exponentiation(Fp12<N> const &f) const
    {
        // use Zexe and pairing crate fused
        // https://eprint.iacr.org/2012/232.pdf

        // f1 = r.conjugate() = f^(p^6)
        auto f1 = f;
        f1.frobenius_map(6);

        if (auto of2 = f.inverse())
        {
            auto f2 = of2.value();
            auto r = f1;
            r.mul(f2);

            f2 = r;

            r.frobenius_map(2);
            r.mul(f2);

            auto fp = r;
            fp.frobenius_map(1);

            auto fp2 = r;
            fp2.frobenius_map(2);
            auto fp3 = fp2;
            fp3.frobenius_map(1);

            auto fu = r;
            this->exp_by_x(fu);
            // exp_by_x(fu, x);

            auto fu2 = fu;
            this->exp_by_x(fu2);
            // exp_by_x(fu2, x);

            auto fu3 = fu2;
            this->exp_by_x(fu3);
            // exp_by_x(fu3, x);

            auto y3 = fu;
            y3.frobenius_map(1);

            auto fu2p = fu2;
            fu2p.frobenius_map(1);

            auto fu3p = fu3;
            fu3p.frobenius_map(1);

            auto y2 = fu2;
            y2.frobenius_map(2);

            auto y0 = fp;
            y0.mul(fp2);
            y0.mul(fp3);

            auto y1 = r;
            y1.conjugate();

            auto y5 = fu2;
            y5.conjugate();

            y3.conjugate();

            auto y4 = fu;
            y4.mul(fu2p);
            y4.conjugate();

            auto y6 = fu3;
            y6.mul(fu3p);
            y6.conjugate();

            y6.square();
            y6.mul(y4);
            y6.mul(y5);

            auto t1 = y3;
            t1.mul(y5);
            t1.mul(y6);

            y6.mul(y2);

            t1.square();
            t1.mul(y6);
            t1.square();

            auto t0 = t1;
            t0.mul(y1);

            t1.mul(y0);

            t0.square();
            t0.mul(t1);

            return t0;
        }
        else
        {
            return {};
        }
    }

    void exp_by_x(Fp12<N> &f) const
    {
        f = f.cyclotomic_exp(this->u);
        if (u_is_negative)
        {
            f.conjugate();
        }
    }
};

#endif
