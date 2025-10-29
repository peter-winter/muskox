/**
 * @file symbol_list.h
 * @brief Defines a type alias for a list of symbol references.
 *
 * This header provides a simple type alias for a vector of symbol_ref objects,
 * used to represent sequences or lists of symbols in the grammar or parsing context.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <vector>

#include "symbol_ref.h"

namespace muskox
{

/**
 * @brief Type alias for a vector of symbol references.
 *
 * This is used to store lists of symbols, such as the right-hand side of productions.
 */
using symbol_list = std::vector<symbol_ref>;

} // namespace muskox
