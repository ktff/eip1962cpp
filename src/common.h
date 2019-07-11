#ifndef H_COMMON
#define H_COMMON

#include "types.h"

std::string err_concat(std::string const &a, std::string const &b);
// Defines so that compiler can see that an exception is being thrown.

#define unimplemented(ERR) throw std::runtime_error(err_concat("Unimplemented", ERR));

#define api_err(ERR) throw std::domain_error(ERR);

#define input_err(ERR) throw std::domain_error(err_concat("Invalid input parameters, ", ERR));

#define unexpected_zero_err(ERR) throw std::domain_error(err_concat("parameter expected to be non-zero, ", ERR));

std::string stringf(const char *format, ...);

#endif