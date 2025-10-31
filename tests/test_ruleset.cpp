#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("ruleset add_rule", "[ruleset]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    
    sc.validate();
    
    ruleset rs(sc);    

    SECTION("basic add")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("S", {"a", "B"});
        REQUIRE(ridx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx) == 2);
        REQUIRE(rs.get_symbol_type(s_idx, ridx, 0) == symbol_type::terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx, 0) == a_idx);
        REQUIRE(rs.get_symbol_type(s_idx, ridx, 1) == symbol_type::non_terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx, 1) == b_idx);
        REQUIRE(!rs.get_rside_precedence(s_idx, ridx).has_value());
    }

    SECTION("empty right side")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("S", {});
        REQUIRE(ridx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx) == 0);
        REQUIRE(!rs.get_rside_precedence(s_idx, ridx).has_value());
    }

    SECTION("lside_not_exists")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("nonexist", {}),
            grammar_error,
            Message("Left side 'nonexist' does not exist")
        );
    }

    SECTION("lside_term")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("a", {"S"}),
            grammar_error,
            Message("Left side 'a' is a terminal")
        );
    }

    SECTION("rside_not_exists")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("S", {"nonexist"}),
            grammar_error,
            Message("Right side symbol 'nonexist' does not exist")
        );
    }

    SECTION("explicit precedence")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("S", {"a", "B"}, 5);
        REQUIRE(rs.get_rside_precedence(s_idx, ridx) == 5);
    }

    SECTION("multiple rules")
    {
        [[maybe_unused]] size_t ridx1 = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t ridx2 = rs.add_rule("S", {"B"});
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 2);
        REQUIRE(rs.get_symbol_count(s_idx, ridx1) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx2) == 1);
    }
}

TEST_CASE("ruleset dims", "[ruleset]")
{
    symbol_collection sc;
    
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    sc.validate();
    
    ruleset rs(sc);

    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr", "a"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"b"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "b", "c"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {});

    SECTION("rside part space dims")
    {
        auto dims = rs.get_suffix_space_dims();
        REQUIRE(dims[0] == 3);  // nterm count, including $root
        REQUIRE(dims[1] == 2);  // max rside count
        REQUIRE(dims[2] == 3);  // max symbol count
    }

    SECTION("lr1 set item space dims")
    {
        auto dims = rs.get_lr1_set_item_space_dims();
        REQUIRE(dims[0] == 3);  // nterm count
        REQUIRE(dims[1] == 2);  // max rside count
        REQUIRE(dims[2] == 4);  // max symbol count + 1 (for dot positions)
        REQUIRE(dims[3] == 4);  // term count, including $eof
    }
}

TEST_CASE("ruleset lr1_set_item_to_string", "[ruleset]")
{
    symbol_collection sc;
    
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    sc.validate();
    
    ruleset rs(sc);

    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "Expr"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});

    SECTION("dot at beginning")
    {
        lr1_set_item item(expr_idx, expr_r0, 0, c_idx);
        REQUIRE(rs.lr1_set_item_to_string(item) == "Expr -> . a Expr / c");
    }

    SECTION("dot in middle")
    {
        lr1_set_item item(expr_idx, expr_r0, 1, c_idx);
        REQUIRE(rs.lr1_set_item_to_string(item) == "Expr -> a . Expr / c");
    }

    SECTION("dot at end")
    {
        lr1_set_item item(expr_idx, expr_r0, 2, c_idx);
        REQUIRE(rs.lr1_set_item_to_string(item) == "Expr -> a Expr . / c");
    }

    SECTION("single symbol production")
    {
        lr1_set_item item(expr_idx, expr_r1, 0, c_idx);
        REQUIRE(rs.lr1_set_item_to_string(item) == "Expr -> . b / c");
    }

    SECTION("single symbol, dot at end")
    {
        lr1_set_item item(expr_idx, expr_r1, 1, c_idx);
        REQUIRE(rs.lr1_set_item_to_string(item) == "Expr -> b . / c");
    }
}

