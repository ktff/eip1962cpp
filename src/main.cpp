#include <iostream>

// #include "extension_towers/fp2.h"
// #include "extension_towers/fp3.h"
#include "extension_towers/fp4.h"
#include "pairings/mnt4.h"
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
    const PrimeField<6> context({0, 0, 0, 0, 0, 372864237846327846});
    const FieldExtension2<6> context2(Fp<6>::from_repr({0, 0, 0, 0, 0, 1243}, context), context);
    auto fp2 = Fp2<6>(Fp<6>::from_repr({0, 0, 0, 0, 0, 34564356}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 235423}, context2), context2);
    const FieldExtension2over2<6> context4(context2);

    std::vector<u64> x, w0, w1;
    // const WeierstrassCurve<Fp<6>> weis1(Fp<6>::from_repr({0, 0, 0, 0, 0, 34564356}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 235423}, context2), {0}, 1);
    const WeierstrassCurve<Fp2<6>> weis2(fp2, fp2, {0}, 1);
    MNT4<6> const mnt4(x, false, w0, w1, false, weis2, fp2);

    std::vector<std::tuple<CurvePoint<Fp<6>>, CurvePoint<Fp2<6>>>> points;
    mnt4.pair(points, context4);

    // const WeierstrassCurve<Fp<6>> weis(Fp<6>::from_repr({0, 0, 0, 0, 0, 34564356}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 235423}, context2), {0}, 1);

    // CurvePoint<Fp<6>> ca(Fp<6>::from_repr({0, 0, 0, 0, 0, 234234}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 124323}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 5467657}, context2));
    // CurvePoint<Fp<6>> cb(Fp<6>::from_repr({0, 0, 0, 0, 0, 123}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 534534}, context2), Fp<6>::from_repr({0, 0, 0, 0, 0, 12423532}, context2));

    // ca.add(cb, weis, context);

    // std::cout
    //     << "Sumed" << std::endl;

    // const FieldExtension3<6> context3(Fp<6>::from_repr({0, 0, 0, 0, 0, 1243}, context), context);
    // const Fp3<6> fp3_a(Fp<6>::from_repr({0, 0, 0, 0, 0, 234234}, context3), Fp<6>::from_repr({0, 0, 0, 0, 0, 124323}, context3), Fp<6>::from_repr({0, 0, 0, 0, 0, 5467657}, context3), context3);
}