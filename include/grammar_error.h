// grammar_error.h
#pragma once

#include <formatted_error.h>

namespace ptg
{

struct grammar_error_templates
{
    enum class code
    {
        ok, 
        symbol_exists, 
        lside_not_exists, 
        lside_term, 
        rside_not_exist, 
        nterm_no_rules, 
        root_term, 
        root_not_exists, 
        cannot_refer_special, 
        no_nterm,
        nterm_unsolvable_left_recursion,
        unused_term,
        unused_nterm,
        conflict_intro,
        conflict_detail,
        conflict_detail_highest_prec,
        conflict_detail_shift,
        conflict_detail_shift_highest_prec,
        conflict_resolved,
        conflict_unresolved
    };

    static constexpr const char* templates_[] =
    {
        "Ok",
        "Symbol '{}' already exists",
        "Left side '{}' does not exist",
        "Left side '{}' is a terminal",
        "Right side symbol '{}' does not exist",
        "Nonterminal '{}' has no productions",
        "Root symbol '{}' is a terminal",
        "Root symbol '{}' does not exist",
        "Cannot refer special '{}' symbol",
        "No nonterminals",
        "Nonterminal '{}' has unsolvable left recursion",
        "Terminal '{}' is unused",
        "Nonterminal '{}' is unused",
        "Conflict in state {} on lookahead '{}' :",
        "\n    {}",
        "\n    {} (highest precedence)",
        "\n    shift on '{}'",
        "\n    shift on '{}' to state {} has the highest precedence",
        "Conflict in state {} on lookahead '{}' resolved",
        "Conflict in state {} on lookahead '{}' unresolved",
    };
};

using grammar_error = formatted_error<grammar_error_templates>;
using grammar_warning = formatted_message<grammar_error_templates>;

} // namespace ptg
