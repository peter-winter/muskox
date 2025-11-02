#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("ruleset symbol collection not validated", "[ruleset]")
{
    SECTION("not validated")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");

        REQUIRE_THROWS_MATCHES(
            ruleset(sc),
            std::runtime_error,
            Message("Symbol collection not validated")
        );
    }
    
    SECTION("has issues")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_term("a");
        
        sc.validate();

        REQUIRE_THROWS_MATCHES(
            ruleset(sc),
            std::runtime_error,
            Message("Symbol collection has issues")
        );
    }
}

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
        [[maybe_unused]] size_t r1_idx = rs.add_rule("S", {"a", "B"});
        [[maybe_unused]] size_t r2_idx = rs.add_rule("B", {"a"});
        rs.validate();
        
        REQUIRE(r1_idx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, r1_idx) == 2);
        REQUIRE(rs.get_symbol_type(s_idx, r1_idx, 0) == symbol_type::terminal);
        REQUIRE(rs.get_symbol_index(s_idx, r1_idx, 0) == a_idx);
        REQUIRE(rs.get_symbol_type(s_idx, r1_idx, 1) == symbol_type::non_terminal);
        REQUIRE(rs.get_symbol_index(s_idx, r1_idx, 1) == b_idx);
        REQUIRE(!rs.get_explicit_rside_precedence(s_idx, r1_idx).has_value());
    }

    SECTION("empty right side")
    {
        [[maybe_unused]] size_t r1_idx = rs.add_rule("S", {});
        [[maybe_unused]] size_t r2_idx = rs.add_rule("B", {"a"});
        rs.validate();
        
        REQUIRE(r1_idx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, r1_idx) == 0);
        REQUIRE(!rs.get_explicit_rside_precedence(s_idx, r1_idx).has_value());
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
        [[maybe_unused]] size_t r1_idx = rs.add_rule("S", {"a", "B"}, 5);
        [[maybe_unused]] size_t r2_idx = rs.add_rule("B", {"a"});
        rs.validate();
        
        REQUIRE_FALSE(rs.get_explicit_rside_precedence(b_idx, r2_idx).has_value());
        REQUIRE(rs.get_explicit_rside_precedence(s_idx, r1_idx).has_value());
        REQUIRE(rs.get_explicit_rside_precedence(s_idx, r1_idx).value() == 5);
    }

    SECTION("multiple rules")
    {
        [[maybe_unused]] size_t r1_idx = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t r2_idx = rs.add_rule("S", {"B"});
        [[maybe_unused]] size_t r3_idx = rs.add_rule("B", {});
        rs.validate();
        
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 2);
        REQUIRE(rs.get_symbol_count(s_idx, r1_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, r2_idx) == 1);
        REQUIRE(rs.get_symbol_count(b_idx, r3_idx) == 0);
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
    
    rs.validate();

    SECTION("get_suffix_space_dims")
    {
        auto dims = rs.get_suffix_space_dims();
        REQUIRE(dims[0] == 3);  // nterm count, including $root
        REQUIRE(dims[1] == 2);  // max rside count
        REQUIRE(dims[2] == 3);  // max symbol count
    }

    SECTION("get_lr1_set_item_space_dims")
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

    rs.validate();
    
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

TEST_CASE("ruleset get_effective_rside_precedence", "[ruleset]")
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
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {"Expr"});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"a", "b"}, 100);
        
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(expr_idx, ridx) == 100);
    }

    SECTION("no explicit, last term prec")
    {
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(s_idx, ridx) == 10);
    }

    SECTION("no explicit, multiple terms, last with prec")
    {
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"Expr", "+", "Expr"});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(expr_idx, ridx) == 20);
    }

    SECTION("no explicit, last no prec, previous has")
    {
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {"Expr"});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"Expr", "*", "id"});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(expr_idx, ridx) == 30);
    }

    SECTION("no terms with prec")
    {
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {"id"});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(s_idx, ridx) == 0);
    }

    SECTION("empty rside")
    {
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(expr_idx, ridx) == 0);
    }

    SECTION("only nonterms")
    {
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {"Expr"});
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(s_idx, sidx) == 0);
    }

    SECTION("mixed, last terminal no prec, but earlier has")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"a", "Expr", "b"});
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(expr_idx, ridx) == 10);  // last b has none, previous a has 10
    }

    SECTION("no explicit, first has prec, last no")
    {
        [[maybe_unused]] size_t ridx = rs.add_rule("Expr", {"a", "id"});
        [[maybe_unused]] size_t sidx = rs.add_rule("S", {});
        rs.validate();
        REQUIRE(rs.get_effective_rside_precedence(expr_idx, ridx) == 10);
    }
}

