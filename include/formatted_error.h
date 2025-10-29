#pragma once

#include <string>
#include <string_view>

#include <format>
#include <utility>
#include <stdexcept>

namespace muskox
{

template <typename ErrorTemplates>
class formatted_message
{
private:
    std::string message_;

public:
    using code = typename ErrorTemplates::code;

    template <typename... Args>
    formatted_message(code c, Args&&... args)
        : message_(std::vformat(ErrorTemplates::templates_[std::to_underlying(c)], std::make_format_args(std::forward<Args>(args)...)))
    {
    }

    operator std::string() const
    {
        return message_;
    }

    const std::string& str() const
    {
        return message_;
    }
};

template <typename ErrorTemplates>
class formatted_error : public std::runtime_error
{
public:
    using code = typename ErrorTemplates::code;

    template <typename... Args>
    formatted_error(code c, Args&&... args)
        : std::runtime_error(formatted_message<ErrorTemplates>(c, std::forward<Args>(args)...))
    {
    }
};

} // namespace muskox
