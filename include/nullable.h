/**
 * @file nullable.h
 * @brief Determines nullability of non-terminals and rule parts in a grammar.
 *
 * This header defines the nullable class, which computes whether non-terminals
 * and segments of right-hand sides in production rules can be nullable (derive epsilon).
 * Supports individual and bulk calculations for efficiency in parsing algorithms.
 *
 * Integrates with ruleset for grammar structure.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "base_index_subset.h"
#include "ruleset.h"

namespace muskox
{

/**
 * @class nullable
 * @brief Computes nullability for non-terminals and rule segments.
 *
 * Determines if non-terminals or parts of right-hand sides can be nullable
 * (derive epsilon). Uses memoization with bitsets for efficiency.
 */
class nullable
{
private:
    const ruleset& rs_; //!< Reference to the ruleset.
    base_index_subset<1> nullable_nterms_; //!< Bitset for nullable non-terminals.
    base_index_subset<3> non_nullable_rside_parts_; //!< Bitset for non-nullable rule parts.
    base_index_subset<1> nullable_nterms_completed_; //!< Bitset for calculate_nterm memoization.
    base_index_subset<3> non_nullable_rside_parts_completed_; //!< Bitset for calculate_rside_part memoization.

public:
    /**
     * @brief Constructs the nullable analyzer with a ruleset.
     *
     * @param rs The ruleset to analyze.
     */
    nullable(const ruleset& rs);

    /**
     * @brief Destructor.
     */
    ~nullable() = default;
    
    /**
     * @brief Computes nullability for a specific non-terminal.
     *
     * @param nterm_idx The non-terminal index.
     * @return True if nullable.
     */
    bool calculate_nterm(size_t nterm_idx);

    /**
     * @brief Computes nullability for a rule segment starting at a symbol.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param symbol_idx The starting symbol index.
     * @return True if the segment is nullable.
     */
    bool calculate_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx);

    /**
     * @brief Computes nullability for all non-terminals and rule parts.
     */
    void calculate_all();
    
    /**
     * @brief Checks if a non-terminal is nullable.
     *
     * @param idx The non-terminal index.
     * @return True if nullable.
     * @throw std::out_of_range If index invalid.
     */
    bool is_nullable_nterm(size_t idx) const;

    /**
     * @brief Checks if a rule segment is nullable.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param symbol_idx The starting symbol index.
     * @return True if nullable.
     * @throw std::out_of_range If indices invalid.
     */
    bool is_nullable_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
};

} // namespace muskox
