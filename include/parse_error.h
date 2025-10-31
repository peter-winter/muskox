#pragma once

#include "formatted_error.h"

namespace muskox
{

struct parse_error_templates
{
    enum class code
    {
        ok,
        syntax_error
    };

    static constexpr const char* templates_[] =
    {
        "Ok",
        "Syntax error: unexpected {}"
    };
};

using parse_error = formatted_message<parse_error_templates>;

} // namespace muskox
