#include <iostream>
#include "weierstrass/curve.h"
#include "fp.h"
#include "element.h"
#include "field.h"
#include "extension_towers/fp2.h"
// #include ".h"
// #include "public_interface/api.h"
// #include "representation.h"
// #include "api.h"
// #include "err"

int main()
{
    // Contextes must be valid for the whole scope
    const PrimeField<6> context({0, 0, 0, 0, 0, 76});
    const FieldExtension2<6> context2(Fp<6>({0, 0, 0, 0, 0, 1243}, context), context);
    const WeierstrassCurve<Fp<6>> weis(Fp<6>({0, 0, 0, 0, 0, 34564356}, context2), Fp<6>({0, 0, 0, 0, 0, 235423}, context2));

    CurvePoint<Fp<6>> ca(Fp<6>({0, 0, 0, 0, 0, 234234}, context2), Fp<6>({0, 0, 0, 0, 0, 124323}, context2), Fp<6>({0, 0, 0, 0, 0, 5467657}, context2));
    CurvePoint<Fp<6>> cb(Fp<6>({0, 0, 0, 0, 0, 123}, context2), Fp<6>({0, 0, 0, 0, 0, 534534}, context2), Fp<6>({0, 0, 0, 0, 0, 12423532}, context2));

    ca.add(cb, weis, context);

    std::cout
        << "Sumed" << std::endl;
}