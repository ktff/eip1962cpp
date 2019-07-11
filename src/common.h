#ifndef H_COMMON
#define H_COMMON

#include "types.h"

void unimplemented();

void unimplemented(std::string const &err);

void api_err(std::string const &err);

void input_err(std::string const &err);

void unexpected_zero_err(std::string const &err);

std::string stringf(const char *format, ...);

#endif