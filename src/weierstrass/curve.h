#ifndef H_W_CURVE
#define H_W_CURVE

#include "../common.h"

// C: CurveParameters<P>
template <class C, class P>
class WeierstrassCurve
{
public:
    C &params;
};

// C: CurveParameters<P>
template <class C, class P>
class CurvePoint
{

    static CurvePoint<C, P> point_from_xy(
        WeierstrassCurve<C, P> &curve,
        P x,
        P y)
    {
        //TODO
    }

    Tuple<P, P> into_xy()
    {
        //TODO
    }
};

#endif