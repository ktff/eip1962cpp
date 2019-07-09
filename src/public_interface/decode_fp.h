#ifndef H_DECODE_FP
#define H_DECODE_FP

#include "../common.h"
#include "decode_utils.h"
#include "../field.h"
#include "../extension_towers/fp3.h"
#include "../extension_towers/fp2.h"
#include "../fp.h"

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Fp<E, F>, Slice<u8>>, ApiError> decode_fp(
    Slice<u8> bytes,
    usize field_byte_len,
    F &base_field)
{
    //TODO
    // TRY(splitted_x,split(bytes, field_byte_len, "Input is not long enough to get Fp element"));
    // auto x_encoding=get<0>(splitted_x);
    // auto rest=get<1>(splitted_x);

    // let x = Fp::from_be_bytes(base_field, x_encoding, true).map_err(| _ | {ApiError::InputError("Failed to parse Fp element".to_owned())}) ? ;

    // tuple(x, rest)
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Vec<u8>, ApiError> serialize_fp2_fixed_len

    (
        usize field_byte_len,
        Fp2<E, F> &element)
{
    // TODO
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Vec<u8>, ApiError> serialize_fp3_fixed_len

    (
        usize field_byte_len,
        Fp3<E, F> &element)
{
    // TODO
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Fp3<E, F>, Slice<u8>>, ApiError> decode_fp3(
    Slice<u8> bytes,
    usize field_byte_len,
    Extension3<E, F> &extension_field)

{
    // TODO
}

// E: ElementRepr
// F: SizedPrimeField<Repr = E>
template <class E, class F>
Result<Tuple<Fp3<E, F>, Slice<u8>>, ApiError> decode_fp2(
    Slice<u8> bytes,
    usize field_byte_len,
    Extension3<E, F> &extension_field)

{
    // TODO
}

#endif