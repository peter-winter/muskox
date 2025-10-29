/**
 * @file term.h
 * @brief Defines a class for terminal symbols in a grammar.
 *
 * This header provides a class to represent terminal symbols, which are the
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
 * @class term
 * @brief Represents a terminal symbol.
 */
class term
{
public:
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

    /**
     * @brief Destructor.
     */
    ~term() = default;

    /**
     * @brief Gets the name.
     *
     * @return The name.
     */
    std::string_view name() const { return name_; }

    /**
     * @brief Gets the associativity.
     *
     * @return The associativity.
     */
    associativity assoc() const { return assoc_; }

    /**
     * @brief Gets the precedence.
     *
     * @return Optional precedence.
     */
    std::optional<size_t> prec() const { return prec_; }

private:
    std::string_view name_; //!< The name of the terminal.
    associativity assoc_; //!< The associativity.
    std::optional<size_t> prec_; //!< The optional precedence.
};

} // namespace muskox
