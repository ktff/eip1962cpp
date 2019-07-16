#include "api.h"
#include "constants.h"
#include "deserialization.h"
#include "multiexp.h"
#include "extension_towers/fp4.h"
#include "pairings/mnt4.h"
#include "pairings/mnt6.h"
#include "pairings/bn.h"

/*
Execution path goes run -> run_limbed -> run_operation -> {run_pairing_mnt,run_pairing_bn,run_operation_extension}
*/

// Executes non-pairing operation with given extension degree
template <class C, class F, usize N>
std::vector<std::uint8_t> run_operation_extension(u8 operation, u8 mod_byte_len, PrimeField<N> const &field, u8 extension_degree, Deserializer deserializer)
{
    // deser Extension & Weierstrass curve
    auto const extension = C(deserialize_non_residue<N, Fp<N>>(mod_byte_len, field, extension_degree, deserializer), field);
    auto const wc = deserialize_weierstrass_curve<C, F, N>(mod_byte_len, extension, deserializer, false);

    // Run the operation for the result
    std::vector<u8> result;
    switch (operation)
    {
    // Addition
    case OPERATION_G2_ADD:
    {
        // deser CurvePoints to be added
        auto p_0 = deserialize_curve_point<C, F, N>(mod_byte_len, extension, deserializer);
        auto const p_1 = deserialize_curve_point<C, F, N>(mod_byte_len, extension, deserializer);

        // Apply addition
        p_0.add(p_1, wc, extension);

        // seri Result
        p_0.serialize(mod_byte_len, result);
        break;
    }
    // Multiplication
    case OPERATION_G2_MUL:
    {
        // deser CurvePoint & Scalar
        auto const p_0 = deserialize_curve_point<C, F, N>(mod_byte_len, extension, deserializer);
        auto const scalar = deserialize_scalar(wc, deserializer);

        // Apply multiplication
        auto r = p_0.mul(scalar, wc, extension);

        // seri Result
        r.serialize(mod_byte_len, result);
        break;
    }
    // Multiexponentiation
    case OPERATION_G2_MULTIEXP:
    {
        // deser (CurvePoint,Scalar) pairs
        auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
        if (num_pairs == 0)
        {
            input_err("Invalid number of pairs");
        }

        // Check if remaining input size is exact
        u32 const expected_pair_len = 2 * extension_degree * mod_byte_len + wc.order_len();
        if (deserializer.remaining() != expected_pair_len)
        {
            input_err("Input length is invalid for number of pairs");
        }

        std::vector<std::tuple<CurvePoint<F>, std::vector<u64>>> pairs;
        for (auto i = 0; i < num_pairs; i++)
        {
            auto const p = deserialize_curve_point<C, F, N>(mod_byte_len, extension, deserializer);
            auto const scalar = deserialize_scalar(wc, deserializer);
            pairs.push_back(tuple(p, scalar));
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

template <usize N>
std::vector<std::uint8_t> run_pairing_bn(u8 mod_byte_len, PrimeField<N> const &field, Deserializer deserializer)
{
    // Deser Weierstrass 1 & Extension2
    auto const g1_curve = deserialize_weierstrass_curve<PrimeField<N>, Fp<N>, N>(mod_byte_len, field, deserializer, true);
    auto const extension2 = FieldExtension2(deserialize_non_residue<N, Fp<N>>(mod_byte_len, field, 2, deserializer), field);

    // Deser Extension6 & TwistType
    auto const e6_non_residue = deserialize_non_residue<N, Fp2<N>>(mod_byte_len, extension2, 6, deserializer);
    auto const twist_type = deserialize_pairing_twist_type(deserializer);
    auto exp_base = WindowExpBase<Fp2<N>>(e6_non_residue, Fp2<N>::one(extension2), 8, 7);
    auto const extension6 = FieldExtension3over2(e6_non_residue, extension2, exp_base);

    // Construct Extension12
    auto const extension12 = FieldExtension2over3over2(extension6, exp_base);

    // Compute Weierstrass 2
    auto const o_e6_non_residue_inv = e6_non_residue.inverse();
    if (!o_e6_non_residue_inv)
    {
        unexpected_zero_err("Fp2 non-residue must be invertible");
    }
    auto const e6_non_residue_inv = o_e6_non_residue_inv.value();
    auto b_fp2 = Fp2<N>::zero(extension2);
    switch (twist_type)
    {
    case D:
    {
        b_fp2 = e6_non_residue_inv;
        b_fp2.mul_by_fp(g1_curve.get_b());
        break;
    }
    case M:
    {
        b_fp2 = e6_non_residue;
        b_fp2.mul_by_fp(g1_curve.get_b());
        break;
    }
    }
    auto const a_fp2 = Fp2<N>::zero(extension2);
    auto const g2_curve = WeierstrassCurve(a_fp2, b_fp2, g1_curve.subgroup_order(), g1_curve.order_len());

    // Decode u and it's sign
    auto const u = deserialize_scalar_with_bit_limit(MAX_BN_U_BIT_LENGTH, deserializer);
    auto const u_is_negative = deserialize_sign(deserializer);

    // Calculate six_u_plus_two
    auto six_u_plus_two = u;
    mul_scalar(six_u_plus_two, 6);
    add_scalar(six_u_plus_two, 2);
    if (calculate_hamming_weight(six_u_plus_two) > MAX_BN_SIX_U_PLUS_TWO_HAMMING)
    {
        input_err("6*U + 2 has too large hamming weight");
    }

    // Calculate non_residue_in_p_minus_one_over_2
    constexpr Repr<N> one = {1};
    auto const p_minus_one_over_2 = cbn::shift_right(field.mod() - one, 1);
    Fp2<N> const non_residue_in_p_minus_one_over_2 = e6_non_residue.pow(p_minus_one_over_2);

    // deser (CurvePoint<Fp<N>>,CurvePoint<F>) pairs
    auto const points = deserialize_points<N, Fp2<N>>(mod_byte_len, extension2, g1_curve, g2_curve, deserializer);

    // Construct BN engine
    auto const engine = BNengine(u, six_u_plus_two, u_is_negative, twist_type, g2_curve, non_residue_in_p_minus_one_over_2);

    // Execute pairing
    auto const opairing_result = engine.pair(points, extension12);
    if (!opairing_result)
    {
        unknown_parameter_err("Pairing engine returned no value");
    }

    // Finish
    auto const one_fp12 = Fp12<N>::one(extension12);
    auto const pairing_result = opairing_result.value();
    std::vector<std::uint8_t> result;
    if (pairing_result == one_fp12)
    {
        result.push_back(1);
    }
    else
    {
        result.push_back(0);
    }
    return result;
}

template <class F, class F2, class FEO, class FE, class ENGINE, usize N>
std::vector<std::uint8_t> run_pairing_mnt(u8 mod_byte_len, PrimeField<N> const &field, u8 extension_degree, Deserializer deserializer)
{
    // Deser Weierstrass 1 & Extension
    auto const g1_curve = deserialize_weierstrass_curve<PrimeField<N>, Fp<N>, N>(mod_byte_len, field, deserializer, true);
    auto const extension = FE(deserialize_non_residue<N, Fp<N>>(mod_byte_len, field, extension_degree * 2, deserializer), field);

    // Construct Extension 2
    auto const extension_2 = FEO(extension);

    // Construct Weistrass 2
    auto const one = Fp<N>::one(field);

    auto twist = F::zero(extension);
    twist.c1 = one;

    auto twist_squared = twist;
    twist_squared.square();

    auto twist_cubed = twist_squared;
    twist_cubed.mul(twist);

    auto a_fp2 = twist_squared;
    a_fp2.mul_by_fp(g1_curve.get_a());

    auto b_fp2 = twist_cubed;
    b_fp2.mul_by_fp(g1_curve.get_b());

    auto const g2_curve = WeierstrassCurve<F>(a_fp2, b_fp2, g1_curve.subgroup_order(), g1_curve.order_len());

    // Deserialize x
    auto const x = deserialize_scalar_with_bit_limit(MAX_ATE_PAIRING_ATE_LOOP_COUNT, deserializer);
    if (calculate_hamming_weight(x) > MAX_ATE_PAIRING_ATE_LOOP_COUNT_HAMMING)
    {
        input_err("X has too large hamming weight");
    }
    auto const x_is_negative = deserialize_sign(deserializer);

    // Deserialize exp_w0 & exp_w1
    auto const exp_w0 = deserialize_scalar_with_bit_limit(MAX_ATE_PAIRING_FINAL_EXP_W0_BIT_LENGTH, deserializer);
    auto const exp_w1 = deserialize_scalar_with_bit_limit(MAX_ATE_PAIRING_FINAL_EXP_W1_BIT_LENGTH, deserializer);
    auto const exp_w0_is_negative = deserialize_sign(deserializer);

    // deser (CurvePoint<Fp<N>>,CurvePoint<F>) pairs
    auto const points = deserialize_points<N, F>(mod_byte_len, extension, g1_curve, g2_curve, deserializer);

    // Construct MNT engine
    ENGINE const engine(x, x_is_negative, exp_w0, exp_w1, exp_w0_is_negative, g2_curve, twist);

    // Execute pairing
    auto const opairing_result = engine.pair(points, extension_2);
    if (!opairing_result)
    {
        unknown_parameter_err("Pairing engine returned no value");
    }

    // Finish
    auto const one_fp4 = F2::one(extension_2);
    auto const pairing_result = opairing_result.value();
    std::vector<std::uint8_t> result;
    if (pairing_result == one_fp4)
    {
        result.push_back(1);
    }
    else
    {
        result.push_back(0);
    }
    return result;
}

// Executes operation with known limb length
template <usize N>
std::vector<std::uint8_t> run_operation(u8 operation, std::optional<u8> curve_type, u8 mod_byte_len, Deserializer deserializer)
{
    // deser Modulus -> Field
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);
    auto const field = PrimeField(modulus);

    if (curve_type)
    {
        // Pairing operation
        assert(operation == OPERATION_PAIRING);

        switch (curve_type.value())
        {
        case MNT4:
            return run_pairing_mnt<Fp2<N>, Fp4<N>, FieldExtension2over2<N>, FieldExtension2<N>, MNT4engine<N>, N>(mod_byte_len, field, 2, deserializer);
        case MNT6:
            return run_pairing_mnt<Fp3<N>, Fp6_2<N>, FieldExtension2over3<N>, FieldExtension3<N>, MNT6engine<N>, N>(mod_byte_len, field, 3, deserializer);
        case BLS12:
            unimplemented("");
        case BN:
            return run_pairing_bn<N>(mod_byte_len, field, deserializer);

        default:
            unreachable("");
        }
    }
    else
    {
        // Non pairing operations
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
}

std::vector<std::uint8_t> run_limbed(u8 operation, std::optional<u8> curve_type, Deserializer deserializer)
{
    // Deserialize modulus length
    auto mod_byte_len = deserializer.byte("Input is not long enough to get modulus length");
    auto limb_count = (mod_byte_len + 7) / 8;

    // Call run_operation with adequate number of limbs
    switch (limb_count)
    {
    case 0:
        input_err("Modulus length is zero");
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        return run_operation<4>(operation, curve_type, mod_byte_len, deserializer);
        // case 5:
        //     return run_operation<5>(operation,curve_type, mod_byte_len, deserializer);
        // case 6:
        //     return run_operation<6>(operation,curve_type, mod_byte_len, deserializer);
        // case 7:
        //     return run_operation<7>(operation,curve_type, mod_byte_len, deserializer);
        // case 8:
        //     return run_operation<8>(operation,curve_type, mod_byte_len, deserializer);
        // case 9:
        //     return run_operation<9>(operation, curve_type,mod_byte_len, deserializer);
        // case 10:
        //     return run_operation<10>(operation,curve_type, mod_byte_len, deserializer);
        // case 11:
        //     return run_operation<11>(operation,curve_type, mod_byte_len, deserializer);
        // case 12:
        //     return run_operation<12>(operation,curve_type, mod_byte_len, deserializer);
        // case 13:
        //     return run_operation<13>(operation,curve_type, mod_byte_len, deserializer);
        // case 14:
        //     return run_operation<14>(operation,curve_type, mod_byte_len, deserializer);
        // case 15:
        //     return run_operation<15>(operation,curve_type, mod_byte_len, deserializer);
        // case 16:
        //     return run_operation<16>(operation,curve_type, mod_byte_len, deserializer);

    default:
        unimplemented(stringf("for %u modulus limbs", limb_count));
    }
}

// Main API function which receives ABI input and returns the result of operations, or description of occured error.
std::variant<std::vector<std::uint8_t>, std::basic_string<char>>
run(std::vector<std::uint8_t> const &input)
{
    try
    {
        // Deserialize operation
        auto deserializer = Deserializer(input);
        auto operation = deserializer.byte("Input should be longer than operation type encoding");

        switch (operation)
        {
        case OPERATION_G1_ADD:
        case OPERATION_G1_MUL:
        case OPERATION_G1_MULTIEXP:
            unimplemented("");

        case OPERATION_G2_ADD:
        case OPERATION_G2_MUL:
        case OPERATION_G2_MULTIEXP:
        {
            return run_limbed(operation, {}, deserializer);
        }
        case OPERATION_PAIRING:
        {
            auto curve_type = deserialize_pairing_curve_type(deserializer);
            return run_limbed(operation, curve_type, deserializer);
        }
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
