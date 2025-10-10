// include/ruleset.h
#pragma once

#include <symbol_collection.h>
#include <symbol_list.h>

#include <string_view>
#include <vector>

namespace ptg
{

class ruleset
{
public:
    struct rside
    {
        symbol_list symbols_;
        size_t precedence_;

        rside(symbol_list symbols = {}, size_t precedence = 0)
            : symbols_(std::move(symbols)), precedence_(precedence)
        {
        }
    };

private:
    const symbol_collection& symbols_;
    std::vector<std::vector<rside>> rsides_;
    symbol_ref root_;

    void validate_nterm_idx(size_t nterm_idx) const;
    void validate_rside_idx(size_t nterm_idx, size_t rside_idx) const;
    void validate_symbol_idx(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
    
    symbol_ref set_root(symbol_ref root);
    
public:
    ruleset(const symbol_collection& symbols);
    ~ruleset() = default;

    symbol_ref set_root(std::string_view name);

    size_t add_rule(std::string_view left, const std::vector<std::string_view>& rights, size_t precedence = 0);

    std::string to_string() const;

    size_t get_nterm_rside_count(size_t nterm_idx) const;

    // Returns the maximum number of rsides across all nterms
    size_t get_max_rside_count() const;

    size_t get_symbol_count(size_t nterm_idx, size_t rside_idx) const;

    symbol_ref get_symbol(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    symbol_type get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
    
    size_t get_symbol_index(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    size_t get_rside_precedence(size_t nterm_idx, size_t rside_idx) const;

    // Returns the maximum number of symbols in any rside across all rsides
    size_t get_max_symbol_count() const;
    
    symbol_ref get_root() const;

    size_t get_rside_part_flat_index(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    // The ones below delegate to symbol collection
    size_t get_nterm_count() const;
    
    size_t get_term_count() const;
    
    std::string_view get_nterm_name(size_t nterm_idx) const;
};

} // namespace ptg
