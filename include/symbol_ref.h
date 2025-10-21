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

inline bool operator<(const symbol_ref& lhs, const symbol_ref& rhs)
{
    return std::tie(lhs.type_, lhs.index_) < std::tie(rhs.type_, rhs.index_);
}

} // namespace ptg
