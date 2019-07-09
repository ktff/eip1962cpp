#ifndef H_DECODE_UTILS
#define H_DECODE_UTILS

#include "../common.h"
#include "constants.h"
// #include "../representation.h"

Result<Tuple<Slice<u8>, Slice<u8>>, ApiError> split(Slice<u8> bytes, usize at, const char *err)
{
    if (bytes.len() < at)
    {
        return ApiError(ApiErrorKind::InputError, String(err));
    }
    else
    {
        return bytes.split_at(at);
    }
}

// TODO: BigUint size should be known at compile time, but this function is dynamic, so needs to be specialized for call Sites
Result<Tuple<Slice<u8>, Slice<u8>>, ApiError> decode_biguint_with_length(
    Slice<u8> bytes)

{
    // let(length_encoding, rest)
    TRY(splited_length, split(bytes, BYTES_FOR_LENGTH_ENCODING, "Input is not long enough to get modulus length"));
    auto length_encoding = get<0>(splited_length);
    auto rest_0 = get<1>(splited_length);
    usize length = length_encoding[0];

    TRY(splited_be, split(rest_0, length, "Input is not long enough to get modulus"))
    return splited_be;
}

Result<Tuple<usize, Slice<u8>, Slice<u8>>, ApiError> parse_modulus_and_length(
    Slice<u8> bytes)
{
    TRY(splited_length, split(bytes, BYTES_FOR_LENGTH_ENCODING, "Input is not long enough to get modulus length"));
    auto length_encoding = get<0>(splited_length);
    auto rest_0 = get<1>(splited_length);
    usize length = length_encoding[0];

    TRY(splited_be, split(rest_0, length, "Input is not long enough to get modulus"))

    return tuple(length, get<0>(splited_be), get<1>(splited_be));
}

Result<Tuple<BigUint, usize, Slice<u8>>, ApiError> get_base_field_params(Slice<u8> bytes)
{
    // use crate::constants::THREE_BIGUINT;
    TRY(modulus_split, split(bytes, BYTES_FOR_LENGTH_ENCODING, "Input is not long enough to get modulus length"));
    usize modulus_len = get<0>(modulus_split)[0];
    auto rest_0 = get<1>(modulus_split);
    if (modulus_len == 0)
    {
        return ApiError(ApiErrorKind::InputError, "Modulus is length is zero");
    }

    TRY(modulus_encoding_split, split(rest_0, modulus_len, "Input is not long enough to get modulus"));
    auto modulus_encoding = get<0>(modulus_encoding_split);
    auto rest_1 = get<1>(modulus_encoding_split);
    if (modulus_encoding[0] == 0)
    {
        return ApiError(ApiErrorKind::InputError, "In modulus encoding highest byte is zero");
    }

    BigUint modulus(modulus_encoding);
    if (modulus.is_zero())
    {
        return ApiError(ApiErrorKind::UnexpectedZero, "Modulus can not be zero");
    }
    if (modulus.is_even())
    {
        return ApiError(ApiErrorKind::InputError, "Modulus is even");
    }
    if (modulus.less_than(THREE_BIGUINT))
    {
        return ApiError(ApiErrorKind::InputError, "Modulus is less than 3");
    }

    return tuple(modulus, modulus_len, rest_1);
}

// return:
// - modulus,
// - modulus_len,
// - extension degree
// - non-residue encoding
// - rest
Result<Tuple<BigUint, usize, u8, Slice<u8>, Slice<u8>>, ApiError> parse_modulus_and_extension_degree(Slice<u8> bytes)
{
    TRY(parsed, get_base_field_params(bytes));
    auto modulus = get<0>(parsed);
    auto modulus_len = get<1>(parsed);
    auto rest_0 = get<2>(parsed);

    TRY(splitted_degree, split(rest_0, EXTENSION_DEGREE_ENCODING_LENGTH, "Input is not long enough to get extension degree"));
    auto extension_degree_slice = (get<0>(splitted_degree));
    u8 extension_degree = extension_degree_slice[0];
    auto rest_1 = get<1>(splitted_degree);
    if (!(extension_degree == EXTENSION_DEGREE_2 || extension_degree == EXTENSION_DEGREE_3))
    {
        return ApiError(ApiErrorKind::InputError, "Extension degree must be 2 or 3");
    }

    TRY(splitted_encoding, split(rest_1, modulus_len, "Input is not long enough to Fp non-residue"));
    auto nonresidue_encoding = get<0>(splitted_encoding);
    auto rest_2 = get<1>(splitted_encoding);
    if (rest_2.len() == 0)
    {
        return ApiError(ApiErrorKind::InputError, "Input is not long enough");
    }

    return tuple(
        modulus,
        modulus_len,
        extension_degree,
        nonresidue_encoding,
        rest_2);
}

Result<usize, ApiError> num_libs_for_modulus(BigUint &modulus)
{
    auto modulus_limbs = (modulus.bits() / 64) + 1;
    if (modulus_limbs > 16)
    {
        return ApiError(ApiErrorKind::InputError, "Modulus is too large");
    }
    if (modulus_limbs < 4)
    {
        modulus_limbs = 4;
    }

    return modulus_limbs;
}

#endif