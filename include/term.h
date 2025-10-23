#pragma once

#include <associativity.h>

#include <string_view>
#include <optional>

namespace ptg
{

class term
{
public:
    term(std::string_view name, std::optional<size_t> prec = std::nullopt, associativity assoc = associativity::left())
        : name_(name), assoc_(assoc), prec_(prec)
    {
    }
    ~term() = default;

    std::string_view name() const { return name_; }
    associativity assoc() const { return assoc_; }
    std::optional<size_t> prec() const { return prec_; }

private:
    std::string_view name_;
    associativity assoc_;
    std::optional<size_t> prec_;
};

} // namespace ptg
