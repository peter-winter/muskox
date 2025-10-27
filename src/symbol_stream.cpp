#include "symbol_stream.h"

namespace ptg
{

symbol_stream::symbol_stream(const std::vector<size_t>& terminals)
    : terminals_(terminals)
{
}

std::optional<size_t> symbol_stream::next()
{
    if (pos_ >= terminals_.size())
    {
        return std::nullopt;
    }
    return terminals_[pos_++];
}

} // namespace ptg
