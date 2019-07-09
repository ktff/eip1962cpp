#ifndef H_G2_OPS
#define H_G2_OPS

#include "../slice.h"
#include "decode_g2.h"

/// Every call has common parameters (may be redundant):
/// - Lengths of modulus (in bytes)
/// - Field modulus
/// - Extension degree (2/3)
/// - Non-residue
/// - Curve A in Fpk
/// - Curve B in Fpk
/// - Length of a scalar field (curve order) (in bytes)
/// - Curve order

class G2Api
{
public:
    virtual Result<Vec<u8>, ApiError> add_points(Slice<u8> bytes) = 0;
    virtual Result<Vec<u8>, ApiError> mul_point(Slice<u8> bytes) = 0;
    virtual Result<Vec<u8>, ApiError> multiexp(Slice<u8> bytes) = 0;
};

// E: ElementRepr
template <class E>
class G2ApiImplementationFp2
{

    Result<Vec<u8>, ApiError> add_points(Slice<u8> bytes)
    {
        // TODO
    }

    Result<Vec<u8>, ApiError> mul_point(Slice<u8> bytes)
    {
        // TODO
    }

    Result<Vec<u8>, ApiError> multiexp(Slice<u8> bytes)
    {
        // TODO
    }
};

class PublicG2Api : G2Api
{
public:
    Result<Vec<u8>, ApiError> add_points(Slice<u8> bytes)
    {
        // TODO
    }

    Result<Vec<u8>, ApiError> mul_point(Slice<u8> bytes)
    {
        // TODO
    }

    Result<Vec<u8>, ApiError> multiexp(Slice<u8> bytes)
    {
        // TODO
    }
};

#endif