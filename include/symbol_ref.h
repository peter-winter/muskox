#pragma once

#include <cstddef>

namespace ptg
{

enum class symbol_type { terminal, non_terminal };

struct symbol_ref
{
    symbol_type type_;
    size_t index_;
};

inline bool operator==(const symbol_ref& lhs, const symbol_ref& rhs)
{
    return lhs.type_ == rhs.type_ && lhs.index_ == rhs.index_;
}

struct symbol_ref_hash
{
    size_t operator()(const symbol_ref& sr) const
    {
        std::size_t seed = std::hash<int>()(static_cast<int>(sr.type_));
        std::size_t hash_index = std::hash<std::size_t>()(sr.index_);
        seed ^= hash_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

} // namespace ptg
