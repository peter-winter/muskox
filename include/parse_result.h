#pragma once

#include <string>
#include <vector>

namespace muskox
{

class parse_result
{
public:
    static parse_result ok();
    static parse_result error(std::vector<std::string> errors);

    bool is_success() const;
    const std::vector<std::string>& get_errors() const;
    std::string errors_to_string() const;

private:
    parse_result(bool success, std::vector<std::string> errors);

    bool success_;
    std::vector<std::string> errors_;
};

} // namespace muskox
