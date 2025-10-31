#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <firsts.h>
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

    firsts f(rs);

    SECTION("get_nterm_firsts invalid")
    {
        REQUIRE_THROWS_MATCHES(
            f.get_nterm_firsts(rs.get_nterm_count()),
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("get_rside_part_firsts invalid nterm")
    {
        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(rs.get_nterm_count(), 0, 0),
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("get_rside_part_firsts invalid rside")
    {
        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(s_idx, rs.get_nterm_rside_count(s_idx), 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
    }

    SECTION("get_rside_part_firsts invalid symbol")
    {
        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(s_idx, s_r0, rs.get_symbol_count(s_idx, s_r0)),
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

    firsts f(rs);

    SECTION("calculate_nterm")
    {
        const auto& s_first = f.calculate_nterm(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(a_term_idx));
        REQUIRE(s_first.contains(b_term_idx));
    }

    SECTION("calculate_rside_part terminal")
    {
        const auto& part0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE(part0.get_count() == 1);
        REQUIRE(part0.contains(a_term_idx));

        const auto& part1 = f.calculate_rside_part(s_idx, s_r1, 0);
        REQUIRE(part1.get_count() == 1);
        REQUIRE(part1.contains(b_term_idx));
    }
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

    firsts f(rs);

    SECTION("calculate_nterm for nullable A")
    {
        const auto& a_first = f.calculate_nterm(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(b_term_idx));
    }

    SECTION("calculate_nterm for S propagating nullable")
    {
        const auto& s_first = f.calculate_nterm(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(b_term_idx));
        REQUIRE(s_first.contains(c_term_idx));
    }

    SECTION("calculate_rside_part for S")
    {
        const auto& part0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE(part0.get_count() == 2);
        REQUIRE(part0.contains(b_term_idx));
        REQUIRE(part0.contains(c_term_idx));  // Because A nullable

        const auto& part1 = f.calculate_rside_part(s_idx, s_r0, 1);
        REQUIRE(part1.get_count() == 1);
        REQUIRE(part1.contains(c_term_idx));
    }

    SECTION("calculate_rside_part for epsilon")
    {
        const auto& epsilon_part = f.calculate_rside_part(a_idx, a_r1, 0);
        REQUIRE(epsilon_part.get_count() == 0);
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

    firsts f(rs);

    SECTION("chain propagation")
    {
        const auto& s_first = f.calculate_nterm(s_idx);
        REQUIRE(s_first.get_count() == 1);
        REQUIRE(s_first.contains(d_term_idx));

        const auto& a_first = f.calculate_nterm(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(d_term_idx));

        const auto& b_first = f.calculate_nterm(b_idx);
        REQUIRE(b_first.get_count() == 1);
        REQUIRE(b_first.contains(d_term_idx));

        const auto& c_first = f.calculate_nterm(c_idx);
        REQUIRE(c_first.get_count() == 1);
        REQUIRE(c_first.contains(d_term_idx));
    }
}

TEST_CASE("firsts calculate_all and getters", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});

    sc.validate();
    
    firsts f(rs);

    f.calculate_all();

    SECTION("get_nterm_firsts")
    {
        const auto& s_first = f.get_nterm_firsts(s_idx);
        REQUIRE(s_first.has_value());
        REQUIRE(s_first.value().get_count() == 2);
        REQUIRE(s_first.value().contains(a_term_idx));
        REQUIRE(s_first.value().contains(b_term_idx));

        const auto& expr_first = f.get_nterm_firsts(expr_idx);
        REQUIRE(expr_first.has_value());
        REQUIRE(expr_first.value().get_count() == 2);
        REQUIRE(expr_first.value().contains(a_term_idx));
        REQUIRE(expr_first.value().contains(b_term_idx));
    }

    SECTION("get_rside_part_firsts")
    {
        const auto& s_part0 = f.get_rside_part_firsts(s_idx, s_r0, 0);
        REQUIRE(s_part0.has_value());
        REQUIRE(s_part0.value().get_count() == 2);
        REQUIRE(s_part0.value().contains(a_term_idx));
        REQUIRE(s_part0.value().contains(b_term_idx));

        const auto& expr_part0_r0 = f.get_rside_part_firsts(expr_idx, expr_r0, 0);
        REQUIRE(expr_part0_r0.has_value());
        REQUIRE(expr_part0_r0.value().get_count() == 1);
        REQUIRE(expr_part0_r0.value().contains(a_term_idx));

        const auto& expr_part0_r1 = f.get_rside_part_firsts(expr_idx, expr_r1, 0);
        REQUIRE(expr_part0_r1.has_value());
        REQUIRE(expr_part0_r1.value().get_count() == 1);
        REQUIRE(expr_part0_r1.value().contains(b_term_idx));
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

        firsts f(rs);

        const auto& a_first = f.calculate_nterm(a_idx);
        
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

        firsts f(rs);

        REQUIRE_THROWS_MATCHES(
            f.calculate_nterm(a_idx),
            grammar_error,
            Message("Nonterminal 'A' has unsolvable left recursion")
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

        firsts f(rs);

        REQUIRE_THROWS_MATCHES(
            f.calculate_nterm(a_idx),
            grammar_error,
            Message("Nonterminal 'A' has unsolvable left recursion")
        );

        REQUIRE_THROWS_MATCHES(
            f.calculate_nterm(b_idx),
            grammar_error,
            Message("Nonterminal 'B' has unsolvable left recursion")
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

        firsts f(rs);

        const auto& a_first = f.calculate_nterm(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(c_term_idx));

        const auto& b_first = f.calculate_nterm(b_idx);
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

        firsts f(rs);

        const auto& a_first = f.calculate_nterm(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(c_term_idx));

        const auto& b_first = f.calculate_nterm(b_idx);
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

    firsts f(rs);

    SECTION("calculate_all in complex")
    {
        f.calculate_all();

        const auto& expr_first = f.get_nterm_firsts(expr_idx);
        REQUIRE(expr_first.has_value());
        REQUIRE(expr_first.value().get_count() == 2);
        REQUIRE(expr_first.value().contains(num_idx));
        REQUIRE(expr_first.value().contains(lpar_idx));

        const auto& term_first = f.get_nterm_firsts(term_idx);
        REQUIRE(term_first.has_value());
        REQUIRE(term_first.value().get_count() == 2);
        REQUIRE(term_first.value().contains(num_idx));
        REQUIRE(term_first.value().contains(lpar_idx));

        const auto& factor_first = f.get_nterm_firsts(factor_idx);
        REQUIRE(factor_first.has_value());
        REQUIRE(factor_first.value().get_count() == 2);
        REQUIRE(factor_first.value().contains(num_idx));
        REQUIRE(factor_first.value().contains(lpar_idx));
    }

    SECTION("rside_part in complex")
    {
        const auto& expr_part1 = f.calculate_rside_part(expr_idx, expr_r1, 1);  // Expr -> Expr + . Term
        REQUIRE(expr_part1.get_count() == 1);
        REQUIRE(expr_part1.contains(plus_idx));

        const auto& factor_part1 = f.calculate_rside_part(factor_idx, factor_r1, 1);  // Factor -> ( . Expr )
        REQUIRE(factor_part1.get_count() == 2);
        REQUIRE(factor_part1.contains(num_idx));
        REQUIRE(factor_part1.contains(lpar_idx));
    }
}

TEST_CASE("firsts mix individual and calculate_all", "[firsts]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A"});
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"B", "c"});

    firsts f(rs);

    SECTION("mix")
    {
        const auto& a_first = f.calculate_nterm(a_idx);
        REQUIRE(a_first.get_count() == 0);

        const auto& s_part0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE(s_part0.get_count() == 1);
        REQUIRE(s_part0.contains(c_term_idx));  // B nullable

        f.calculate_all();

        const auto& s_first = f.get_nterm_firsts(s_idx);
        REQUIRE(s_first.has_value());
        REQUIRE(s_first.value().get_count() == 1);
        REQUIRE(s_first.value().contains(c_term_idx));

        const auto& b_first = f.get_nterm_firsts(b_idx);
        REQUIRE(b_first.has_value());
        REQUIRE(b_first.value().get_count() == 0);
        REQUIRE(b_first.value().get_count() == 0);
    }
}
