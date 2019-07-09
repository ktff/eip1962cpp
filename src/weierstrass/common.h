#ifndef H_W_COMMON
#define H_W_COMMON

#include "../common.h"

template <class P>
class CurveParameters
{
public:
    virtual P params() = 0;
};

#endif