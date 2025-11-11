/**
 * @file parse_table_generator.h
 * @brief Defines the parse table generator class for LR(1) parsing.
 *
 * This header provides classes for generating LR(1) parse tables from a ruleset.
 * It includes a hint structure for table entries and the main generator class,
 * which computes states, handles conflicts, and produces the final parse table.
 * Warnings are collected for conflicts and resolutions.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "closure.h"
#include "ruleset.h"
#include "lr1_state.h"
#include "parse_table.h"
#include "action.h"

#include <vector>
#include <string>

namespace muskox
{

/**
 * @class table_entry_hint
 * @brief Represents a hint for populating the parse table.
 *
 * Stores a state index, symbol reference, and the corresponding table entry
 * (e.g., shift or reduce action). Used to build the parse table after state generation.
 */
class table_entry_hint
{
private:
    size_t state_idx_; /// The state index.
    symbol_ref ref_; /// The symbol reference (terminal or non-terminal).
    parse_table_entry entry_; /// The parse table entry (shift/reduce).

public:    
    /**
     * @brief Default constructor.
     */
    table_entry_hint() = default;

    /**
     * @brief Constructs a hint with state, symbol, and entry.
     *
     * @param state_idx The state index.
     * @param ref The symbol reference.
     * @param entry The parse table entry.
     */
    table_entry_hint(size_t state_idx, symbol_ref ref, parse_table_entry entry)
        : state_idx_(state_idx), ref_(ref), entry_(entry)
    {}
    
    /**
     * @brief Equality operator for comparing hints.
     *
     * @param other The other hint.
     * @return True if all fields match, false otherwise.
     */
    bool operator == (const table_entry_hint& other) const
    {
        return state_idx_ == other.state_idx_ && ref_ == other.ref_ && entry_ == other.entry_;
    }
    
    /**
     * @brief Gets the parse table entry.
     *
     * @return The entry.
     */
    parse_table_entry get_entry() const { return entry_; }

    /**
     * @brief Gets the symbol reference.
     *
     * @return The reference.
     */
    symbol_ref get_ref() const { return ref_; }

    /**
     * @brief Gets the state index.
     *
     * @return The state index.
     */
    size_t get_state_idx() const { return state_idx_; }
};

/**
 * @class parse_table_generator
 * @brief Generates LR(1) parse tables from a ruleset.
 *
 * Computes the canonical collection of LR(1) states, resolves conflicts using
 * precedence and associativity, collects warnings, and produces hints for the
 * parse table. The final table can be created from these hints.
 */
class parse_table_generator
{    
public:
    /**
     * @brief Constructs the generator with a ruleset.
     *
     * @param rs The ruleset to generate tables for.
     */
    parse_table_generator(const ruleset& rs);

    /**
     * @brief Destructor.
     */
    ~parse_table_generator() = default;

    /**
     * @brief Gets the collected warnings (e.g., conflicts).
     *
     * @return Const reference to the warnings vector.
     */
    const std::vector<std::string>& get_warnings() const;
    
    /**
     * @brief Converts all states to a string representation.
     *
     * @return String of all states.
     */
    std::string states_to_string() const;
    
    /**
     * @brief Gets the generated states.
     *
     * @return Const reference to the states vector.
     */
    const std::vector<lr1_state>& get_states() const;
    
    /**
     * @brief Gets the table entry hints.
     *
     * @return Const reference to the hints vector.
     */
    const std::vector<table_entry_hint>& get_table_entry_hints() const;
    
    /**
     * @brief Creates the parse table from hints.
     *
     * @return The generated parse table.
     */
    parse_table create_parse_table() const;
    
private:
    const ruleset& rs_; /// Reference to the ruleset.
    closure cl_; /// Closure computer.
    std::vector<std::string> warnings_; /// Collected warnings.
    std::vector<lr1_state> states_; /// Generated LR(1) states.
    std::vector<lr1_sorted_set> new_kernels_; /// Pending new kernels during generation.
    
    std::vector<table_entry_hint> table_entry_hints_; /// Hints for populating the table.

    /**
     * @brief Generates the LR(1) states.
     */
    void generate_states();
    
    /**
     * @brief Finds a state matching a kernel.
     *
     * @param kernel The kernel to search for.
     * @return Optional index if found.
     */
    std::optional<size_t> find_state(const lr1_sorted_set& kernel) const;
    
    /**
     * @brief Collects warnings for a conflict.
     *
     * @param state_idx The state index.
     * @param lookahead_idx The lookahead terminal index.
     * @param reds The reductions involved.
     * @param has_shift True if there's a shift.
     * @param prefered_idx_reduce Optional preferred reduce index.
     * @param shift_over_reduce_state_idx Optional shift state index for resolution.
     */
    void collect_conflict_warnings(
        size_t state_idx, 
        size_t lookahead_idx, 
        const action::reductions& reds,
        bool has_shift,
        std::optional<size_t> prefered_idx_reduce, 
        std::optional<size_t> shift_over_reduce_state_idx);
        
    /**
     * @brief Processes a shift action.
     *
     * @param state_idx The current state.
     * @param ref The symbol.
     * @param a The action.
     * @return The new state index.
     */
    std::size_t process_shift(size_t state_idx, symbol_ref ref, action& a);

    /**
     * @brief Processes a reduce action.
     *
     * @param state_idx The state index.
     * @param lookahead_idx The lookahead.
     * @param r The reduction.
     */
    void process_reduce(size_t state_idx, size_t lookahead_idx, const action::reduction& r);

    /**
     * @brief Processes a conflict.
     *
     * @param state_idx The state.
     * @param term_idx The terminal.
     * @param a The action with conflict.
     * @return Optional state index if resolved to shift.
     */
    std::optional<std::size_t> process_conflict(size_t state_idx, size_t term_idx, action& a);
    
    /**
     * @brief Checks if shift should be preferred over reduce.
     *
     * @param term_idx The terminal.
     * @param lhs_nterm_idx The left-hand non-terminal.
     * @param rside_idx The right-hand side.
     * @return True if shift over reduce.
     */
    bool shift_over_reduce(size_t term_idx, size_t lhs_nterm_idx, size_t rside_idx) const;
};

} // namespace muskox
