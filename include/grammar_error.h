#pragma once

#include <formatted_error.h>

namespace ptg
{

struct grammar_error_templates
{
    enum class code { ok, symbol_exists, lside_not_exists, lside_term, rside_not_exist, nterm_no_rules, root_term, root_not_exists, cannot_refer_special };

    static constexpr const char* templates_[] =
    {
        "Ok.",
        "Symbol '{}' already exists.",
        "Left side '{}' does not exist.",
        "Left side '{}' is a terminal.",
        "Right side symbol '{}' does not exist.",
        "Nonterminal {} has no productions.",
        "Root symbol '{}' is a terminal.",
        "Root symbol '{}' does not exist.",
        "Cannot refer special '{}' symbol."
    };
};

using grammar_error = formatted_error<grammar_error_templates>;

} // namespace ptg
