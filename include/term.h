#pragma once

#include <associativity.h>

#include <string_view>

namespace ptg
{

class term
{
public:
    term(std::string_view name, associativity assoc = associativity::left(), size_t prec = 0)
        : name_(name), assoc_(assoc), prec_(prec)
    {
    }
    ~term() = default;

    std::string_view name() const { return name_; }
    associativity assoc() const { return assoc_; }
    size_t prec() const { return prec_; }

private:
    std::string_view name_;
    associativity assoc_;
    size_t prec_;
};

} // namespace ptg