TEST_CASE("ruleset to symbol collection delegation", "[ruleset]")
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

    rs.validate();
    
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

    rs.validate();
    
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
        
        rs.validate();

        REQUIRE(rs.get_errors().size() == 1);
        REQUIRE(rs.get_errors()[0] == "Nonterminal 'B' has no productions");
        REQUIRE(rs.get_warnings().empty());
        REQUIRE(rs.is_validated() == true);
    }
}

TEST_CASE("ruleset before/after validation", "[ruleset]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    sc.validate();

    ruleset rs(sc);

    SECTION("before validation")
    {
        [[maybe_unused]] size_t r_idx = rs.add_rule("S", {"a"});
        
        REQUIRE_NOTHROW(rs.add_rule("S", {"a"}));
        REQUIRE_NOTHROW(rs.set_root("S"));
                
        REQUIRE_THROWS_MATCHES(
            rs.get_effective_rside_precedence(s_idx, r_idx),
            std::runtime_error,
            Message("Cannot query effective rside precedence before validation")
        );
        
        REQUIRE_THROWS_MATCHES(
            rs.get_suffix_space_dims(),
            std::runtime_error,
            Message("Cannot query suffix space dims before validation")
        );
        
        REQUIRE_THROWS_MATCHES(
            rs.get_lr1_set_item_space_dims(),
            std::runtime_error,
            Message("Cannot query lr1 set item space dims before validation")
        );
        
        REQUIRE_THROWS_MATCHES(
            rs.is_suffix_nullable(s_idx, r_idx, 0),
            std::runtime_error,
            Message("Cannot query suffix nullability before validation")
        );
        
        REQUIRE_THROWS_MATCHES(
            rs.is_nterm_nullable(s_idx),
            std::runtime_error,
            Message("Cannot query nterm nullability before validation")
        );
        
        REQUIRE_THROWS_MATCHES(
            rs.get_suffix_first(s_idx, r_idx, 0),
            std::runtime_error,
            Message("Cannot query suffix FIRST sets before validation")
        );
        
        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(s_idx),
            std::runtime_error,
            Message("Cannot query nterm FIRST sets before validation")
        );
    
        rs.validate();
    
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("S", {"b"}), 
            std::runtime_error,
            Message("Cannot add rules after validation")
        );
    
        REQUIRE_THROWS_MATCHES(
            rs.set_root("S"),
            std::runtime_error,
            Message("Cannot set root after validation")
        );
        
        REQUIRE_NOTHROW(rs.get_effective_rside_precedence(s_idx, r_idx));
        REQUIRE_NOTHROW(rs.get_suffix_space_dims());
        REQUIRE_NOTHROW(rs.get_lr1_set_item_space_dims());
        REQUIRE_NOTHROW(rs.is_suffix_nullable(s_idx, r_idx, 0));
        REQUIRE_NOTHROW(rs.is_nterm_nullable(s_idx));
        REQUIRE_NOTHROW(rs.get_suffix_first(s_idx, r_idx, 0));
        REQUIRE_NOTHROW(rs.get_nterm_first(s_idx));
    }

    SECTION("is_validated")
    {
        REQUIRE(!rs.is_validated());
        rs.add_rule("S", {"b"});
        rs.validate();
        REQUIRE(rs.is_validated());
        REQUIRE_THROWS_MATCHES(
            rs.validate(),
            std::runtime_error,
            Message("Cannot validate twice")
        );
    }
}
