#include <iostream>
#include "weierstrass/curve.h"
#include "fp.h"
#include "element.h"
#include "field.h"
#include "extension_towers/fp2.h"
#include "extension_towers/fp3.h"
#include "api.h"

// #include ".h"
// #include "public_interface/api.h"
// #include "representation.h"
// #include "api.h"
// #include "err"

int main()
{
    // Api
    std::vector<u8> input = {0, 1, 2, 3, 4};
    auto result = run(input);
    if (auto err = std::get_if<1>(&result))
    {
        std::cout
            << "Err: " << *err << std::endl;
    }
    else
    {
        std::cout
            << "Ok" << std::endl;
    }

    // // Contextes must be valid for the whole scope
    // const PrimeField<6> context({0, 0, 0, 0, 0, 372864237846327846});
    // const FieldExtension2<6> context2(Fp<6>::from_repr({0, 0, 0, 0, 0, 1243}, context), context);
    // const WeierstrassCurve<Fp<6>> weis(Fp<6>::from_repr({0, 0, 0, 0, 0, 34564356}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 235423}, context2), {0}, 1);

    // CurvePoint<Fp<6>> ca(Fp<6>::from_repr({0, 0, 0, 0, 0, 234234}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 124323}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 5467657}, context2));
    // CurvePoint<Fp<6>> cb(Fp<6>::from_repr({0, 0, 0, 0, 0, 123}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 534534}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 12423532}, context2));

    // ca.add(cb, weis, context);

    // std::cout
    //     << "Sumed" << std::endl;

    // const FieldExtension3<6> context3(Fp<6>::from_repr({0, 0, 0, 0, 0, 1243}, context), context);
    // const Fp3<6> fp3_a(Fp<6>::from_repr({0, 0, 0, 0, 0, 234234}, context3), Fp<6>::from_repr({0, 0, 0, 0, 0, 124323}, context3), Fp<6>::from_repr({0, 0, 0, 0, 0, 5467657}, context3), context3);
}