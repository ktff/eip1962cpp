#include "api.h"
#include "constants.h"
#include "deserialization.h"
#include "multiexp.h"

/*
Execution path goes run -> run_operation -> run_operation_extension
Where every call adds at least one new template argument.
*/

// Executes non-pairing operation with given extension degree
template <class C, class F, usize N>
std::vector<std::uint8_t> run_operation_extension(u8 operation, u8 mod_byte_len, PrimeField<N> const &field, u8 extension_degree, Deserializer deserializer)
{
    // deser Extension2 & Weierstrass curve
    auto const extension = C(deserialize_non_residue<N>(mod_byte_len, field, extension_degree, deserializer), field);
    auto const wc = deserialize_weierstrass_curve<C, F, N>(mod_byte_len, extension, deserializer);

    // Run the operation for the result
    std::vector<u8> result;
    switch (operation)
    {
    // Addition
    case OPERATION_G2_ADD:
    {
        // deser CurvePoints to be added
        auto p_0 = deserialize_g2_point<C, F, N>(mod_byte_len, extension, deserializer);
        auto const p_1 = deserialize_g2_point<C, F, N>(mod_byte_len, extension, deserializer);

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
        auto const p_0 = deserialize_g2_point<C, F, N>(mod_byte_len, extension, deserializer);
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
        auto const expected_pair_len = 2 * extension_degree * mod_byte_len + wc.order_len();
        if (deserializer.remaining() != expected_pair_len)
        {
            input_err("Input length is invalid for number of pairs");
        }

        std::vector<std::tuple<CurvePoint<F>, std::vector<u64>>> pairs;
        for (auto i = 0; i < num_pairs; i++)
        {
            auto const p = deserialize_g2_point<C, F, N>(mod_byte_len, extension, deserializer);
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

// Executes non-pairing operation with known limb length
template <usize N>
std::vector<std::uint8_t> run_operation(u8 operation, u8 mod_byte_len, Deserializer deserializer)
{
    // deser Modulus -> Field
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);
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
                return run_operation<4>(operation, mod_byte_len, deserializer);
                // case 5:
                //     return run_operation<5>(operation, mod_byte_len, deserializer);
                // case 6:
                //     return run_operation<6>(operation, mod_byte_len, deserializer);
                // case 7:
                //     return run_operation<7>(operation, mod_byte_len, deserializer);
                // case 8:
                //     return run_operation<8>(operation, mod_byte_len, deserializer);
                // case 9:
                //     return run_operation<9>(operation, mod_byte_len, deserializer);
                // case 10:
                //     return run_operation<10>(operation, mod_byte_len, deserializer);
                // case 11:
                //     return run_operation<11>(operation, mod_byte_len, deserializer);
                // case 12:
                //     return run_operation<12>(operation, mod_byte_len, deserializer);
                // case 13:
                //     return run_operation<13>(operation, mod_byte_len, deserializer);
                // case 14:
                //     return run_operation<14>(operation, mod_byte_len, deserializer);
                // case 15:
                //     return run_operation<15>(operation, mod_byte_len, deserializer);
                // case 16:
                //     return run_operation<16>(operation, mod_byte_len, deserializer);

            default:
                unimplemented(stringf("for %u modulus limbs", limb_count));
            }
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
