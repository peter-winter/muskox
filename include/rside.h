/**
 * @file rside.h
 * @brief Defines the rside struct for right-hand sides of production rules.
 *
 * This header provides the rside struct, which represents the right-hand side
 * of a production rule, including the list of symbols and optional precedence.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "symbol_list.h"

#include <optional>
#include <vector>

namespace muskox
{

/**
 * @struct rside
 * @brief Represents a right-hand side of a production rule.
 */
struct rside
{
    symbol_list symbols_; /// List of symbols in the right-hand side.
    std::optional<size_t> precedence_; /// Optional precedence for the rule.
    std::vector<size_t> potentially_nullable_suffixes_; /// Remaining counts for potentially nullable suffixes.
    std::optional<size_t> effective_precedence_; /// Effective precedence, computed in ruleset::calculate_rside_precedence. Stored for optimization.
    /**
     * @brief Constructs a right-hand side.
     *
     * @param symbols List of symbols (default empty).
     * @param precedence Optional precedence (default nullopt).
     */
    rside(symbol_list symbols = {}, std::optional<size_t> precedence = std::nullopt)
        : symbols_(std::move(symbols)), 
          precedence_(precedence), 
          effective_precedence_(std::nullopt)
    {}
};

} // namespace muskox
