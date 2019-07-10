#include <iostream>
#include "weierstrass/curve.h"
#include "fp.h"
#include "element.h"
#include "field.h"
// #include ".h"
// #include "public_interface/api.h"
// #include "representation.h"
// #include "api.h"
// #include "err"

int main()
{
    PrimeField<6> context({0, 0, 0, 0, 0, 76});
    Fp<6> a({0, 0, 0, 0, 0, 23}, context), b({0, 0, 0, 0, 0, 34}, context);
    WeierstrassCurve<Fp<6>> weis(a, b);
    CurvePoint<Fp<6>> ca(Fp<6>({0, 0, 0, 0, 0, 234234}, context), Fp<6>({0, 0, 0, 0, 0, 124323}, context), Fp<6>({0, 0, 0, 0, 0, 5467657}, context));
    CurvePoint<Fp<6>> cb(Fp<6>({0, 0, 0, 0, 0, 123}, context), Fp<6>({0, 0, 0, 0, 0, 534534}, context), Fp<6>({0, 0, 0, 0, 0, 12423532}, context));

    ca.add(cb, weis, context);

    std::cout
        << "Builded" << std::endl;
}