/**
 * @file formatted_error.h
 * @brief Provides templated classes for formatted error messages and exceptions.
 *
 * This header defines utility classes for creating formatted messages and runtime
 * errors using std::format. It allows for type-safe, compile-time checked error
 * templates, useful for consistent error reporting in the MuskOx project.
 *
 * The formatted_message class handles message construction, while formatted_error
 * extends std::runtime_error for exception throwing.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <string>
#include <format>
#include <utility>
#include <stdexcept>

namespace muskox
{

/**
 * @class formatted_message
 * @brief Templated class for formatted messages.
 *
 * @tparam ErrorTemplates The template struct with codes and strings.
 */
template <typename ErrorTemplates>
class formatted_message
{
private:
    std::string message_; /// The formatted message string.

public:
    using code = typename ErrorTemplates::code; /// Type alias for error code.

    /**
     * @brief Constructs the message.
     *
     * @tparam Args Argument types.
     * @param c The error code.
     * @param args Format arguments.
     */
    template <typename... Args>
    formatted_message(code c, Args&&... args)
        : message_(std::vformat(ErrorTemplates::templates_[std::to_underlying(c)], std::make_format_args(std::forward<Args>(args)...)))
    {
    }

    /**
     * @brief Implicit conversion to string.
     *
     * @return The message.
     */
    operator std::string() const
    {
        return message_;
    }

    /**
     * @brief Gets the message string.
     *
     * @return The message.
     */
    const std::string& str() const
    {
        return message_;
    }
};

/**
 * @class formatted_error
 * @brief Templated runtime error with formatted message.
 *
 * @tparam ErrorTemplates The template struct.
 */
template <typename ErrorTemplates>
class formatted_error : public std::runtime_error
{
public:
    using code = typename ErrorTemplates::code; /// Type alias for error code.

    /**
     * @brief Constructs the error.
     *
     * @tparam Args Argument types.
     * @param c The code.
     * @param args Format arguments.
     */
    template <typename... Args>
    formatted_error(code c, Args&&... args)
        : std::runtime_error(formatted_message<ErrorTemplates>(c, std::forward<Args>(args)...))
    {
    }
};

} // namespace muskox
