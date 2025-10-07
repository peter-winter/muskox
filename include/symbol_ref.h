#pragma once

namespace ptg
{

enum class symbol_type { terminal, non_terminal };

struct symbol_ref
{
    symbol_type type_;
    size_t index_;
};

} // namespace ptg
