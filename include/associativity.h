#pragma once

#include <string>

namespace ptg
{

class associativity
{
public:
    enum class type { left, right };

    associativity(type t) : type_(t) {}

    static associativity left()
    {
        return associativity(type::left);
    }

    static associativity right()
    {
        return associativity(type::right);
    }

    std::string to_string() const
    {
        switch (type_)
        {
        case type::left: return "left";
        case type::right: return "right";
        }
        return "unknown";
    }

private:
    type type_;
};

} // namespace ptg
