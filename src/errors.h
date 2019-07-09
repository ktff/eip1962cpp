#ifndef H_ERROR
#define H_ERROR

#include "types.h"
#include <utility>
#include <iostream>

enum ApiErrorKind
{
    UnexpectedZero,
    InputError,
    DivisionByZero,
    UnknownParameter,
    OutputError,
    RuntimeError
};

class ApiError
{
    ApiErrorKind kind;
    Option<String> description;

    ApiError();

public:
    ApiError(ApiErrorKind kind) : kind(kind)
    {
        description = {};
    }

    ApiError(ApiErrorKind kind, const char *desc) : kind(kind), description(String(desc))
    {
    }

    ApiError(ApiErrorKind kind, String desc) : kind(kind), description(desc)
    {
    }

    friend std::ostream &operator<<(std::ostream &output, const ApiError &e)
    {
        switch (e.kind)
        {
        case UnexpectedZero:
        {
            output << "parameter expected to be non-zero";
            break;
        }
        case InputError:
        {
            output << "invalid input parameters";
            break;
        }
        case DivisionByZero:
        {
            output << "division by zero";
            break;
        }
        case UnknownParameter:
        {
            output << "parameter has value out of bounds";
            break;
        }
        case OutputError:
        {
            output << "error outputing results";
            break;
        }
        default:
        {
            // TODO
        }
        }

        if (e.description)
        {
            output << ", " << *e.description;
        }

        return output;
    }
};

#endif