/**
 * @file closure.h
 * @brief Defines the closure class for computing LR(1) item closures.
 *
 * This header provides the closure class, which computes and caches the full
 * (transitive) closures of LR(1) items based on a given ruleset.
 *
 * The class uses memoization to store computed closures for efficiency.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "ruleset.h"
#include "ordered_bitset_nd.h"
#include "lr1_set_item.h"
#include "vector_nd.h"

namespace muskox
{

/**
 * @class closure
 * @brief Computes and caches full closures of LR(1) items.
 *
 * Given a ruleset, this class calculates the full closure of an LR(1) item,
 * which includes the item itself and all items that can be derived by
 * expanding non-terminals in the production, transitively.
 *
 * Uses multi-dimensional vectors for caching results.
 */
class closure
{
public:
    /**
     * @brief Constructs a closure instance with a ruleset.
     *
     * Initializes caching structures based on the ruleset's dimensions.
     *
     * @param rs The ruleset (must be validated and error-free).
     */
    explicit closure(const ruleset& rs);
    ~closure() = default;
    
    using opt_bitset = std::optional<ordered_bitset_nd<4>>;

    /**
     * @brief Computes the full (transitive) closure of an LR(1) item.
     *
     * Expands the item by adding kernel items for non-terminals after the dot,
     * with lookaheads from FIRST(beta a), and recursively applies this to all
     * added items until no new items are added. Implemented iteratively to avoid
     * stack overflow.
     *
     * Uses memoization: caches and reuses results.
     *
     * @param item The LR(1) item to fully close.
     * @return Reference to the computed optional bitset.
     */
    const opt_bitset& calculate_full(const lr1_set_item& item);
    
private:
    /**
     * @brief Tests if the ruleset is validated and error-free.
     *
     * @param rs The ruleset to test.
     * @return The ruleset if valid.
     * @throw std::runtime_error If not validated or has errors.
     */
    const ruleset& test_ruleset_validated(const ruleset& rs) const;
    const ruleset& rs_;
    
    vector_nd<opt_bitset, 4> lr1_item_full_closures_;
};

} // namespace muskox