TEST_CASE("ruleset root", "[ruleset]")
{
    symbol_collection sc;
    
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t other_idx = sc.add_nterm("Other");
    
    sc.validate();
    
    SECTION("valid root")
    {
        ruleset rs(sc);
        REQUIRE(sc.get_symbol_name(rs.get_root()) == "S");
        REQUIRE(rs.set_root("Other") == symbol_ref{symbol_type::non_terminal, 2});
        REQUIRE(rs.get_root().index_ == 2);
        REQUIRE(sc.get_symbol_name(rs.get_root()) == "Other");
        
        rs.add_rule("S", {});
        rs.add_rule("Other", {"a"});
        rs.validate();
        REQUIRE(rs.get_symbol(0, 0, 0) == sc.get_symbol_ref("Other"));
    }

    SECTION("set invalid name")
    {
        ruleset rs(sc);
        REQUIRE_THROWS_MATCHES(
            rs.set_root("nonexist"),
            grammar_error,
            Message("Root symbol 'nonexist' does not exist")
        );
    }

    SECTION("set term")
    {
        ruleset rs(sc);
        REQUIRE_THROWS_MATCHES(
            rs.set_root("a"),
            grammar_error,
            Message("Root symbol 'a' is a terminal")
        );
    }
    
    SECTION("set $root")
    {
        ruleset rs(sc);
        REQUIRE_THROWS_MATCHES(
            rs.set_root("$root"),
            grammar_error,
            Message("Cannot refer special '$root' symbol")
        );
    }
}

TEST_CASE("ruleset calculate_rside_precedence", "[ruleset]")
{
    symbol_collection sc;
        
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t a_idx = sc.add_term("a", 10);
    [[maybe_unused]] size_t plus_idx = sc.add_term("+", 20);
    [[maybe_unused]] size_t mul_idx = sc.add_term("*", 30);
    [[maybe_unused]] size_t b_idx = sc.add_term("b");  // no prec
    [[maybe_unused]] size_t id_idx = sc.add_term("id");  // no prec
    sc.validate();
    
    ruleset rs(sc);

    SECTION("explicit precedence")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"a", "b"}, 100);
        REQUIRE(rs.calculate_rside_precedence(expr_idx, ridx) == 100);
    }

    SECTION("no explicit, last term prec")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("S", {"a"});
        REQUIRE(rs.calculate_rside_precedence(s_idx, ridx) == 10);
    }

    SECTION("no explicit, multiple terms, last with prec")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"Expr", "+", "Expr"});
        REQUIRE(rs.calculate_rside_precedence(expr_idx, ridx) == 20);
    }

    SECTION("no explicit, last no prec, previous has")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"Expr", "*", "id"});
        REQUIRE(rs.calculate_rside_precedence(expr_idx, ridx) == 30);
    }

    SECTION("no terms with prec")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("S", {"id"});
        REQUIRE(rs.calculate_rside_precedence(s_idx, ridx) == 0);
    }

    SECTION("empty rside")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {});
        REQUIRE(rs.calculate_rside_precedence(expr_idx, ridx) == 0);
    }

    SECTION("only nonterms")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("S", {"Expr"});
        REQUIRE(rs.calculate_rside_precedence(s_idx, ridx) == 0);
    }

    SECTION("mixed, last terminal no prec, but earlier has")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"a", "Expr", "b"});
        REQUIRE(rs.calculate_rside_precedence(expr_idx, ridx) == 10);  // last b has none, previous a has 10
    }

    SECTION("no explicit, first has prec, last no")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"a", "id"});
        REQUIRE(rs.calculate_rside_precedence(expr_idx, ridx) == 10);
    }
}

