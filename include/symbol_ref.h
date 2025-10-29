/**
 * @file symbol_ref.h
 * @brief Defines a reference to a symbol, distinguishing between terminals and non-terminals.
 *
 * This header introduces an enumeration for symbol types and a struct to reference
 * symbols by their type and index. It also provides comparison operators for sorting
 * and equality checks, useful in collections and parsing algorithms.
 *
 * Symbols are fundamental in the MuskOx grammar system, where terminals represent
 * leaf nodes (e.g., tokens) and non-terminals represent rules or productions.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <cstddef>
#include <tuple>

namespace muskox
{

/**
 * @brief Enumeration for symbol types.
 *
 * Distinguishes between terminal (leaf) and non-terminal (rule) symbols.
 */
enum class symbol_type { terminal, non_terminal };

/**
 * @brief Struct representing a reference to a symbol.
 *
 * Combines the type and an index into the corresponding collection in symbol_collection (terms_ or nterms_).
 */
struct symbol_ref
{
    symbol_type type_; //!< The type of the symbol (terminal or non-terminal).
    size_t index_; //!< The index within the type's collection.
};

/**
 * @brief Equality operator for symbol_ref.
 *
 * @param lhs Left-hand side symbol reference.
 * @param rhs Right-hand side symbol reference.
 * @return True if types and indices match, false otherwise.
 */
inline bool operator==(const symbol_ref& lhs, const symbol_ref& rhs)
{
    return lhs.type_ == rhs.type_ && lhs.index_ == rhs.index_;
}

/**
 * @brief Less-than operator for symbol_ref.
 *
 * Allows sorting, first by type then by index.
 *
 * @param lhs Left-hand side symbol reference.
 * @param rhs Right-hand side symbol reference.
 * @return True if lhs is less than rhs.
 */
inline bool operator<(const symbol_ref& lhs, const symbol_ref& rhs)
{
    return std::tie(lhs.type_, lhs.index_) < std::tie(rhs.type_, rhs.index_);
}

} // namespace muskox
