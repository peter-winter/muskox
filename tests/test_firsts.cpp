#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;


TEST_CASE("firsts invalid index handling", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

    rs.validate();

    SECTION("get_nterm_first invalid")
    {
        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(rs.get_nterm_count()),
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("get_suffix_firsts invalid nterm")
    {
        REQUIRE_THROWS_MATCHES(
            rs.get_suffix_first(rs.get_nterm_count(), 0, 0),
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("get_suffix_first invalid rside")
    {
        REQUIRE_THROWS_MATCHES(
            rs.get_suffix_first(s_idx, rs.get_nterm_rside_count(s_idx), 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
    }

    SECTION("get_suffix_first invalid symbol")
    {
        REQUIRE_THROWS_MATCHES(
            rs.get_suffix_first(s_idx, s_r0, rs.get_symbol_count(s_idx, s_r0)),
            std::out_of_range,
            Message("Symbol index out of range")
        );
    }
}

TEST_CASE("firsts basic calculations", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b");

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"b"});

    rs.validate();

    SECTION("get_nterm_first")
    {
        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(a_term_idx));
        REQUIRE(s_first.contains(b_term_idx));
    }

    SECTION("get_suffix_first terminal")
    {
        const auto& part0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(part0.get_count() == 1);
        REQUIRE(part0.contains(a_term_idx));

        const auto& part1 = rs.get_suffix_first(s_idx, s_r1, 0);
        REQUIRE(part1.get_count() == 1);
        REQUIRE(part1.contains(b_term_idx));
    }
}

TEST_CASE("firsts with pure epsilon non-termnial", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {});
    
    rs.validate();
    
    REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(s_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );
}

TEST_CASE("firsts with epsilon productions", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_term_idx = sc.add_term("c");

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "c"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"b"});
    [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

    rs.validate();

    SECTION("get_nterm_first for nullable A")
    {
        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(b_term_idx));
    }

    SECTION("get_nterm_first for S propagating nullable")
    {
        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(b_term_idx));
        REQUIRE(s_first.contains(c_term_idx));
    }

    SECTION("get_suffix_first for S")
    {
        const auto& part0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(part0.get_count() == 2);
        REQUIRE(part0.contains(b_term_idx));
        REQUIRE(part0.contains(c_term_idx));  // Because A nullable

        const auto& part1 = rs.get_suffix_first(s_idx, s_r0, 1);
        REQUIRE(part1.get_count() == 1);
        REQUIRE(part1.contains(c_term_idx));
    }

    SECTION("get_suffix_first for epsilon")
    {
        REQUIRE_THROWS_MATCHES(
            rs.get_suffix_first(a_idx, a_r1, 0),
            std::out_of_range,
            Message("Symbol index out of range")
        );
    }
}

TEST_CASE("firsts nterm chain", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_idx = sc.add_nterm("C");
    [[maybe_unused]] size_t d_term_idx = sc.add_term("d");

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"B"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"C"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"d"});

    rs.validate();

    SECTION("chain propagation")
    {
        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 1);
        REQUIRE(s_first.contains(d_term_idx));

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(d_term_idx));

        const auto& b_first = rs.get_nterm_first(b_idx);
        REQUIRE(b_first.get_count() == 1);
        REQUIRE(b_first.contains(d_term_idx));

        const auto& c_first = rs.get_nterm_first(c_idx);
        REQUIRE(c_first.get_count() == 1);
        REQUIRE(c_first.contains(d_term_idx));
    }
}

TEST_CASE("firsts left recursion handling", "[firsts]")
{
    SECTION("direct left recursion with epsilon")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

        rs.validate();

        const auto& a_first = rs.get_nterm_first(a_idx);
        
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(b_term_idx));
    }
    
    SECTION("direct left recursion no base throws")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(a_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );
    }

    SECTION("indirect left recursion unsolved")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A", "c"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(a_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(b_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );
    }

    SECTION("indirect left recursion solved")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {"c"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A", "c"});

        rs.validate();

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(c_term_idx));

        const auto& b_first = rs.get_nterm_first(b_idx);
        REQUIRE(b_first.get_count() == 1);
        REQUIRE(b_first.contains(c_term_idx));
    }

    SECTION("mutual recursion not left-recursive")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        [[maybe_unused]] size_t d_term_idx = sc.add_term("d");
        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"c", "B"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"d", "A"});

        rs.validate();

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(c_term_idx));

        const auto& b_first = rs.get_nterm_first(b_idx);
        REQUIRE(b_first.get_count() == 1);
        REQUIRE(b_first.contains(d_term_idx));
    }
}

TEST_CASE("firsts complex grammar", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t term_idx = sc.add_nterm("Term");
    [[maybe_unused]] size_t factor_idx = sc.add_nterm("Factor");
    [[maybe_unused]] size_t plus_idx = sc.add_term("+");
    [[maybe_unused]] size_t mul_idx = sc.add_term("*");
    [[maybe_unused]] size_t num_idx = sc.add_term("num");
    [[maybe_unused]] size_t lpar_idx = sc.add_term("(");
    [[maybe_unused]] size_t rpar_idx = sc.add_term(")");
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"Term"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"Expr", "+", "Term"});
    [[maybe_unused]] size_t term_r0 = rs.add_rule("Term", {"Factor"});
    [[maybe_unused]] size_t term_r1 = rs.add_rule("Term", {"Term", "*", "Factor"});
    [[maybe_unused]] size_t factor_r0 = rs.add_rule("Factor", {"num"});
    [[maybe_unused]] size_t factor_r1 = rs.add_rule("Factor", {"(", "Expr", ")"});

    rs.validate();

    SECTION("nterms in complex")
    {
        const auto& expr_first = rs.get_nterm_first(expr_idx);
        REQUIRE(expr_first.get_count() == 2);
        REQUIRE(expr_first.contains(num_idx));
        REQUIRE(expr_first.contains(lpar_idx));

        const auto& term_first = rs.get_nterm_first(term_idx);
        REQUIRE(term_first.get_count() == 2);
        REQUIRE(term_first.contains(num_idx));
        REQUIRE(term_first.contains(lpar_idx));

        const auto& factor_first = rs.get_nterm_first(factor_idx);
        REQUIRE(factor_first.get_count() == 2);
        REQUIRE(factor_first.contains(num_idx));
        REQUIRE(factor_first.contains(lpar_idx));
    }

    SECTION("suffix in complex")
    {
        const auto& expr_part1 = rs.get_suffix_first(expr_idx, expr_r1, 1);  // Expr -> Expr . + Term
        REQUIRE(expr_part1.get_count() == 1);
        REQUIRE(expr_part1.contains(plus_idx));

        const auto& factor_part1 = rs.get_suffix_first(factor_idx, factor_r1, 1);  // Factor -> ( . Expr )
        REQUIRE(factor_part1.get_count() == 2);
        REQUIRE(factor_part1.contains(num_idx));
        REQUIRE(factor_part1.contains(lpar_idx));
    }
}

