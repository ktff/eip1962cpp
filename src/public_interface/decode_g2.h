#ifndef H_DECODE_G1
#define H_DECODE_G1

#include "../common.h"
#include "decode_utils.h"
#include "../field.h"
#include "../extension_towers/fp3.h"
#include "../extension_towers/fp2.h"
#include "decode_fp.h"
#include "../weierstrass/curve.h"
#include "../weierstrass/common.h"
#include "../fp.h"

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Extension2<E, F>, Slice<u8>>, ApiError> create_fp2_extension(
    Slice<u8> bytes,
    BigUint modulus,
    usize field_byte_len,
    F &base_field)
{
    TRY(splitted_degree, split(bytes, EXTENSION_DEGREE_ENCODING_LENGTH, "Input is not long enough to get extension degree"));
    auto extension_degree = get<0>(splitted_degree);
    auto rest_0 = get<1>(splitted_degree);
    if (extension_degree[0] != EXTENSION_DEGREE_2)
    {
        return ApiError(ApiErrorKind::UnknownParameter, "Extension degree expected to be 2");
    }

    TRY(decoded_fp, decode_fp(rest_0, field_byte_len, base_field));
    auto fp_non_residue = get<0>(decoded_fp);
    auto rest_1 = get<1>(decoded_fp);
    if (fp_non_residue.is_zero())
    {
        return ApiError(ApiErrorKind::UnexpectedZero, "Fp2 non-residue can not be zero");
    }

    {
        auto modulus_minus_one_by_2 = modulus.clone() - BigUint(1);
        modulus_minus_one_by_2 = modulus_minus_one_by_2 >> 1;
        auto not_a_square = is_non_square(&fp_non_residue, biguint_to_u64_vec(modulus_minus_one_by_2));
        if (!not_a_square)
        {
            return ApiError(ApiErrorKind::InputError, "Non-residue for Fp2 is actually a residue");
        }
    }

    Extension2 extension_2(fp_non_residue);
    if (!extension_2.calculate_frobenius_coeffs(modulus))
    {
        return ApiError(ApiErrorKind::UnknownParameter, "Failed to calculate Frobenius coeffs for Fp2");
    }

    return tuple(extension_2, rest_1);
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Extension3<E, F>, Slice<u8>>, ApiError> create_fp3_extension(
    Slice<u8> bytes,
    BigUint modulus,
    usize field_byte_len,
    F &base_field)
{
    TRY(splitted_degree, split(bytes, EXTENSION_DEGREE_ENCODING_LENGTH, "Input is not long enough to get extension degree"));
    auto extension_degree = get<0>(splitted_degree);
    auto rest_0 = get<1>(splitted_degree);
    if (extension_degree[0] != EXTENSION_DEGREE_3)
    {
        return ApiError(ApiErrorKind::UnknownParameter, "Extension degree expected to be 3");
    }

    TRY(decoded_fp, decode_fp(rest_0, field_byte_len, base_field));
    auto fp_non_residue = get<0>(decoded_fp);
    auto rest_1 = get<1>(decoded_fp);
    if (fp_non_residue.is_zero())
    {
        return ApiError(ApiErrorKind::UnexpectedZero, "Fp3 non-residue can not be zero");
    }

    Extension3 extension_3(fp_non_residue);
    if (!extension_3.calculate_frobenius_coeffs(modulus))
    {
        return ApiError(ApiErrorKind::UnknownParameter, "Failed to calculate Frobenius coeffs for Fp3");
    }

    return tuple(extension_3, rest_1);
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
// C: CurveParameters<BaseFieldElement = Fp2<E, F>>
template <class E, class F, class C>
Result<Tuple<CurvePoint<C, Fp2<E, F>>, Slice<u8>>, ApiError> decode_g2_point_from_xy_in_fp2(
    Slice<u8> bytes,
    usize field_byte_len,
    WeierstrassCurve<C, Fp2<E, F>> &curve)
{
    TRY(decoded_x, decode_fp2(bytes, field_byte_len, curve.params.params()));
    auto x = get<0>(decoded_x);
    auto rest_x = get<1>(decoded_x);

    TRY(decoded_y, decode_fp2(rest_x, field_byte_len, curve.params.params()));
    auto y = get<0>(decoded_y);
    auto rest = get<1>(decoded_y);

    auto p = CurvePoint<C, Fp2<E, F>>::point_from_xy(&curve, x, y);

    return tuple(p, rest);
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
// C: CurveParameters<BaseFieldElement = Fp3<E, F>>
template <class E, class F, class C>
Result<Tuple<CurvePoint<C, Fp3<E, F>>, Slice<u8>>, ApiError> decode_g2_point_from_xy_in_fp3(
    Slice<u8> bytes,
    usize field_byte_len,
    WeierstrassCurve<C, Fp3<E, F>> &curve)
{
    TRY(decoded_x, decode_fp3(bytes, field_byte_len, curve.params.params()));
    auto x = get<0>(decoded_x);
    auto rest_x = get<1>(decoded_x);

    TRY(decoded_y, decode_fp3(rest_x, field_byte_len, curve.params.params()));
    auto y = get<0>(decoded_y);
    auto rest = get<1>(decoded_y);

    auto p = CurvePoint<E, Fp3<E, F>>::point_from_xy(&curve, x, y);

    return tuple(p, rest);
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
// C: CurveParameters<BaseFieldElement = Fp2<E, F>>
template <class E, class F, class C>
Result<Vec<u8>, ApiError> serialize_g2_point_in_fp2(
    usize modulus_len,
    CurvePoint<C, Fp2<E, F>> &point)
{
    auto xy = point.into_xy();
    TRY(a, serialize_fp2_fixed_len(modulus_len, &get<0>(xy)));
    TRY(b, serialize_fp2_fixed_len(modulus_len, &get<1>(xy)));

    a.insert(a.end(), b.begin(), b.end());

    return a;
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
// C: CurveParameters<BaseFieldElement = Fp3<E, F>>
template <class E, class F, class C>
Result<Vec<u8>, ApiError> serialize_g2_point_in_fp3(
    usize modulus_len,
    CurvePoint<C, Fp3<E, F>> &point)
{
    auto xy = point.into_xy();
    TRY(a, serialize_fp3_fixed_len(modulus_len, &get<0>(xy)));
    TRY(b, serialize_fp3_fixed_len(modulus_len, &get<1>(xy)));

    a.insert(a.end(), b.begin(), b.end());

    return a;
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Fp2<E, F>, Fp2<E, F>, Slice<u8>>, ApiError> parse_ab_in_fp2_from_encoding(
    Slice<u8> encoding,
    usize modulus_len,
    Extension2<E, F> &field)
{
    TRY(decoded_a, decode_fp2(&encoding, modulus_len, field));
    auto a = get<0>(decoded_a);
    auto rest = get<1>(decoded_a);

    TRY(decoded_b, decode_fp2(&rest, modulus_len, field));

    return tuple(a, get<0>(decoded_b), get<1>(decoded_b));
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Fp3<E, F>, Fp3<E, F>, Slice<u8>>, ApiError> parse_ab_in_fp3_from_encoding(
    Slice<u8> encoding,
    usize modulus_len,
    Extension3<E, F> &field)
{
    TRY(decoded_a, decode_fp3(&encoding, modulus_len, field));
    auto a = get<0>(decoded_a);
    auto rest = get<1>(decoded_a);

    TRY(decoded_b, decode_fp3(&rest, modulus_len, field));

    return tuple(a, get<0>(decoded_b), get<1>(decoded_b));
}

#endif