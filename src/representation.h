#ifndef H_REPRESENTATION
#define H_REPRESENTATION

#include "types.h"
#include "ctbignum/ctbignum.hpp"
using namespace cbn::literals;

// // C++ equivalent of num-bigint BigUint
// // Where Rust's version stores integer in u32 form, but computes in u64 form.
// // While C++ version computes with stored format, that's why this with u64.
// template <usize N>
// using Element = cbn::big_int<N, u64>;

// using namespace cbn::literals;

// // C++ equivalent of num-bigint BigUint
// // Where Rust's version stores integer in u32 form, but computes in u64 form.
// // While C++ version computes with stored format, that's why this with u32.
// typedef<usize N> class Element
// {
//     // use namespace cbn;
//     // big_int<N, u64> num;

// public:
//     const static usize NUM_LIMBS = N;
// }

#endif