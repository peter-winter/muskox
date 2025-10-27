#include "parser.h"

#include "parse_context.h"
#include "parse_error.h"

#include <optional>

namespace ptg
{

parser::parser(const parse_table& pt, const ruleset& rs)
    : pt_(pt), rs_(rs)
{
}

size_t parser::get_lookahead(symbol_stream& stream) const
{
    auto token = stream.next();
    return token ? *token : 0; // eof_idx = 0
}

parse_result parser::parse(symbol_stream& stream) const
{
    parse_context ctx(rs_);

    ctx.push_state_on_stack(0, 0);

    size_t term_idx = get_lookahead(stream);
    symbol_ref lookahead{symbol_type::terminal, term_idx};

    while (!ctx.stack_empty(0))
    {
        size_t current_state = ctx.get_back_state(0);

        const parse_table_entry& entry = pt_.get(current_state, lookahead);

        if (entry.is_shift())
        {
            do_shift(ctx, entry.get_shift_state_idx(), lookahead);
            term_idx = get_lookahead(stream);
            lookahead = {symbol_type::terminal, term_idx};
        }
        else if (entry.is_reduce())
        {
            size_t nterm_idx = entry.get_reduce_nterm_idx();
            size_t rside_idx = entry.get_reduce_rside_idx();

            if (do_reduce(ctx, nterm_idx, rside_idx, term_idx))
            {
                return parse_result::ok();
            }
        }
        else
        {
            do_error(ctx, current_state, term_idx);
            return parse_result::error(ctx.take_errors());
        }
    }

    return parse_result::error(ctx.take_errors());
}

void parser::do_shift(parse_context& ctx, size_t new_state, symbol_ref symbol) const
{
    size_t from_state = ctx.get_back_state(0);
    ctx.push_state_on_stack(0, new_state);
    ctx.on_shift(0, from_state, new_state, symbol);
}

bool parser::do_reduce(parse_context& ctx, size_t nterm_idx, size_t rside_idx, size_t term_idx) const
{
    size_t pop_count = rs_.get_symbol_count(nterm_idx, rside_idx);

    if (!ctx.pop_n_from_stack(0, pop_count))
    {
        do_error(ctx, ctx.get_back_state(0), term_idx);
        return false;
    }

    if (nterm_idx == 0) // $root index = 0
    {
        if (term_idx != 0)
        {
            do_error(ctx, ctx.stack_empty(0) ? 0 : ctx.get_back_state(0), term_idx);
            return false;
        }
        do_success(ctx);
        return true;
    }

    if (ctx.stack_empty(0))
    {
        do_error(ctx, 0, term_idx);
        return false;
    }

    symbol_ref nterm_ref{symbol_type::non_terminal, nterm_idx};
    size_t current_state = ctx.get_back_state(0);
    const parse_table_entry& goto_entry = pt_.get(current_state, nterm_ref);

    if (!goto_entry.is_shift())
    {
        do_error(ctx, current_state, term_idx);
        return false;
    }

    ctx.push_state_on_stack(0, goto_entry.get_shift_state_idx());
    ctx.on_reduce(0, nterm_idx, rside_idx, pop_count);
    return false;
}

void parser::do_error(parse_context& ctx, size_t state, size_t term_idx) const
{
    symbol_ref lookahead{symbol_type::terminal, term_idx};
    ctx.on_error(0, state, lookahead);
    syntax_error(ctx, term_idx);
}

void parser::do_success(parse_context& ctx) const
{
    ctx.on_accept(0);
}

void parser::syntax_error(parse_context& ctx, size_t term_idx) const
{
    std::string_view symbol = rs_.get_term_name(term_idx);
    parse_error err(parse_error_templates::code::syntax_error, symbol);
    ctx.add_error(err.str());
}

} // namespace ptg