TEST_CASE("ruleset delegation", "[ruleset]")
{
    symbol_collection sc;
    
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b", 10);
    sc.validate();
    
    ruleset rs(sc);

    SECTION("get_nterm_count")
    {
        REQUIRE(rs.get_nterm_count() == 3);  // $root, S, Expr
    }

    SECTION("get_term_count")
    {
        REQUIRE(rs.get_term_count() == 3);  // $eof, a, b
    }

    SECTION("get_term_plus_nterm_count")
    {
        REQUIRE(rs.get_term_plus_nterm_count() == 6);  // terms + nterms
    }

    SECTION("get_nterm_name")
    {
        REQUIRE(rs.get_nterm_name(s_idx) == "S");
        REQUIRE(rs.get_nterm_name(expr_idx) == "Expr");
    }

    SECTION("get_term_name")
    {
        REQUIRE(rs.get_term_name(a_idx) == "a");
        REQUIRE(rs.get_term_name(b_idx) == "b");
    }
    
    SECTION("get_term_prec")
    {
        REQUIRE(rs.get_term_prec(a_idx) == 0);
        REQUIRE(rs.get_term_prec(b_idx) == 10);
    }
}

TEST_CASE("ruleset space dims", "[ruleset]")
{
    symbol_collection sc;
    
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    sc.validate();
    
    ruleset rs(sc);

    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr", "a"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"b"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "b", "c"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {});

    SECTION("get_suffix_space_dims")
    {
        auto dims = rs.get_suffix_space_dims();
        REQUIRE(dims[0] == rs.get_nterm_count());
        REQUIRE(dims[1] == rs.get_max_rside_count());
        REQUIRE(dims[2] == rs.get_max_symbol_count());
    }

    SECTION("get_lr1_set_item_space_dims")
    {
        auto dims = rs.get_lr1_set_item_space_dims();
        REQUIRE(dims[0] == rs.get_nterm_count());
        REQUIRE(dims[1] == rs.get_max_rside_count());
        REQUIRE(dims[2] == rs.get_max_symbol_count() + 1);  // +1 for dot positions
        REQUIRE(dims[3] == rs.get_term_count());
    }
}

TEST_CASE("ruleset user idx validation", "[ruleset]")
{
    symbol_collection sc;
    
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    
    sc.validate();
    
    ruleset rs(sc);

    [[maybe_unused]] size_t ridx = rs.add_rule("S", {"a"});

    SECTION("validate_term_idx valid")
    {
        REQUIRE_NOTHROW(rs.validate_term_idx(a_idx));
    }

    SECTION("validate_term_idx invalid")
    {
        REQUIRE_THROWS_AS(rs.validate_term_idx(100), std::out_of_range);
    }

    SECTION("validate_nterm_idx valid")
    {
        REQUIRE_NOTHROW(rs.validate_nterm_idx(s_idx));
    }

    SECTION("validate_nterm_idx invalid")
    {
        REQUIRE_THROWS_AS(rs.validate_nterm_idx(100), std::out_of_range);
    }

    SECTION("validate_rside_idx valid")
    {
        REQUIRE_NOTHROW(rs.validate_rside_idx(s_idx, ridx));
    }

    SECTION("validate_rside_idx invalid nterm")
    {
        REQUIRE_THROWS_AS(rs.validate_rside_idx(100, 0), std::out_of_range);
    }

    SECTION("validate_rside_idx invalid rside")
    {
        REQUIRE_THROWS_AS(rs.validate_rside_idx(s_idx, 100), std::out_of_range);
    }

    SECTION("validate_suffix_idx valid")
    {
        REQUIRE_NOTHROW(rs.validate_suffix_idx(s_idx, ridx, 0));
    }

    SECTION("validate_suffix_idx invalid nterm")
    {
        REQUIRE_THROWS_AS(rs.validate_suffix_idx(100, ridx, 0), std::out_of_range);
    }

    SECTION("validate_suffix_idx invalid rside")
    {
        REQUIRE_THROWS_AS(rs.validate_suffix_idx(s_idx, 100, 0), std::out_of_range);
    }

    SECTION("validate_suffix_idx invalid symbol")
    {
        REQUIRE_THROWS_AS(rs.validate_suffix_idx(s_idx, ridx, 100), std::out_of_range);
    }
}

TEST_CASE("ruleset validation", "[ruleset]")
{
    SECTION("nterm no rsides")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        
        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        REQUIRE_THROWS_MATCHES(
            rs.validate(),
            grammar_error,
            Message("Nonterminal 'B' has no productions")
        );
    }
}

