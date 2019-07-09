#ifndef H_SLICE
#define H_SLICE

#include "types.h"
#include "errors.h"

// TODO: with c++20 transition to span
template <class T>
class Slice
{
    const Vec<T> &vec;
    // First element of slice
    usize start;
    // After last element of slice
    usize end;

    Slice(const Vec<T> &vec, usize start, usize end) : vec(vec), start(start), end(end) {}

public:
    Slice(const Vec<T> &vec) : start(0), end(vec.size()), vec(vec) {}

    Slice<T>(const Slice<T> &s) : start(s.start), end(s.end), vec(s.vec) {}

    usize len(void)
    {
        return end - start;
    }

    Result<Tuple<Slice<u8>, Slice<u8>>, ApiError> split_at(usize at)
    {
        if (at < start || at >= end)
        {
            return ApiError(ApiErrorKind::RuntimeError, "Slice::split_at, at out of range");
        }
        else
        {
            Slice<u8> left(vec, start, at);
            Slice<u8> right(
                vec,
                at,
                end);
            return tuple(left, right);
        }
    }

    Vec<T> to_vec()
    {
        //TODO
    }

    const T &operator[](usize pos)
    {
        usize at = start + pos;
        if (at >= end)
        {
            // TODO: panic
        }
        return vec[at];
    }

    // BigUint to_biguint()
    // {
    // }
};

#endif