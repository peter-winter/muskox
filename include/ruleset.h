/**
 * @file ruleset.h
 * @brief Defines the ruleset class for managing grammar production rules.
 *
 * This header provides the ruleset class, which stores and manages the production
 * rules of a grammar, including left-hand non-terminals and right-hand symbol lists.
 * It supports adding rules, setting the root, querying dimensions and counts,
 * and generating string representations. Validation methods ensure index integrity.
 * Additionally, it computes nullable non-terminals and all nullable rule suffixes incrementally during rule addition.
 *
 * Integrates with symbol_collection for symbol references.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "symbol_collection.h"
#include "symbol_list.h"
#include "lr1_set_item.h"
#include "ordered_bitset_nd.h"
#include "rside.h"
#include "nterm_data.h"
#include "vector_nd.h"
#include "defs.h"

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
 * Computes nullable non-terminals all nullable rule suffixes incrementally upon adding rules .
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
    explicit ruleset(const symbol_collection& symbols);

    /**
     * @brief Destructor.
     */
    ~ruleset() = default;

    /**
     * @brief Validates the ruleset.
     *
     * Adds implicit $root rule.
     * Checks for issues in symbol collection.
     *
     * @return Number of errors.
     * @throw std::runtime_error If unexpected $root rules.
     * @throw std::runtime_error If validated twice.
     */
    size_t validate();

    /**
     * @brief Checks if the ruleset has been validated.
     *
     * @return True if validated.
     */
    bool is_validated() const;

    /**
     * @brief Sets the root non-terminal by name.
     * 
     * Requires validation to not have been called.
     *
     * @param name The name of the root non-terminal.
     * @return The root reference.
     * @throw grammar_error If invalid name or not a non-terminal.
     * @throw std::runtime_error If called after validation.
     */
    symbol_ref set_root(std::string_view name);

    /**
     * @brief Adds a production rule by names.
     *
     * Validates inputs, adds to rules, updates nullables incrementally.
     * Requires validation to not have been called.
     * 
     * @param left Left-hand non-terminal name.
     * @param rights Right-hand symbol names.
     * @param precedence Optional precedence.
     * @return New right-hand side index for the non-terminal.
     * @throw grammar_error On invalid inputs or duplicates.
     * @throw std::runtime_error If called after validation.
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
     * @brief Gets the symbol reference from the LR(1) set item that is either after '.' or lookahead if '.' at the end.
     *
     * @param it LR(1) set item.
     * @return The symbol reference.
     * @throw std::out_of_range If indices invalid.
     */
    symbol_ref get_symbol_of_interest(const lr1_set_item& it) const;
    
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
     * @brief Gets the effective precedence of a right-hand side.
     * 
     * Calls calculate_rside_precedence and stores it in rside for future calls.
     * Requires validation to have been called.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @return Precedence.
     * @throw std::out_of_range If indices invalid.
     * @throw std::runtime_error If called before validation.
     */
    size_t get_effective_rside_precedence(size_t nterm_idx, size_t rside_idx) const;
    
    /**
     * @brief Gets the explicit precedence of a right-hand side.
     * 
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @return Optional precedence.
     * @throw std::out_of_range If indices invalid.
     */
    std::optional<size_t> get_explicit_rside_precedence(size_t nterm_idx, size_t rside_idx) const;
    
    /**
     * @brief Gets dimensions for suffixes.
     *
     * Requires validation to have been called.
     * 
     * @return Array of {nterms, max rsides, max symbols}.
     * @throw std::runtime_error If called before validation.
     */
    std::array<size_t, 3> get_suffix_space_dims() const;

    /**
     * @brief Gets dimensions for LR(1) set items.
     *
     * Requires validation to have been called.
     * 
     * @return Array of {nterms, max rsides, max symbols + 1, terms}.
     * @throw std::runtime_error If called before validation.
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
     * @throw std::out_of_range If invalid.
     * @throw std::runtime_error If called before validation.
     */
    bool is_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;

    /**
     * @brief Checks if a non-terminal is nullable.
     * 
     * Requires validation to have been called.
     *
     * @param nterm_idx Index.
     * @return True if nullable.
     * @throw std::out_of_range If invalid.
     * @throw std::runtime_error If called before validation.
     */
    bool is_nterm_nullable(size_t nterm_idx) const;
    
    /**
     * @brief Gets a suffix's FIRST set.
     *
     * Requires validation to have been called.
     *
     * @param nterm_idx Non-terminal index.
     * @param rside_idx Right-hand side index.
     * @param suffix_idx Start index.
     * @return FIRST set of the suffix. Returns std::nullopt when no FIRST set for suffix, either suffix with pure epsilon non-terminals, or caused by unsolvable left recursion.
     * @throw std::out_of_range If invalid.
     * @throw std::runtime_error If called before validation.
     */
    const std::optional<first_set>& get_suffix_first(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const;
    
    /**
     * @brief Gets a non-terminal's FIRST set
     * 
     * Requires validation to have been called.
     *
     * @param nterm_idx Index.
     * @return FIRST set of the non-terminal.
     * @throw std::out_of_range If invalid.
     * @throw std::runtime_error If called before validation.
     * @throw std::runtime_error no FIRST set for non-terminal, either one and only epsilon production, or unsolvable left recursion.
     */
    const first_set& get_nterm_first(size_t nterm_idx) const;

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
     * @brief Converts an LR(1) set to a string.
     *
     * @param s The set.
     * @return The string representation.
     */
    std::string lr1_set_to_string(const lr1_set& s) const;

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
    
    /**
     * @brief Gets the errors.
     *
     * @return The vector of errors.
     */
    const std::vector<std::string>& get_errors() const;

    /**
     * @brief Gets the warnings.
     *
     * @return The vector of warnings.
     */
    const std::vector<std::string>& get_warnings() const;

private:
    const symbol_collection& symbols_; /// Reference to the symbol collection.
    std::vector<nterm_data> nterms_data_; /// Data for each non-terminal.
    symbol_ref root_ = {}; /// The root non-terminal reference.
    bool validated_ = false; /// Flag indicating if validated.
    bitset_nd<1> nullable_nterms_; /// Nullable non-terminals.
    std::vector<std::string> errors_; /// Vector of errors.
    std::vector<std::string> warnings_; /// Vector of warnings.
    
    /**
     * @brief Propagates nullability when a non-terminal becomes nullable.
     *
     * @param nt_idx The newly nullable non-terminal index.
     */
    void propagate_nullable(size_t nt_idx);
    
    /**
     * @brief Propagates terminal added to non-terminal FIRST set.
     *
     * @param nt_idx The non-terminal which FIRST set was modified.
     * @param t_idx The terminal added to FIRST set
     */
    void propagate_added_to_first_set(size_t nt_idx, size_t t_idx);

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
     * @brief Internal method to set the root by reference.
     *
     * Just sets root_, no rule addition.
     *
     * @param root The root symbol reference.
     * @return The set root reference.
     */
    symbol_ref set_root_impl(symbol_ref root);
    
    /**
     * @brief Calculates the effective precedence of a right-hand side and stores the result for future use.
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
    size_t calculate_effective_rside_precedence(size_t nterm_idx, size_t rside_idx);
    
    /**
     * @brief If nullopt passed, creates FIRST set, then adds an element with indices in array.
     *
     * @param opt Optional first set
     * @param t_dix The terminal to add.
     * @return True if newly added terminal
     */
    bool first_set_add_with_lazy_init(std::optional<first_set>& opt, size_t t_idx);
    
    /**
     * @brief If nullopt passed, creates FIRST set, then unions the 'other' set into 'opt' one
     *
     * @param opt Optional first set
     * @param other The other set (also optional) to union into 'opt'.
     * @return New size of opt
     */
    size_t first_set_add_with_lazy_init(std::optional<first_set>& opt, const std::optional<first_set>& other);
    
    /**
     * @brief Tests if symbol collection was validated and error free
     *
     * @param sc Symbol collection to test
     * @return The symbol collection passed as const reference
     * @throw std::runtime_error If not validated or has errors.
     */
    const symbol_collection& test_symbol_collection_validated(const symbol_collection& sc) const;
    
    /**
     * @brief Checks for non-terminals with no rsides
     */
    void check_nterm_no_rsides();
    
    /**
     * @brief Checks for unused symbols
     */
    void check_usused_symbols();
    
    /**
     * @brief Checks for unsolvable non-terminals. 
     * 
     * It means unresolved left recursion, no productions for  non-terminal is reported from check_nterm_no_rsides
     */
    void check_unsolvable_nterms();
};

} // namespace muskox
