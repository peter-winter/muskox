#pragma once

#include "parse_table.h"
#include "parse_context.h"
#include "parse_result.h"
#include "ruleset.h"
#include "symbol_ref.h"
#include "symbol_stream.h"

namespace ptg
{

class parser
{
public:
    parser(const parse_table& pt, const ruleset& rs);

    parse_result parse(symbol_stream& stream) const;

private:
    size_t get_lookahead(symbol_stream& stream) const;

    void do_shift(parse_context& ctx, size_t new_state, symbol_ref symbol) const;

    bool do_reduce(parse_context& ctx, size_t nterm_idx, size_t rside_idx, size_t term_idx) const;

    void do_error(parse_context& ctx, size_t state, size_t term_idx) const;

    void do_success(parse_context& ctx) const;

    void syntax_error(parse_context& ctx, size_t term_idx) const;

    const parse_table& pt_;
    const ruleset& rs_;
};

} // namespace ptg
