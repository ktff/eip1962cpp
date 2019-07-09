#include "../types.h"
#include "../slice.h"
#include "../errors.h"
#include "decode_utils.h"
#include "g2_ops.h"

class API
{
public:
    static Result<Vec<u8>, ApiError> run(const Vec<u8> &bytes)
    {
        TRY(result, split(Slice<u8>(bytes), OPERATION_ENCODING_LENGTH, "Input should be longer than operation type encoding"));
        auto op_type = get<0>(result);
        auto rest = get<1>(result);

        switch (op_type[0])
        {
        // case OPERATION_G1_ADD : {
        //     PublicG1Api::add_points(&rest)
        // },
        // OPERATION_G1_MUL => {
        //     PublicG1Api::mul_point(&rest)
        // },
        // OPERATION_G1_MULTIEXP => {
        //     PublicG1Api::multiexp(&rest)
        // },
        // case OPERATION_G2_ADD:
        // {
        //     return PublicG2Api().add_points(rest);
        // }
        // case OPERATION_G2_MUL:
        // {
        //     return PublicG2Api().mul_point(rest);
        // }
        // case OPERATION_G2_MULTIEXP:
        // {
        //     return PublicG2Api().multiexp(rest);
        // }
        // case OPERATION_PAIRING => {
        //     PublicPairingApi::pair(&rest)
        // },
        default:
        {
            return ApiError(ApiErrorKind::InputError, "Unknown operation type type");
        }
        }
    }
};