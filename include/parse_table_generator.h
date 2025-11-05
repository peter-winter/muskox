#pragma once

#include "closure.h"
#include "ruleset.h"
#include "lr1_state.h"

#include <vector>
#include <string>

#include <parse_table.h>

namespace muskox
{

class table_entry_hint
{
private:
    size_t state_idx_;
    symbol_ref ref_;
    parse_table_entry entry_;

public:    
    table_entry_hint() = default;

    table_entry_hint(size_t state_idx, symbol_ref ref, parse_table_entry entry)
        : state_idx_(state_idx), ref_(ref), entry_(entry)
    {}
    
    bool operator == (const table_entry_hint& other) const
    {
        return state_idx_ == other.state_idx_ && ref_ == other.ref_ && entry_ == other.entry_;
    }
    
    parse_table_entry get_entry() const { return entry_; }
    symbol_ref get_ref() const { return ref_; }
    size_t get_state_idx() const { return state_idx_; }
};

class parse_table_generator
{    
public:
    parse_table_generator(const ruleset& rs);
    ~parse_table_generator() = default;

    const std::vector<std::string>& get_warnings() const;
    
    std::string states_to_string() const;
    
    const std::vector<lr1_state>& get_states() const;
    
    const std::vector<table_entry_hint>& get_table_entry_hints() const;
    
    parse_table create_parse_table() const;
    
private:
    const ruleset& rs_;
    closure cl_;
    std::vector<std::string> warnings_;
    std::vector<lr1_state> states_;
    
    std::vector<table_entry_hint> table_entry_hints_;
    std::vector<parse_table_entry> glr_splits_;

    void collect_conflict_warnings(
        size_t state_idx, 
        size_t lookahead_idx, 
        const lr1_state::conflict& c, 
        std::optional<size_t> prefered_idx_reduce, 
        std::optional<size_t> shift_over_reduce_state_idx);
    
    void generate_states();
    
    std::optional<size_t> find_state(const ordered_bitset_nd<4>& kernel) const;
    
    size_t process_shift(size_t state_idx, symbol_ref ref, const lr1_state::shift& s);
    void process_reduce(size_t state_idx, size_t lookahead_idx, const lr1_state::reduction& r);
    void process_conflict(size_t state_idx, size_t term_idx, const lr1_state::conflict& c);
    bool shift_over_reduce(size_t term_idx, const lr1_state::reduction& r) const;
};

} // namespace muskox
