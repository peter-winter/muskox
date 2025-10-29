#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <lr1_state.h>
#include <ruleset.h>
#include <symbol_collection.h>

#include <variant>
#include <optional>
#include <array>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("lr1_state", "[lr1_state]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    ruleset rs(sc);

    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "Expr"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});
    [[maybe_unused]] size_t expr_r2 = rs.add_rule("Expr", {});

    auto dims = rs.get_lr1_set_item_space_dims();

    SECTION("basic construction")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        const auto& all_items = state.get_all_items();
        REQUIRE(all_items.get_count() == 1);
        REQUIRE(all_items.contains(root_idx, 0, 0, eof_idx));
    }

    SECTION("construction with multiple kernel items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);
        kernel.add(s_idx, s_r0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        const auto& all_items = state.get_all_items();
        REQUIRE(all_items.get_count() == 2);
        REQUIRE(all_items.contains(root_idx, 0, 0, eof_idx));
        REQUIRE(all_items.contains(s_idx, s_r0, 0, eof_idx));
    }

    SECTION("add items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        index_subset<4> additional(dims);
        additional.add(s_idx, s_r0, 0, eof_idx);
        additional.add(expr_idx, expr_r0, 0, a_idx);

        state.add_items(additional);

        const auto& all_items = state.get_all_items();
        REQUIRE(all_items.get_count() == 3);
        REQUIRE(all_items.contains(root_idx, 0, 0, eof_idx));
        REQUIRE(all_items.contains(s_idx, s_r0, 0, eof_idx));
        REQUIRE(all_items.contains(expr_idx, expr_r0, 0, a_idx));
    }

    SECTION("add duplicate items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        index_subset<4> additional(dims);
        additional.add(root_idx, 0, 0, eof_idx);
        additional.add(s_idx, s_r0, 0, eof_idx);

        state.add_items(additional);

        const auto& all_items = state.get_all_items();
        REQUIRE(all_items.get_count() == 2);
        REQUIRE(all_items.contains(root_idx, 0, 0, eof_idx));
        REQUIRE(all_items.contains(s_idx, s_r0, 0, eof_idx));
    }

    SECTION("contains all items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);
        kernel.add(s_idx, s_r0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        index_subset<4> subset(dims);
        subset.add(root_idx, 0, 0, eof_idx);
        REQUIRE(state.contains_all_items(subset));

        index_subset<4> full(dims);
        full.add(root_idx, 0, 0, eof_idx);
        full.add(s_idx, s_r0, 0, eof_idx);
        REQUIRE(state.contains_all_items(full));

        index_subset<4> extra(dims);
        extra.add(root_idx, 0, 0, eof_idx);
        extra.add(s_idx, s_r0, 0, eof_idx);
        extra.add(expr_idx, expr_r0, 0, a_idx);
        REQUIRE_FALSE(state.contains_all_items(extra));
    }

    SECTION("get all items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        index_subset<4> additional(dims);
        additional.add(s_idx, s_r0, 0, eof_idx);

        state.add_items(additional);

        const auto& all_items = state.get_all_items();
        REQUIRE(all_items.get_count() == 2);

        const auto& indices = all_items.get_indices();
        REQUIRE(indices.size() == 2);
        REQUIRE(indices[0] == std::array<size_t, 4>{root_idx, 0, 0, eof_idx});
        REQUIRE(indices[1] == std::array<size_t, 4>{s_idx, s_r0, 0, eof_idx});
    }

    SECTION("get actions - shift on terminal")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 0, b_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::terminal, a_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::shift>(it->second));

        auto& s = std::get<lr1_state::shift>(it->second);
        REQUIRE(s.items_.size() == 1);
        REQUIRE(s.items_[0] == std::array<size_t, 4>{expr_idx, expr_r0, 1, b_idx});
    }

    SECTION("get actions - multiple shifts on same symbol")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 0, b_idx);
        kernel.add(expr_idx, expr_r0, 0, c_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::terminal, a_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::shift>(it->second));

        auto& s = std::get<lr1_state::shift>(it->second);
        REQUIRE(s.items_.size() == 2);
        REQUIRE(s.items_[0] == std::array<size_t, 4>{expr_idx, expr_r0, 1, b_idx});
        REQUIRE(s.items_[1] == std::array<size_t, 4>{expr_idx, expr_r0, 1, c_idx});
    }

    SECTION("get actions - shift on non_terminal")
    {
        index_subset<4> kernel(dims);
        kernel.add(s_idx, s_r0, 0, b_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::non_terminal, expr_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::shift>(it->second));

        auto& s = std::get<lr1_state::shift>(it->second);
        REQUIRE(s.items_.size() == 1);
        REQUIRE(s.items_[0] == std::array<size_t, 4>{s_idx, s_r0, 1, b_idx});
    }

    SECTION("get actions - reduction")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r1, 1, b_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::terminal, b_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::reduction>(it->second));

        auto& r = std::get<lr1_state::reduction>(it->second);
        REQUIRE(r.nterm_idx_ == expr_idx);
        REQUIRE(r.rside_idx_ == expr_r1);
    }

    SECTION("get actions - reduce reduce conflict")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r1, 1, a_idx);
        kernel.add(expr_idx, expr_r2, 0, a_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::terminal, a_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::conflict>(it->second));

        auto& c = std::get<lr1_state::conflict>(it->second);
        REQUIRE(!c.s_.has_value());
        REQUIRE(c.r_.size() == 2);
        REQUIRE(c.r_[0].nterm_idx_ == expr_idx);
        REQUIRE(c.r_[0].rside_idx_ == expr_r1);
        REQUIRE(c.r_[1].nterm_idx_ == expr_idx);
        REQUIRE(c.r_[1].rside_idx_ == expr_r2);
    }

    SECTION("get actions - shift reduce conflict")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 0, b_idx);
        kernel.add(expr_idx, expr_r1, 1, a_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::terminal, a_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::conflict>(it->second));

        auto& c = std::get<lr1_state::conflict>(it->second);
        REQUIRE(c.s_.has_value());
        REQUIRE(c.s_->items_.size() == 1);
        REQUIRE(c.s_->items_[0] == std::array<size_t, 4>{expr_idx, expr_r0, 1, b_idx});
        REQUIRE(c.r_.size() == 1);
        REQUIRE(c.r_[0].nterm_idx_ == expr_idx);
        REQUIRE(c.r_[0].rside_idx_ == expr_r1);
    }

    SECTION("get actions - add to existing conflict")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r1, 1, a_idx);
        kernel.add(expr_idx, expr_r0, 0, b_idx);
        kernel.add(expr_idx, expr_r2, 0, a_idx);
        kernel.add(expr_idx, expr_r0, 0, c_idx);

        lr1_state state(rs, std::move(kernel));

        auto actions = state.get_actions();

        REQUIRE(actions.size() == 1);

        symbol_ref key{symbol_type::terminal, a_idx};

        auto it = actions.find(key);
        REQUIRE(it != actions.end());

        REQUIRE(std::holds_alternative<lr1_state::conflict>(it->second));

        auto& c = std::get<lr1_state::conflict>(it->second);
        REQUIRE(c.s_.has_value());
        REQUIRE(c.s_->items_.size() == 2);
        REQUIRE(c.s_->items_[0] == std::array<size_t, 4>{expr_idx, expr_r0, 1, b_idx});
        REQUIRE(c.s_->items_[1] == std::array<size_t, 4>{expr_idx, expr_r0, 1, c_idx});
        REQUIRE(c.r_.size() == 2);
        REQUIRE(c.r_[0].nterm_idx_ == expr_idx);
        REQUIRE(c.r_[0].rside_idx_ == expr_r1);
        REQUIRE(c.r_[1].nterm_idx_ == expr_idx);
        REQUIRE(c.r_[1].rside_idx_ == expr_r2);
    }
    
    SECTION("to string - single item")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "$root -> . S / $eof");
    }

    SECTION("to string - multiple items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        lr1_state state(rs, std::move(kernel));

        index_subset<4> additional(dims);
        additional.add(s_idx, s_r0, 0, eof_idx);

        state.add_items(additional);

        REQUIRE(state.to_string() ==
            "$root -> . S / $eof\n"
            "S -> . Expr / $eof"
        );
    }

    SECTION("to string - shift item")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 0, b_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "Expr -> . a Expr / b");
    }

    SECTION("to string - reduction item")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r1, 1, b_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "Expr -> b . / b");
    }

    SECTION("to string - empty production item")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r2, 0, a_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "Expr -> . / a");
    }

    SECTION("to string - mixed shift and reduce")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 0, eof_idx);
        kernel.add(expr_idx, expr_r1, 1, b_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "Expr -> . a Expr / $eof\n"
            "Expr -> b . / b"
        );
    }

    SECTION("to string - mixed with empty production")
    {
        index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 0, eof_idx);
        kernel.add(expr_idx, expr_r2, 0, b_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "Expr -> . a Expr / $eof\n"
            "Expr -> . / b"
        );
    }

    SECTION("to string - shift on nonterm, reduce, and empty")
    {
        index_subset<4> kernel(dims);
        kernel.add(s_idx, s_r0, 0, eof_idx);
        kernel.add(expr_idx, expr_r1, 1, a_idx);
        kernel.add(expr_idx, expr_r2, 0, c_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "S -> . Expr / $eof\n"
            "Expr -> b . / a\n"
            "Expr -> . / c"
        );
    }

    SECTION("to string - four mixed items")
    {
        index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);
        kernel.add(s_idx, s_r0, 0, eof_idx);
        kernel.add(expr_idx, expr_r0, 1, b_idx);
        kernel.add(expr_idx, expr_r2, 0, c_idx);

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "$root -> . S / $eof\n"
            "S -> . Expr / $eof\n"
            "Expr -> a . Expr / b\n"
            "Expr -> . / c"
        );
    }
}
