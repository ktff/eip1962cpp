#ifndef H_API
#define H_API

#include <variant>
#include <vector>
#include <string>

// Main api function.
// Provides eliptic curve arithmetic operations through ABI as defined in https://eips.ethereum.org/EIPS/eip-1962
// Returns result of the operation, or a string describing error that occured.
std::variant<std::vector<std::uint8_t>, std::basic_string<char>> run(std::vector<std::uint8_t> const &input);

#endif
