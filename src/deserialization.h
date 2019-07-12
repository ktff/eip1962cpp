#ifndef H_DESERIALIZATION
#define H_DESERIALIZATION

#include "common.h"
#include "repr.h"
#include "field.h"
#include "curve.h"
#include "extension_towers/fp2.h"
#include "extension_towers/fp3.h"

// *************************** PRIMITIVE deserialization *********************** //

class Deserializer
{
    std::vector<uint8_t>::const_iterator begin;
    std::vector<uint8_t>::const_iterator const end;

public:
    Deserializer(std::vector<std::uint8_t> const &input) : begin(input.cbegin()), end(input.cend()) {}

    u8
    byte(str &err)
    {
        if (!ended())
        {
            auto ret = *begin;
            begin++;
            return ret;
        }
        else
        {
            input_err(err);
        }
    }

    u8 peek_byte(str &err) const
    {
        if (!ended())
        {
            return *begin;
        }
        else
        {
            input_err(err);
        }
    }

    // Deserializes number in Big endian format with bytes.
    template <usize N>
    Repr<N> number(u8 bytes, str &err)
    {
        Repr<N> num = {0};
        read(bytes, num, err);
        return num;
    }

    // Deserializes number in Big endian format with bytes.
    std::vector<u64> dyn_number(u8 bytes, str &err)
    {
        std::vector<u64> num;
        num.resize((bytes + sizeof(u64) - 1) / sizeof(u64), 0);
        read(bytes, num, err);
        return num;
    }

    bool ended() const
    {
        return begin == end;
    }

private:
    // Deserializes number in Big endian format with bytes.
    template <class T>
    void read(u8 bytes, T &num, str &err)
    {
        for (auto i = 0; i < bytes; i++)
        {
            auto b = byte(err);
            auto j = bytes - 1 - i;
            auto at = j / sizeof(u64);
            auto off = (j - at * sizeof(u64)) * 8;
            num[at] |= ((u64)b) << off;
        }
    }
};

template <class E>
std::vector<u64> deserialize_scalar(u8 mod_byte_len, WeierstrassCurve<E> const &wc, Deserializer &deserializer)
{
    auto scalar = deserializer.dyn_number(mod_byte_len, "Input is not long enough to get scalar");
    if (greater_or_equal(scalar, wc.subgroup_order()))
    {
        input_err("Group order is less or equal scalar");
    }
    return scalar;
}

// *************************** SPECIAL PRIMITIVE deserialization *********************** //

template <usize N>
Repr<N> deserialize_modulus(u8 mod_byte_len, Deserializer &deserializer)
{
    if (deserializer.peek_byte("Input is not long enough to get modulus") == 0)
    {
        input_err("In modulus encoding highest byte is zero");
    }
    auto modulus = deserializer.number<N>(mod_byte_len, "Input is not long enough to get modulus");
    constexpr Repr<N> zero = {0};
    if (modulus == zero)
    {
        unexpected_zero_err("Modulus can not be zero");
    }
    if (is_even(modulus))
    {
        input_err("Modulus is even");
    }
    constexpr Repr<N> three = {3};
    if (modulus < three)
    {
        input_err("Modulus is less than 3");
    }
    return modulus;
}

template <usize N>
Fp<N> deserialize_non_residue(u8 mod_byte_len, PrimeField<N> const &field, u8 extension_degree, Deserializer &deserializer)
{
    auto x = deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp element");
    auto const non_residue = Fp<N>::from_repr(x, field);
    if (non_residue.is_zero())
    {
        unexpected_zero_err("Fp* non-residue can not be zero");
    }

    if (!non_residue.is_non_nth_root(extension_degree))
    {
        input_err("Non-residue for Fp* is actually a residue");
    }

    return non_residue;
}

// ********************* OVERLOADED deserializers of Fp2 and Fp3 *********************** //

template <usize N>
Fp2<N> deserialize_fpM(u8 mod_byte_len, FieldExtension2<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp2_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp2_c1 element"), field);
    return Fp2<N>(c0, c1, field);
}

template <usize N>
Fp3<N> deserialize_fpM(u8 mod_byte_len, FieldExtension3<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c1 element"), field);
    auto const c2 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c2 element"), field);
    return Fp3<N>(c0, c1, c2, field);
}

// ************************* CURVE deserializers ***************************** //

template <class C, class F, usize N>
WeierstrassCurve<F> deserialize_weierstrass_curve(u8 mod_byte_len, C const &field, Deserializer &deserializer)
{
    F a = deserialize_fpM(mod_byte_len, field, deserializer);
    F b = deserialize_fpM(mod_byte_len, field, deserializer);

    auto order_len = deserializer.byte("Input is not long enough to get group size length");
    auto order = deserializer.dyn_number(order_len, "Input is not long enough to get main group order size");

    auto zero = true;
    for (auto it = order.cbegin(); it != order.cend(); it++)
    {
        zero &= *it == 0;
    }
    if (zero)
    {
        input_err("Group order is zero");
    }

    return WeierstrassCurve(a, b, order, order_len);
}

template <class C, class F, usize N>
CurvePoint<F> deserialize_g2_point(u8 mod_byte_len, C const &field, Deserializer &deserializer)
{
    F x = deserialize_fpM(mod_byte_len, field, deserializer);
    F y = deserialize_fpM(mod_byte_len, field, deserializer);
    return CurvePoint(x, y);
}

#endif