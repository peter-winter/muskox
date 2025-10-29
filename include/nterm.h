/**
 * @file nterm.h
 * @brief Defines a class for non-terminal symbols in a grammar.
 *
 * This header provides a simple class to represent non-terminal symbols,
 * which are the left-hand side of production rules in a context-free grammar.
 * Non-terminals are identified by their names and are used in building
 * the symbol collection and parser tables.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <string_view>

namespace muskox
{

/**
 * @class nterm
 * @brief Represents a non-terminal symbol.
 */
class nterm
{
public:
    /**
     * @brief Constructs a non-terminal with a name.
     *
     * @param name The name of the non-terminal.
     */
    nterm(std::string_view name) : name_(name) {}

    /**
     * @brief Destructor.
     */
    ~nterm() = default;

    /**
     * @brief Gets the name of the non-terminal.
     *
     * @return The name as string_view.
     */
    std::string_view name() const { return name_; }

private:
    std::string_view name_; //!< The stored name of the non-terminal.
};

} // namespace muskox
