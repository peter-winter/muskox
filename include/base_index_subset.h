#pragma once

#include <vector>
#include <cstddef>

namespace ptg
{

class base_index_subset
{
protected:
    std::vector<bool> bits_;

public:
    base_index_subset(size_t n, bool value = false);
    ~base_index_subset() = default;

    bool add(size_t index);  // Returns true if newly added
    bool contains(size_t index) const;

    size_t get_size() const;
};

} // namespace ptg
