#pragma once

#include <associativity.h>
#include <nterm.h>
#include <symbol_ref.h>
#include <term.h>
#include <symbol_list.h>
#include <ostream>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace ptg
{

struct string_hash
{
    using is_transparent = void;

    size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};

class symbol_collection
{
private:
    std::vector<term> terms_;
    std::vector<nterm> nterms_;
    std::unordered_map<std::string, symbol_ref, string_hash, std::equal_to<>> name_to_ref_;

    void validate_nterm_idx(size_t index) const;
    void validate_term_idx(size_t index) const;
    
public:
    symbol_collection();
    ~symbol_collection() = default;

    size_t add_term(std::string name, associativity assoc = associativity::left(), size_t prec = 0);
    size_t add_nterm(std::string name);

    bool contains(std::string_view name) const;

    symbol_ref get_symbol_ref(std::string_view name) const;

    std::string_view get_symbol_name(symbol_ref ref) const;

    std::string_view get_term_name(size_t index) const;
    std::string_view get_nterm_name(size_t index) const;

    associativity get_term_assoc(size_t index) const;
    size_t get_term_prec(size_t index) const;

    size_t get_term_count() const;
    size_t get_nterm_count() const;

    void print_symbol_list(std::ostream& os, const symbol_list& sl) const;
    void print_symbol_list_from_to(std::ostream& os, const symbol_list& sl, size_t start, size_t end) const;
};

} // namespace ptg
