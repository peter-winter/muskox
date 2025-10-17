#pragma once

#include <closure.h>
#include <ruleset.h>
#include <lr1_state.h>

#include <vector>
#include <string>


namespace ptg
{

class parse_table_generator
{
public:
    parse_table_generator(const ruleset& rs);
    ~parse_table_generator() = default;

    const std::vector<std::string>& get_warnings() const;
    
    std::string states_to_string() const;
    
private:
    const ruleset& rs_;
    closure cl_;
    std::vector<std::string> warnings_;
    std::vector<lr1_state> states_;

    const ruleset& validate(const ruleset& rs) const;
    void collect_unused_warnings();
    void generate_states();
    
    std::optional<size_t> find_state(const index_subset<4>& kernel) const;
    
    void process_shift(size_t term_idx, const lr1_state::shift& s);
    void process_reduce(size_t term_idx, const lr1_state::reduction& r);
    void process_conflict(size_t term_idx, const lr1_state::conflict& c);
};

} // namespace ptg
