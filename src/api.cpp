#include "api.h"
#include <stdexcept>
#include "common.h"
#include "constants.h"
#include "repr.h"
#include "field.h"
#include "weierstrass/curve.h"
#include "extension_towers/fp2.h"
#include "extension_towers/fp3.h"
#include "multiexp.h"

typedef std::vector<uint8_t>::const_iterator It;

class Deserializer
{
    It begin;
    It const end;

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

    bool ended() const
    {
        return begin == end;
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

template <class E>
std::vector<u64> decode_scalar(u8 mod_byte_len, WeierstrassCurve<E> const &wc, Deserializer &deserializer)
{
    auto scalar = deserializer.dyn_number(mod_byte_len, "Input is not long enough to get scalar");
    if (greater_or_equal_dyn(scalar, wc.subgroup_order()))
    {
        input_err("Group order is less or equal scalar");
    }
    return scalar;
}

template <usize N>
Repr<N> decode_modulus(u8 mod_byte_len, Deserializer &deserializer)
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
bool is_non_nth_root(Fp<N> const &element, Repr<N> modulus, u64 n)
{
    if (element.is_zero())
    {
        return false;
    }

    auto power = modulus;
    constexpr Repr<N> one = {1};
    power = cbn::subtract_ignore_carry(power, one);
    Repr<N> divisor = {n};
    if (!cbn::is_zero(power % divisor))
    {
        return false;
    }
    power = power / divisor;

    auto l = element.pow(power);
    auto e_one = element.one();

    return l != e_one;
}

// ********************* OVERLOADED deserializers of of Fp2 and Fp3 *********************** //
template <usize N>
Fp2<N> deser_fpM(u8 mod_byte_len, FieldExtension2<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp2_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp2_c1 element"), field);
    return Fp2<N>(c0, c1, field);
}

template <usize N>
Fp3<N> deser_fpM(u8 mod_byte_len, FieldExtension3<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c1 element"), field);
    auto const c2 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c2 element"), field);
    return Fp3<N>(c0, c1, c2, field);
}

// ************************* CURVE deserializers ***************************** //
// Expects directly data of extension.
template <class C, usize N>
C deser_extension(u8 mod_byte_len, PrimeField<N> const &field, u8 extension_degree, Deserializer &deserializer)
{
    auto x = deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp element");
    auto const non_residue = Fp<N>::from_repr(x, field);
    if (non_residue.is_zero())
    {
        unexpected_zero_err("Fp* non-residue can not be zero");
    }

    if (!is_non_nth_root<N>(non_residue, field.mod(), extension_degree))
    {
        input_err("Non-residue for Fp* is actually a residue");
    }

    return C(non_residue, field);
}

template <class C, class F, usize N>
WeierstrassCurve<F> decode_weierstrass_curve(u8 mod_byte_len, C const &field, Deserializer &deserializer)
{
    F a = deser_fpM(mod_byte_len, field, deserializer);
    F b = deser_fpM(mod_byte_len, field, deserializer);

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
CurvePoint<F> decode_g2_point(u8 mod_byte_len, C const &field, Deserializer &deserializer)
{
    F x = deser_fpM(mod_byte_len, field, deserializer);
    F y = deser_fpM(mod_byte_len, field, deserializer);
    return CurvePoint(x, y);
}

// ************************* MAIN functions ******************************** //

template <class C, class F, usize N>
std::vector<std::uint8_t> run_operation_extension(u8 operation, u8 mod_byte_len, PrimeField<N> const &field, u8 extension_degree, Deserializer deserializer)
{
    // deser Extension2 & Weierstrass curve
    auto const extension = deser_extension<C, N>(mod_byte_len, field, extension_degree, deserializer);
    auto const wc = decode_weierstrass_curve<C, F, N>(mod_byte_len, extension, deserializer);

    // Run the operation for the result
    std::vector<u8> result;
    switch (operation)
    {
    case OPERATION_G2_ADD:
    {
        // deser CurvePoints to be added
        auto p_0 = decode_g2_point<C, F, N>(mod_byte_len, extension, deserializer);
        auto const p_1 = decode_g2_point<C, F, N>(mod_byte_len, extension, deserializer);

        // Apply addition
        p_0.add(p_1, wc, extension);

        // seri Result
        p_0.serialize(mod_byte_len, result);
        break;
    }
    case OPERATION_G2_MUL:
    {
        // deser CurvePoint & Scalar
        auto const p_0 = decode_g2_point<C, F, N>(mod_byte_len, extension, deserializer);
        auto const scalar = decode_scalar(mod_byte_len, wc, deserializer);

        // Apply multiplication
        auto r = p_0.mul(scalar, wc, extension);

        // seri Result
        r.serialize(mod_byte_len, result);
        break;
    }
    case OPERATION_G2_MULTIEXP:
    {
        // deser (CurvePoint,Scalar) pairs
        auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
        if (num_pairs == 0)
        {
            input_err("Invalid number of pairs");
        }
        std::vector<std::tuple<CurvePoint<F>, std::vector<u64>>> pairs;
        for (auto i = 0; i < num_pairs; i++)
        {
            auto const p = decode_g2_point<C, F, N>(mod_byte_len, extension, deserializer);
            auto const scalar = decode_scalar(mod_byte_len, wc, deserializer);
            pairs.push_back(tuple(p, scalar));
        }

        // Check if all input has been used up
        if (!deserializer.ended())
        {
            input_err("Input length is invalid for number of pairs");
        }

        // Apply Multiexponentiation
        auto const r = peepinger(pairs, wc, extension);

        // seri Result
        r.serialize(mod_byte_len, result);
        break;
    }
    default:
        unimplemented("");
    }

    // Done
    return result;
}

// Runs non-pairing operation with known limb length
template <usize N>
std::vector<std::uint8_t> run_operation(u8 operation, u8 mod_byte_len, Deserializer deserializer)
{
    // deser Modulus -> Field
    auto const modulus = decode_modulus<N>(mod_byte_len, deserializer);
    auto const field = PrimeField(modulus);

    // Soulution by extension degree
    auto const extension_degree = deserializer.byte("Input is not long enough to get extension degree");
    switch (extension_degree)
    {
    case 2:
    {
        return run_operation_extension<FieldExtension2<N>, Fp2<N>, N>(operation, mod_byte_len, field, extension_degree, deserializer);
    }
    case 3:
    {
        return run_operation_extension<FieldExtension3<N>, Fp3<N>, N>(operation, mod_byte_len, field, extension_degree, deserializer);
    }

    default:
        input_err("Invalid extension degree");
    }
}

std::variant<std::vector<std::uint8_t>, std::basic_string<char>>
run(std::vector<std::uint8_t> const &input)
{
    try
    {
        auto deserializer = Deserializer(input);
        auto operation = deserializer.byte("Input should be longer than operation type encoding");

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
            auto mod_byte_len = deserializer.byte("Input is not long enough to get modulus length");
            auto limb_count = (mod_byte_len + 7) / 8;

            switch (limb_count)
            {
            case 0:
                input_err("Modulus length is zero");
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                return run_operation<4>(operation, mod_byte_len, deserializer);
            case 5:
                return run_operation<5>(operation, mod_byte_len, deserializer);
            case 6:
                return run_operation<6>(operation, mod_byte_len, deserializer);
            case 7:
                return run_operation<7>(operation, mod_byte_len, deserializer);
            case 8:
                return run_operation<8>(operation, mod_byte_len, deserializer);
            case 9:
                return run_operation<9>(operation, mod_byte_len, deserializer);
            case 10:
                return run_operation<10>(operation, mod_byte_len, deserializer);
            case 11:
                return run_operation<11>(operation, mod_byte_len, deserializer);
            case 12:
                return run_operation<12>(operation, mod_byte_len, deserializer);
            case 13:
                return run_operation<13>(operation, mod_byte_len, deserializer);
            case 14:
                return run_operation<14>(operation, mod_byte_len, deserializer);
            case 15:
                return run_operation<15>(operation, mod_byte_len, deserializer);
            case 16:
                return run_operation<16>(operation, mod_byte_len, deserializer);

            default:
                unimplemented(stringf("for %u modulus limbs", limb_count));
                break;
            }
            break;
        }
        case OPERATION_PAIRING:
            unimplemented("");
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
