#include <iostream>
#include "api.h"

int main()
{
    // Api
    std::vector<std::uint8_t> input = {0, 1, 2, 3, 4};
    auto result = run(input);
    if (auto err = std::get_if<1>(&result))
    {
        std::cout
            << "Err: " << *err << std::endl;
    }
    else
    {
        std::cout
            << "Ok" << std::endl;
    }
}