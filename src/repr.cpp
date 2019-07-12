#include "repr.h"

// a >= b
// Where a and b are numbers
bool greater_or_equal_dyn(std::vector<u64> const &a, std::vector<u64> const &b)
{
    for (auto i = a.size(); i < b.size(); i++)
    {
        if (b[i] > 0)
        {
            return false;
        }
    }
    for (auto i = b.size(); i < a.size(); i++)
    {
        if (a[i] > 0)
        {
            return true;
        }
    }

    for (i32 i = ((i32)min(a.size(), b.size())) - 1; i >= 0; i--)
    {
        if (a[i] > b[i])
        {
            return true;
        }
        else if (a[i] < b[i])
        {
            return false;
        }
    }

    // Equal
    return true;
}