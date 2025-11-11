#pragma once

#include "ruleset.h"
#include "refs.h"
#include "vector_nd.h"
#include "parse_table_entry.h"

namespace muskox
{

class parse_table
{
public:
    parse_table(const ruleset& rs, size_t state_count, size_t rr_table_size);
    ~parse_table() = default;

    parse_table_entry& get(size_t state, const symbol_ref& ref);
    const parse_table_entry& get(size_t state, const symbol_ref& ref) const;

    parse_table_entry::reduction& get_rr_conflict(size_t idx);
    const parse_table_entry::reduction& get_rr_conflict(size_t idx) const;
    
    size_t get_state_count() const;
    size_t get_symbol_count() const;
    size_t get_rr_conflicts_count() const;
    
    void validate() const;
    
    const std::vector<parse_table_entry::reduction>& get_rr_table() const;
    
private:
    void validate_state_idx(size_t idx) const;
    void validate_rr_table_idx(size_t idx) const;
    
    const ruleset& rs_;
    size_t state_count_;
    vector_nd<parse_table_entry, 2> table_;
    std::vector<parse_table_entry::reduction> rr_table_;
    
    size_t get_column(const symbol_ref& ref) const;
};

} // namespace muskox
