/**
 * @file associativity.h
 * @brief Defines associativity types for operator precedence in grammars.
 *
 * This header provides an enumeration and class for representing left or right
 * associativity, commonly used in parsing to resolve ambiguities in expressions.
 * It includes factory methods for creating instances and conversion to string
 * for debugging or output purposes.
 *
 * Associativity is key in handling operator precedence and reducing shift-reduce
 * conflicts in parser generation.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <string>

namespace muskox
{

/**
 * @class associativity
 * @brief Represents the associativity of a terminal (left or right).
 */
class associativity
{
public:
    /**
     * @brief Enumeration for associativity types.
     */
    enum class type { left, right };

    /**
     * @brief Constructs an associativity instance.
     *
     * @param t The type (left or right).
     */
    associativity(type t) : type_(t) {}

    /**
     * @brief Factory method for left associativity.
     *
     * @return associativity instance set to left.
     */
    static associativity left()
    {
        return associativity(type::left);
    }

    /**
     * @brief Factory method for right associativity.
     *
     * @return associativity instance set to right.
     */
    static associativity right()
    {
        return associativity(type::right);
    }

    /**
     * @brief Converts the associativity to a string.
     *
     * @return "left", "right", or "unknown".
     */
    std::string to_string() const
    {
        switch (type_)
        {
        case type::left: return "left";
        case type::right: return "right";
        }
        return "unknown";
    }

    /**
     * @brief Gets the underlying type.
     *
     * @return The type enum value.
     */
    type get() const { return type_; }
    
private:
    type type_; //!< The stored associativity type.
};

} // namespace muskox
