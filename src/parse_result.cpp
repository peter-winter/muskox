#include "parse_result.h"

#include "list_printer.h"

#include <utility>

namespace muskox
{

parse_result::parse_result(bool success, std::vector<std::string> errors)
    : success_(success), errors_(std::move(errors))
{
}

parse_result parse_result::ok()
{
    return parse_result(true, {});
}

parse_result parse_result::error(std::vector<std::string> errors)
{
    return parse_result(false, std::move(errors));
}

bool parse_result::is_success() const
{
    return success_;
}

const std::vector<std::string>& parse_result::get_errors() const
{
    return errors_;
}

std::string parse_result::errors_to_string() const
{
    list_printer lp("", "\n", "");
    return lp.print_container(errors_, [](const std::string& s){ return s; });
}

} // namespace muskox
