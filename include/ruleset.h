/**
 * @file ruleset.h
 * @brief Defines the ruleset class for managing grammar production rules.
 *
 * This header provides the ruleset class, which stores and manages the production
 * rules of a grammar, including left-hand non-terminals and right-hand symbol lists.
 * It supports adding rules, setting the root, querying dimensions and counts,
 * and generating string representations. Validation methods ensure index integrity.
 * Additionally, it computes nullable non-terminals incrementally during rule addition
 * and all nullable rule suffixes during validation.
 *
 * Integrates with symbol_collection for symbol references.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "symbol_collection.h"
#include "symbol_list.h"
#include "lr1_set_item.h"
#include "base_index_subset.h"
#include "rside.h"

#include <string_view>
#include <vector>
#include <array>
#include <optional>

namespace muskox
{

/**
 * @class ruleset
 * @brief Manages production rules in a grammar.
 *
 * Stores rules as left-hand non-terminals mapping to lists of right-hand sides.
 * Each right-hand side is a list of symbols with optional precedence.
 * Provides methods for adding rules, setting the root, querying structure,
 * and string conversion. Ensures consistency with the symbol collection.
 * Computes nullable non-terminals incrementally upon adding rules and
 * nullable suffixes on demand.
 */
class ruleset
{
public:
    /**
     * @brief Constructs a ruleset with a symbol collection.
     *
     * Initializes with the given symbols, sets root to first user non-terminal.
     *
     * @param symbols The symbol collection (must be validated).
     * @throw std::runtime_error If symbols not validated.
     */
    ruleset(const symbol_collection& symbols);

    /**
     * @brief Destructor.
     */
    ~ruleset() = default;

    /**
     * @brief Validates the ruleset.
     *
     * Adds implicit $root rule, checks for non-terminals without rules.
     * Computes all nullable rule suffixes.
     *
     * @throw grammar_error On validation errors.
     * @throw std::runtime_error If unexpected $root rules.
     */
    void validate();

    /**
     * @brief Checks if the ruleset has been validated.
     *
     * @return True if validated.
     */
    bool is_validated() const;

    /**
     * @brief Sets the root non-terminal by name.
     *
     * @param name The name of the root non-terminal.
     * @return The root reference.
     * @throw grammar_error If invalid name or not a non-terminal.
     */
    symbol_ref set_root(std::string_view name);

    /**
     * @brief Adds a production rule by names.
     *
     * Validates inputs, adds to rules, updates nullables incrementally.
     *
     * @param left Left-hand non-terminal name.
     * @param rights Right-hand symbol names.
     * @param precedence Optional precedence.
     * @return New right-hand side index for the non-terminal.
     * @throw grammar_error On invalid inputs or duplicates.
     */
    size_t add_rule(std::string_view left, const std::vector<std::string_view>& rights, std::optional<size_t> precedence = std::nullopt);

    /**
     * @brief Gets the root non-terminal reference.
     *
     * @return The root reference.
     */
    symbol_ref get_root() const;

    /**
     * @brief Gets the number of non-terminals.
     *
     * @return The count.
     */
    size_t get_nterm_count() const;

    /**
     * @brief Gets the number of terminals.
     *
     * @return The count.
     */
    size_t get_term_count() const;

    /**
     * @brief Gets the sum of the number of terminals and non-terminals.
     *
     * @return The sum of terminal and non-terminal counts.
     */
    size_t get_term_plus_nterm_count() const;

    /**
     * @brief Gets the number of right-hand sides for a non-terminal.
     *
     * @param nterm_idx The non-terminal index.
     * @return The count.
     * @throw std::out_of_range If nterm_idx invalid.
     */
    size_t get_nterm_rside_count(size_t nterm_idx) const;

    /**
     * @brief Gets the maximum number of right-hand sides across non-terminals.
     *
     * @return The maximum count.
     */
    size_t get_max_rside_count() const;

