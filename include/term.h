/**
 * @file term.h
 * @brief Defines a struct for terminal symbols in a grammar.
 *
 * This header provides a struct to represent terminal symbols, which are the
 * basic tokens or leaves in a parse tree. Terminals can have optional precedence
 * and associativity for handling operator grammars.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "associativity.h"

#include <string_view>
#include <optional>

namespace muskox
{

/**
 * @struct term
 * @brief Represents a terminal symbol.
 */
struct term
{
    std::string_view name_; /// The name of the terminal.
    associativity assoc_; /// The associativity.
    std::optional<size_t> prec_; /// The optional precedence.

    /**
     * @brief Constructs a terminal.
     *
     * @param name The name.
     * @param prec Optional precedence.
     * @param assoc Associativity.
     */
    term(std::string_view name, std::optional<size_t> prec = std::nullopt, associativity assoc = associativity::left())
        : name_(name), assoc_(assoc), prec_(prec)
    {
    }
};

} // namespace muskox
