#include "symbol_stream_from_vector.h"

namespace muskox
{

symbol_stream_from_vector::symbol_stream_from_vector(const std::vector<size_t>& terminals)
    : terminals_(terminals)
{
}

std::optional<size_t> symbol_stream_from_vector::next()
{
    if (pos_ >= terminals_.size())
    {
        return std::nullopt;
    }
    return terminals_[pos_++];
}

} // namespace muskox
