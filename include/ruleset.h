// include/ruleset.h
#pragma once

#include <symbol_collection.h>
#include <symbol_list.h>
#include <lr1_set_item.h>

#include <string_view>
#include <vector>
#include <array>

namespace ptg
{

class ruleset
{
public:
    struct rside
    {
        symbol_list symbols_;
        std::optional<size_t> precedence_;

        rside(symbol_list symbols = {}, std::optional<size_t> precedence = std::nullopt)
            : symbols_(std::move(symbols)), precedence_(precedence)
        {}
    };

private:
    const symbol_collection& symbols_;
    std::vector<std::vector<rside>> rsides_;
    symbol_ref root_;
    
    symbol_ref set_root(symbol_ref root);
    
public:
    ruleset(const symbol_collection& symbols);
    ~ruleset() = default;

    symbol_ref set_root(std::string_view name);

    size_t add_rule(std::string_view left, const std::vector<std::string_view>& rights, std::optional<size_t> precedence = std::nullopt);

    size_t get_nterm_rside_count(size_t nterm_idx) const;

    // Returns the maximum number of rsides across all nterms
    size_t get_max_rside_count() const;

    size_t get_symbol_count(size_t nterm_idx, size_t rside_idx) const;

    symbol_ref get_symbol(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    symbol_type get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
    
    std::optional<size_t> get_term_prec(size_t term_idx) const;
    
    size_t get_symbol_index(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    std::optional<size_t> get_rside_precedence(size_t nterm_idx, size_t rside_idx) const;

    size_t calculate_rside_precedence(size_t nterm_idx, size_t rside_idx) const;
    
    // Returns the maximum number of symbols in any rside across all rsides
    size_t get_max_symbol_count() const;
    
    symbol_ref get_root() const;

    // The ones below delegate to symbol collection
    size_t get_nterm_count() const;
    
    size_t get_term_count() const;
    
    size_t get_symbol_count() const;
    
    std::string_view get_nterm_name(size_t nterm_idx) const;
    std::string_view get_term_name(size_t term_idx) const;
    
    void validate_term_idx(size_t term_idx) const;
    void validate_nterm_idx(size_t nterm_idx) const;
    void validate_rside_idx(size_t nterm_idx, size_t rside_idx) const;
    void validate_symbol_idx(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
    
    std::array<size_t, 3> get_rside_part_space_dims() const;
    std::array<size_t, 4> get_lr1_set_item_space_dims() const;
    
    std::string to_string() const;
    std::string lr1_set_item_to_string(const lr1_set_item& item) const;
};

} // namespace ptg