    /**
     * @brief Gets the number of symbols in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @return The count.
     * @throw std::out_of_range If indices invalid.
     */
    size_t get_symbol_count(size_t nterm_idx, size_t rside_idx) const;

    /**
     * @brief Gets the maximum number of symbols across all right-hand sides.
     *
     * @return The maximum count.
     */
    size_t get_max_symbol_count() const;

    /**
     * @brief Gets a symbol reference in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param suffix_idx The suffix index.
     * @return The symbol reference.
     * @throw std::out_of_range If indices invalid.
     */
    symbol_ref get_symbol(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;

    /**
     * @brief Gets the type of a symbol in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param suffix_idx The suffix index.
     * @return The symbol type.
     * @throw std::out_of_range If indices invalid.
     */
    symbol_type get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;

    /**
     * @brief Gets the index of a symbol in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param suffix_idx The suffix index.
     * @return The symbol's index in its collection.
     * @throw std::out_of_range If indices invalid.
     */
    size_t get_symbol_index(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;

    /**
     * @brief Gets the name of a non-terminal.
     *
     * @param nterm_idx The index.
     * @return The name.
     * @throw std::out_of_range If invalid.
     */
    std::string_view get_nterm_name(size_t nterm_idx) const;

    /**
     * @brief Gets the name of a terminal.
     *
     * @param term_idx The index.
     * @return The name.
     * @throw std::out_of_range If invalid.
     */
    std::string_view get_term_name(size_t term_idx) const;

    /**
     * @brief Gets the precedence of a terminal.
     *
     * @param term_idx The index.
     * @return The precedence (0 if none).
     * @throw std::out_of_range If invalid.
     */
    size_t get_term_prec(size_t term_idx) const;

    /**
     * @brief Gets the associativity of a terminal.
     *
     * @param term_idx The index.
     * @return The associativity type.
     * @throw std::out_of_range If invalid.
     */
    associativity::type get_term_assoc(size_t term_idx) const;

    /**
     * @brief Gets the explicit precedence of a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @return Optional precedence.
     * @throw std::out_of_range If indices invalid.
     */
    std::optional<size_t> get_rside_precedence(size_t nterm_idx, size_t rside_idx) const;

    /**
     * @brief Calculates the effective precedence of a right-hand side.
     *
     * Uses the explicit precedence if set; otherwise, scans the right-hand side from right to left
     * and returns the precedence of the first (rightmost) terminal that has a defined precedence.
     * Returns 0 if no explicit precedence is set and no terminal in the right-hand side has a defined precedence.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @return The effective precedence (0 if none).
     * @throw std::out_of_range If indices invalid.
     */
    size_t calculate_rside_precedence(size_t nterm_idx, size_t rside_idx) const;

    /**
     * @brief Gets dimensions for suffixes.
     *
     * @return Array of {nterms, max rsides, max symbols}.
     */
    std::array<size_t, 3> get_suffix_space_dims() const;

    /**
     * @brief Gets dimensions for LR(1) set items.
     *
     * @return Array of {nterms, max rsides, max symbols + 1, terms}.
     */
    std::array<size_t, 4> get_lr1_set_item_space_dims() const;
    
    /**
     * @brief Checks if a rule suffix is nullable.
     *
     * Requires validation to have been called.
     *
     * @param nterm_idx Non-terminal index.
     * @param rside_idx Right-hand side index.
     * @param suffix_idx Start index.
     * @return True if nullable.
     * @throw std::out_of_range If invalid or not validated.
     */
    bool is_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;

    /**
     * @brief Checks if a non-terminal is nullable.
     *
     * @param idx Index.
     * @return True if nullable.
     * @throw std::out_of_range If invalid.
     */
    bool is_nterm_nullable(size_t idx) const;

    /**
     * @brief Converts the ruleset to a string representation.
     *
     * @return The string.
     */
    std::string to_string() const;

    /**
     * @brief Converts an LR(1) set item to a string.
     *
     * @param item The item.
     * @return The string representation.
     */
    std::string lr1_set_item_to_string(const lr1_set_item& item) const;

    /**
     * @brief Validates a terminal index.
     *
     * @param term_idx The index to validate.
     * @throw std::out_of_range If invalid.
     */
    void validate_term_idx(size_t term_idx) const;

    /**
     * @brief Validates a non-terminal index.
     *
     * @param nterm_idx The index to validate.
     * @throw std::out_of_range If invalid.
     */
    void validate_nterm_idx(size_t nterm_idx) const;

    /**
     * @brief Validates a right-hand side index for a non-terminal.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @throw std::out_of_range If invalid.
     */
    void validate_rside_idx(size_t nterm_idx, size_t rside_idx) const;

    /**
     * @brief Validates a suffix index in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param suffix_idx The suffix index.
     * @throw std::out_of_range If invalid.
     */
    void validate_suffix_idx(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;

private:
    const symbol_collection& symbols_; /// Reference to the symbol collection.
    std::vector<std::vector<rside>> rsides_; /// Rules indexed by non-terminal.
    symbol_ref root_ = {}; /// The root non-terminal reference.
    bool validated_ = false; /// Flag indicating if validated.

    // Incremental nullable non-terminals
    struct potentially_empty_rside
    {
        size_t nterm_idx_;  /// left side nonterminal
        size_t rside_idx_;  /// rside index (all symbols are nonterminals in this rside)
        size_t remaining_;  /// number of nonterminals in rside still not determined
    };

    std::vector<potentially_empty_rside> potentially_empty_rsides_; /// Potentially empty productions.
    std::vector<std::vector<size_t>> appearances_in_pot_rsides_; /// Reverse index: nterm to potential rside indices.
    base_index_subset<1> nullable_nterms_; /// Nullable non-terminals.

    std::optional<base_index_subset<3>> nullable_suffixes_; /// Nullable rule suffixes, precalculated during validation, optional for lazy init.

    /**
     * @brief Propagates nullability when a non-terminal becomes nullable.
     *
     * @param nt_idx The newly nullable non-terminal index.
     */
    void propagate_nullable(size_t nt_idx);

    /**
     * @brief Internal computation for suffix nullability.
     *
     * @param nterm_idx Non-terminal index.
     * @param rside_idx Right-hand side index.
     * @param suffix_idx Starting suffix index.
     * @return True if nullable.
     */
    bool compute_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx);

    /**
     * @brief Validates inputs for adding a rule.
     *
     * @param left The left-hand name.
     * @param rights The right-hand names.
     * @return Pair of lref and rrefs.
     * @throw grammar_error On invalid inputs.
     */
    std::pair<symbol_ref, symbol_list> validate_add_rule_inputs(std::string_view left, const std::vector<std::string_view>& rights) const;

    /**
     * @brief Internal addition of rside and nullable update.
     *
     * @param lhs_idx LHS non-terminal index.
     * @param symbols RHS symbols.
     * @param precedence Optional precedence.
     * @return New rside index.
     */
    size_t add_rside_impl(size_t lhs_idx, symbol_list symbols, std::optional<size_t> precedence = std::nullopt);

    /**
     * @brief Computes all suffix nullabilities.
     */
    void compute_all_suffixes_nullable();
    
    /**
     * @brief Computes and gets if rule suffix is nullable (on demand).
     *
     * @param nterm_idx Non-terminal index.
     * @param rside_idx Right-hand side index.
     * @param suffix_idx Start index.
     * @return True if nullable.
     * @throw std::out_of_range If invalid indices.
     */
    bool calculate_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx);
    
    /**
     * @brief Internal method to set the root by reference.
     *
     * Just sets root_, no rule addition.
     *
     * @param root The root symbol reference.
     * @return The set root reference.
     */
    symbol_ref set_root_impl(symbol_ref root);
};

} // namespace muskox
