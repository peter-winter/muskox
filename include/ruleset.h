/**
 * @file ruleset.h
 * @brief Defines the ruleset class for managing grammar production rules.
 *
 * This header provides the ruleset class, which stores and manages the production
 * rules of a grammar, including left-hand non-terminals and right-hand symbol lists.
 * It supports adding rules, setting the root, querying dimensions and counts,
 * and generating string representations. Validation methods ensure index integrity.
 *
 * Integrates with symbol_collection for symbol references.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "symbol_collection.h"
#include "symbol_list.h"
#include "lr1_set_item.h"

#include <string_view>
#include <vector>
#include <array>

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
 */
class ruleset
{
private:
    /**
     * @struct rside
     * @brief Represents a right-hand side of a production rule.
     */
    struct rside
    {
        symbol_list symbols_; //!< List of symbols in the right-hand side.
        std::optional<size_t> precedence_; //!< Optional precedence for the rule.

        /**
         * @brief Constructs a right-hand side.
         *
         * @param symbols List of symbols (default empty).
         * @param precedence Optional precedence (default nullopt).
         */
        rside(symbol_list symbols = {}, std::optional<size_t> precedence = std::nullopt)
            : symbols_(std::move(symbols)), precedence_(precedence)
        {}
    };

    const symbol_collection& symbols_; //!< Reference to the symbol collection.
    std::vector<std::vector<rside>> rsides_; //!< Rules indexed by non-terminal.
    symbol_ref root_; //!< The root non-terminal reference.

    /**
     * @brief Internal method to set the root by reference.
     *
     * @param root The root symbol reference.
     * @return The set root reference.
     */
    symbol_ref set_root(symbol_ref root);

public:
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
     * @brief Validates a symbol index in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param symbol_idx The symbol index.
     * @throw std::out_of_range If invalid.
     */
    void validate_symbol_idx(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    /**
     * @brief Constructs the ruleset with a symbol collection.
     *
     * Initializes with the given symbols and sets up the root rule.
     *
     * @param symbols The symbol collection.
     * @throw grammar_error If no non-terminals exist (besides $root).
     */
    ruleset(const symbol_collection& symbols);

    /**
     * @brief Destructor.
     */
    ~ruleset() = default;

    /**
     * @brief Sets the root non-terminal by name.
     *
     * @param name The name of the root non-terminal.
     * @return The root symbol reference.
     * @throw grammar_error If name is special, doesn't exist, or is a terminal.
     */
    symbol_ref set_root(std::string_view name);

    /**
     * @brief Adds a production rule.
     *
     * @param left The left-hand non-terminal name.
     * @param rights The right-hand symbol names.
     * @param precedence Optional precedence.
     * @return The index of the added right-hand side.
     * @throw grammar_error If invalid names, non-existent symbols, or left is terminal/special.
     */
    size_t add_rule(std::string_view left, const std::vector<std::string_view>& rights, std::optional<size_t> precedence = std::nullopt);

    /**
     * @brief Gets the root symbol reference.
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
     * @brief Gets the total number of symbols.
     *
     * @return The count.
     */
    size_t get_symbol_count() const;

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
     * @param symbol_idx The symbol index.
     * @return The symbol reference.
     * @throw std::out_of_range If indices invalid.
     */
    symbol_ref get_symbol(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    /**
     * @brief Gets the type of a symbol in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param symbol_idx The symbol index.
     * @return The symbol type.
     * @throw std::out_of_range If indices invalid.
     */
    symbol_type get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    /**
     * @brief Gets the index of a symbol in a right-hand side.
     *
     * @param nterm_idx The non-terminal index.
     * @param rside_idx The right-hand side index.
     * @param symbol_idx The symbol index.
     * @return The symbol's index in its collection.
     * @throw std::out_of_range If indices invalid.
     */
    size_t get_symbol_index(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

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
     * @brief Gets dimensions for right-hand side parts.
     *
     * @return Array of {nterms, max rsides, max symbols}.
     */
    std::array<size_t, 3> get_rside_part_space_dims() const;

    /**
     * @brief Gets dimensions for LR(1) set items.
     *
     * @return Array of {nterms, max rsides, max symbols + 1, terms}.
     */
    std::array<size_t, 4> get_lr1_set_item_space_dims() const;

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
};

} // namespace muskox
