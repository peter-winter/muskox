#pragma once

#include <vector>
#include <cstddef>

namespace ptg
{

class index_subset
{
private:
    std::vector<bool> bits_;
    std::vector<size_t> indices_;

public:
    index_subset(size_t n);
    ~index_subset() = default;

    void add(size_t index);
    bool contains(size_t index) const;

    size_t get_count() const;
    const std::vector<size_t>& get_indices() const;

    size_t get_size() const;

    // Add more methods as needed.
};

} // namespace ptg
