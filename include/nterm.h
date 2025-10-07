#pragma once

#include <symbol_ref.h>

#include <string_view>

namespace ptg
{

class nterm
{
public:
    nterm(std::string_view name) : name_(name) {}
    ~nterm() = default;

    std::string_view name() const { return name_; }

private:
    std::string_view name_;
};

} // namespace ptg
