/**
 * @file parse_table.h
 * @brief Defines the parse_table class for storing LR(1) parsing actions.
 *
 * This header provides the parse_table class, which represents the action and goto table
 * for an LR(1) parser. It uses a 2D vector to store entries indexed by states and symbols
 * (terminals followed by non-terminals). Entries can represent shifts, gotos, reductions,
 * or references to reduce-reduce conflicts. An auxiliary rr_table type is defined for
 * handling multiple reductions in reduce-reduce conflicts, but it is managed separately.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "vector_nd.h"
#include "parse_table_entry.h"

namespace muskox
{

/**
 * @class parse_table
 * @brief Represents the LR(1) parsing table.
 *
 * Stores shift actions for terminals, goto actions for non-terminals, and reduce actions.
 * The table is a 2D structure with rows as states and columns as symbols (terminals first,
 * then non-terminals). For reduce-reduce conflicts, entries may reference an external rr_table.
 * Provides accessors with bounds checking.
 */
class parse_table
{
public:
    /**
     * @brief Constructs the parse table with given dimensions.
     *
     * Initializes the table with default-constructed entries.
     *
     * @param term_count Number of terminals.
     * @param nterm_count Number of non-terminals.
     * @param state_count Number of states.
     */
    parse_table(size_t term_count, size_t nterm_count, size_t state_count);

    /**
     * @brief Destructor.
     */
    ~parse_table() = default;

    /**
     * @brief Gets a mutable reference to the term part entry for a state and terminal.
     *
     * @param state The state index.
     * @param term_idx The terminal index.
     * @return Mutable reference to the entry.
     * @throw std::out_of_range If indices are invalid.
     */
    parse_table_entry& get_term_entry(size_t state, size_t term_idx);

    /**
     * @brief Gets a const reference to the term part entry for a state and terminal.
     *
     * @param state The state index.
     * @param term_idx The terminal index.
     * @return Const reference to the entry.
     * @throw std::out_of_range If indices are invalid.
     */
    const parse_table_entry& get_term_entry(size_t state, size_t term_idx) const;

    /**
     * @brief Gets a mutable reference to the nterm part entry for a state and non-terminal.
     *
     * @param state The state index.
     * @param nterm_idx The non-terminal index.
     * @return Mutable reference to the entry.
     * @throw std::out_of_range If indices are invalid.
     */
    parse_table_entry& get_nterm_entry(size_t state, size_t nterm_idx);

    /**
     * @brief Gets a const reference to the nterm part entry for a state and non-terminal.
     *
     * @param state The state index.
     * @param nterm_idx The non-terminal index.
     * @return Const reference to the entry.
     * @throw std::out_of_range If indices are invalid.
     */
    const parse_table_entry& get_nterm_entry(size_t state, size_t nterm_idx) const;
    
    /**
     * @brief Gets the number of states.
     *
     * @return The state count.
     */
    size_t get_state_count() const;

    /**
     * @brief Gets the total number of symbols (terminals + non-terminals).
     *
     * @return The symbol count.
     */
    size_t get_symbol_count() const;
    
private:
    /**
     * @brief Validates a state index.
     *
     * @param idx The index to validate.
     * @throw std::out_of_range If invalid.
     */
    void validate_state_idx(size_t idx) const;

    /**
     * @brief Validates a terminal index.
     *
     * @param idx The index to validate.
     * @throw std::out_of_range If invalid.
     */
    void validate_term_idx(size_t idx) const;

    /**
     * @brief Validates a non-terminal index.
     *
     * @param idx The index to validate.
     * @throw std::out_of_range If invalid.
     */
    void validate_nterm_idx(size_t idx) const;
    
    size_t term_count_; ///< Number of terminals.
    size_t nterm_count_; ///< Number of non-terminals.
    size_t state_count_; ///< Number of states.
    vector_nd<parse_table_entry, 2> table_; ///< 2D table of entries [states][symbols].
};

/**
 * @typedef rr_table
 * @brief Type alias for a vector of reductions used in reduce-reduce conflicts.
 */
using rr_table = std::vector<parse_table_entry::reduction>;

} // namespace muskox
