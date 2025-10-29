#pragma once

#include <optional>
#include <vector>

namespace muskox
{

class symbol_stream
{
public:
    symbol_stream(const std::vector<size_t>& terminals);

    std::optional<size_t> next();

private:
    std::vector<size_t> terminals_;
    size_t pos_ = 0;
};

} // namespace muskox
