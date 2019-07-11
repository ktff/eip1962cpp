#include "api.h"
#include <stdexcept>
#include "common.h"
#include "constants.h"
#include "repr.h"
#include "field.h"
#include "weierstrass/curve.h"
#include "extension_towers/fp2.h"

typedef std::vector<uint8_t>::const_iterator It;

class Decoder
{
    It begin;
    It const end;

public:
    Decoder(std::vector<std::uint8_t> const &input) : begin(input.cbegin()), end(input.cend()) {}

    u8
    byte(str &err)
    {
        if (begin != end)
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
        if (begin != end)
        {
            return *begin;
        }
        else
        {
            input_err(err);
        }
    }

    // Decodes number in Big endian format with bytes.
    template <usize N>
    Repr<N> number(u8 bytes, str &err)
    {
        Repr<N> num = {0};
        read(bytes, num, err);
        return num;
    }

    // Decodes number in Big endian format with bytes.
    std::vector<u64> dyn_number(u8 bytes, str &err)
    {
        std::vector<u64> num;
        num.resize((bytes + sizeof(u64) - 1) / sizeof(u64), 0);
        read(bytes, num, err);
        return num;
    }

private:
    // Decodes number in Big endian format with bytes.
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

template <usize N>
std::vector<u8> serialize_g2_point(u8 field_element_length, CurvePoint<Fp2<N>> const &point)
{
    unimplemented();
}

template <usize N>
Repr<N> decode_modulus(u8 field_element_length, Decoder &decoder)
{
    if (decoder.peek_byte("Input is not long enough to get modulus") == 0)
    {
        input_err("In modulus encoding highest byte is zero");
    }
    auto modulus = decoder.number<N>(field_element_length, "Input is not long enough to get modulus");
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
bool is_non_square(Fp<N> const &element, Repr<N> modulus_minus_one_by_2)
{
    if (element.is_zero())
    {
        return false;
    }
    auto l = element.pow(modulus_minus_one_by_2);
    auto one = element.one();

    return l != one;
}

// Expects directly data of extension.
template <usize N>
FieldExtension2<N> decode_fp2_extension(u8 field_element_length, PrimeField<N> const &field, Decoder &decoder)
{
    auto x = decoder.number<N>(field_element_length, "Input is not long enough to get Fp element");
    auto const non_residue = Fp<N>::from_repr(x, field);
    if (non_residue.is_zero())
    {
        unexpected_zero_err("Fp2 non-residue can not be zero");
    }

    constexpr Repr<N> one = {1};
    Repr<N> modulus_minus_one = cbn::subtract_ignore_carry(field.mod(), one);
    Repr<N> modulus_minus_one_by_2 = cbn::shift_right(modulus_minus_one, 1);
    if (!is_non_square<N>(non_residue, modulus_minus_one_by_2))
    {
        input_err("Non-residue for Fp2 is actually a residue");
    }

    return FieldExtension2(non_residue, field);
}

template <usize N>
Fp2<N> decode_fp2(u8 field_element_length, FieldExtension2<N> const &field, Decoder &decoder)
{
    auto const c0 = Fp<N>::from_repr(decoder.number<N>(field_element_length, "Input is not long enough to get Fp2_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(decoder.number<N>(field_element_length, "Input is not long enough to get Fp2_c1 element"), field);
    return Fp2<N>(c0, c1, field);
}

template <usize N>
WeierstrassCurve<Fp2<N>> decode_weierstrass_curve(u8 field_element_length, FieldExtension2<N> const &field, Decoder &decoder)
{
    auto a = decode_fp2(field_element_length, field, decoder);
    auto b = decode_fp2(field_element_length, field, decoder);

    auto order_len = decoder.byte("Input is not long enough to get group size length");
    auto order = decoder.dyn_number(order_len, "Input is not long enough to get main group order size");

    auto zero = true;
    for (auto it = order.cbegin(); it != order.cend(); it++)
    {
        zero &= *it == 0;
    }
    if (zero)
    {
        input_err("Group order is zero");
    }

    return WeierstrassCurve(a, b, order);
}

template <usize N>
CurvePoint<Fp2<N>> decode_g2_point(u8 field_element_length, FieldExtension2<N> const &field, Decoder &decoder)
{
    auto x = decode_fp2(field_element_length, field, decoder);
    auto y = decode_fp2(field_element_length, field, decoder);
    return CurvePoint(x, y);
}

// Expects directly data of modulus.
template <usize N>
std::vector<std::uint8_t> g2_add(u8 field_element_length, Decoder decoder)
{
    auto modulus = decode_modulus<N>(field_element_length, decoder);
    auto field = PrimeField(modulus);

    auto extension_degree = decoder.byte("Input is not long enough to get extension degree");
    switch (extension_degree)
    {
    case 2:
    {
        auto extension2 = decode_fp2_extension(field_element_length, field, decoder);
        auto wcurve = decode_weierstrass_curve(field_element_length, extension2, decoder);

        auto p_0 = decode_g2_point(field_element_length, extension2, decoder);
        auto const p_1 = decode_g2_point(field_element_length, extension2, decoder);

        p_0.add(p_1, wcurve, extension2);

        return serialize_g2_point(field_element_length, p_0);
    }
    case 3:
        input_err("Extension degree 3 is not yet implemented");

    default:
        input_err("Invalid extension degree");
    }
}

// Runs non-pairing operation with known limb length
template <usize N>
std::vector<std::uint8_t> run_operation(u8 operation, u8 field_element_length, Decoder decoder)
{

    switch (operation)
    {
    case OPERATION_G2_ADD:
        return g2_add<N>(field_element_length, decoder);
    default:
        unimplemented();
        break;
    }
}

std::variant<std::vector<std::uint8_t>, std::basic_string<char>>
run(std::vector<std::uint8_t> const &input)
{
    try
    {
        auto decoder = Decoder(input);
        auto operation = decoder.byte("Input should be longer than operation type encoding");

        switch (operation)
        {
        case OPERATION_G1_ADD:
        case OPERATION_G1_MUL:
        case OPERATION_G1_MULTIEXP:
        case OPERATION_G2_ADD:
        case OPERATION_G2_MUL:
        case OPERATION_G2_MULTIEXP:
        {
            // Common ABI operations for non-pairing operations
            auto field_element_length = decoder.byte("Input is not long enough to get modulus length");
            auto limb_count = (field_element_length + 7) / 8;

            switch (limb_count)
            {
            case 0:
                input_err("Modulus length is zero");
            case 1:
            case 2:
            case 3:
            case 4:
                return run_operation<4>(operation, field_element_length, decoder);
                // TODO: uncomment
                // case 5:
                //     return run_operation<5>(operation, field_element_length, decoder);
                // case 6:
                //     return run_operation<6>(operation, field_element_length, decoder);
                // case 7:
                //     return run_operation<7>(operation, field_element_length, decoder);
                // case 8:
                //     return run_operation<8>(operation, field_element_length, decoder);
                // case 9:
                //     return run_operation<9>(operation, field_element_length, decoder);
                // case 10:
                //     return run_operation<10>(operation, field_element_length, decoder);
                // case 11:
                //     return run_operation<11>(operation, field_element_length, decoder);
                // case 12:
                //     return run_operation<12>(operation, field_element_length, decoder);
                // case 13:
                //     return run_operation<13>(operation, field_element_length, decoder);
                // case 14:
                //     return run_operation<14>(operation, field_element_length, decoder);
                // case 15:
                //     return run_operation<15>(operation, field_element_length, decoder);
                // case 16:
                //     return run_operation<16>(operation, field_element_length, decoder);

            default:
                unimplemented(stringf("for %u modulus limbs", limb_count));
            }
        }
        case OPERATION_PAIRING:
            unimplemented();
            break;

        default:
            input_err("Unknown operation type");
        }
    }
    catch (std::domain_error const &e)
    {
        return e.what();
    }
    catch (std::runtime_error const &e)
    {
        return e.what();
    }
}
