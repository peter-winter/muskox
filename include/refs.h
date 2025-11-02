/**
 * @file refs.h
 * @brief Defines references to symbols, right-hand sides, and suffixes in the grammar.
 *
 * This header introduces enumerations and structs to reference
 * symbols, right-hand sides, and suffixes by their indices. It also provides 
 * comparison operators for sorting and equality checks, useful in collections 
 * and parsing algorithms.
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
    symbol_type type_; /// The type of the symbol (terminal or non-terminal).
    size_t index_; /// The index within the type's collection.
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

/**
 * @brief Struct representing a reference to a right-hand side.
 *
 * References a specific right-hand side by non-terminal index and right-hand side index.
 */
struct rside_ref
{
    size_t nterm_idx_; /// The non-terminal index.
    size_t rside_idx_; /// The right-hand side index.
};

/**
 * @brief Struct representing a reference to a suffix.
 *
 * References a specific suffix by non-terminal index, right-hand side index, and suffix index.
 */
struct suffix_ref
{
    size_t nterm_idx_; /// The non-terminal index.
    size_t rside_idx_; /// The right-hand side index.
    size_t suffix_idx_; /// The suffix index.
};

} // namespace muskox
