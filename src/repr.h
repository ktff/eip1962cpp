#ifndef H_REPR
#define H_REPR

#include "types.h"
#include "ctbignum/ctbignum.hpp"

template <usize N>
using Repr = cbn::big_int<N>;

#endif