/**
 * @file nullable.h
 * @brief Determines nullability of non-terminals and rule parts in a grammar.
 *
 * This header defines the nullable class, which computes whether non-terminals
 * and segments of right-hand sides in production rules can be nullable (derive epsilon).
 * It uses recursive checks with cycle detection via calculating sets.
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
 * (derive epsilon). Uses memoization with bitsets for efficiency and
 * handles recursion with temporary calculating sets to detect cycles.
 */
class nullable
{
private:
    const ruleset& rs_; //!< Reference to the ruleset.
    base_index_subset<1> nterms_; //!< Bitset for nullable non-terminals.
    base_index_subset<3> rside_parts_; //!< Bitset for nullable rule parts.

    /**
     * @brief Internal recursive check for non-terminal nullability.
     *
     * @param nterm_idx The non-terminal index.
     * @param calculating_nterms Set for cycle detection in non-terminals.
     * @param calculating_rside_parts Set for cycle detection in rule parts.
     * @return True if nullable.
     */
    bool calculate_nterm_impl(
        size_t nterm_idx,
        base_index_subset<1>& calculating_nterms, 
        base_index_subset<3>& calculating_rside_parts);
        
    /**
     * @brief Internal recursive check for rule part nullability.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param symbol_start_idx Starting symbol index in the rule.
     * @param calculating_nterms Set for cycle detection in non-terminals.
     * @param calculating_rside_parts Set for cycle detection in rule parts.
     * @return True if the part is nullable.
     */
    bool calculate_rside_part_impl(
        size_t nterm_idx, 
        size_t rside_idx, 
        size_t symbol_start_idx,
        base_index_subset<1>& calculating_nterms, 
        base_index_subset<3>& calculating_rside_parts);

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
