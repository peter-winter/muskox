#pragma once

#include "ruleset.h"
#include "refs.h"
#include "vector_nd.h"
#include "parse_table_entry.h"

namespace muskox
{

class parse_table
{
private:
    const ruleset& rs_;
    size_t state_count_;
    vector_nd<parse_table_entry, 2> table_;

    size_t get_column(const symbol_ref& ref) const;

public:
    parse_table(const ruleset& rs, size_t state_count);
    ~parse_table() = default;

    parse_table_entry& get(size_t state, const symbol_ref& ref);
    const parse_table_entry& get(size_t state, const symbol_ref& ref) const;

    size_t get_state_count() const;
    size_t get_symbol_count() const;

    void validate() const;
};

} // namespace muskox
