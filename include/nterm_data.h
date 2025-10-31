/**
 * @file nterm_data.h
 * @brief Defines a struct for non-terminal data in a grammar.
 *
 * This header provides a struct to represent additional data for non-terminal symbols,
 * including the list of right-hand sides and appearances in potentially nullable suffixes.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "rside.h"
#include "refs.h"

#include <vector>

namespace muskox
{

/**
 * @struct nterm_data
 * @brief Represents data for a non-terminal symbol.
 */
struct nterm_data
{
    std::vector<rside> rsides_; /// The right-hand sides for this non-terminal.
    std::vector<suffix_ref> appearances_in_potentially_nullable_suffixes_; /// Appearances in potentially nullable suffixes.
};

} // namespace muskox
