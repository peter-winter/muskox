#pragma once

#include <base_index_subset.h>

#include <vector>
#include <cstddef>

namespace ptg
{

class index_subset
{
private:
    base_index_subset base_;
    std::vector<size_t> indices_;

public:
    index_subset(size_t n);
    ~index_subset() = default;

    bool add(size_t index);  // Returns true if newly added
    bool contains(size_t index) const;

    size_t get_count() const;
    const std::vector<size_t>& get_indices() const;

    size_t get_size() const;
};

} // namespace ptg
