/**
 * @file grammar_error.h
 * @brief Defines error templates and types specific to grammar validation and parsing.
 *
 * This header provides an enumeration of grammar-related error codes and corresponding
 * message templates. It uses the formatted_error system to create typed errors and
 * warnings for issues like symbol conflicts, recursion, or invalid roots in grammars.
 *
 * These are used during grammar construction and validation in the MuskOx parser generator.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "formatted_error.h"

namespace muskox
{

/**
 * @struct grammar_error_templates
 * @brief Templates for grammar errors.
 */
struct grammar_error_templates
{
    /**
     * @brief Enumeration of error codes.
     */
    enum class code
    {
        ok, 
        symbol_exists, 
        lside_not_exists, 
        lside_term, 
        rside_not_exist, 
        nterm_no_rsides,
        root_term, 
        root_not_exists, 
        cannot_refer_special, 
        no_nterm,
        no_term,
        nterm_unsolvable,
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

    static constexpr const char* templates_[] = /// Array of template strings.
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
        "No terminals",
        "Nonterminal '{}' is unsolvable",
        "Terminal '{}' is unused",
        "Nonterminal '{}' is unused",
        "Conflict in state {} on lookahead '{}' :",
        "\n    {}",
        "\n    {} (highest precedence)",
        "\n    shift on '{}'",
        "\n    shift on '{}' to state {} has the highest precedence",
        "\nConflict in state {} on lookahead '{}' resolved",
        "\nConflict in state {} on lookahead '{}' unresolved. Will resort to GLR parsing",
    };
};

using grammar_error = formatted_error<grammar_error_templates>; /// Type alias for grammar error.

using grammar_message = formatted_message<grammar_error_templates>; /// Type alias for grammar message.

} // namespace muskox
