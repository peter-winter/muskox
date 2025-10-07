#pragma once

#include <string>
#include <string_view>

#include <format>
#include <utility>
#include <stdexcept>

namespace ptg
{

template <typename ErrorTemplates>
class formatted_error : public std::runtime_error
{
public:
    using code = typename ErrorTemplates::code;

    template <typename... Args>
    formatted_error(code c, Args&&... args)
        : std::runtime_error(std::vformat(ErrorTemplates::templates_[std::to_underlying(c)], std::make_format_args(std::forward<Args>(args)...)))
    {
    }
};

} // namespace ptg
