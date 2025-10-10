#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <firsts.h>
#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("firsts class", "[firsts]")
{
    SECTION("simple grammar with terminals only")
    {
        ptg::symbol_collection sc;
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"a"});
        size_t s_r1 = rs.add_rule("S", {"b"});

        ptg::firsts f(rs);
        f.calculate_all();

        size_t root_idx = 0;

        // FIRST($root) = FIRST(S) = {a, b}
        const auto& root_firsts = f.get_nterm_firsts(root_idx);
        REQUIRE(root_firsts.has_value());
        REQUIRE(root_firsts.value().get_count() == 2);
        REQUIRE(root_firsts.value().contains(a_idx));
        REQUIRE(root_firsts.value().contains(b_idx));

        // FIRST(S) = {a, b}
        const auto& s_firsts = f.get_nterm_firsts(s_idx);
        REQUIRE(s_firsts.has_value());
        REQUIRE(s_firsts.value().get_count() == 2);
        REQUIRE(s_firsts.value().contains(a_idx));
        REQUIRE(s_firsts.value().contains(b_idx));

        // $root -> S (r=0, symbols: S)
        const auto& root_r0_p0 = f.get_rside_part_firsts(root_idx, 0, 0);
        REQUIRE(root_r0_p0.has_value());
        REQUIRE(root_r0_p0.value().get_count() == 2);
        REQUIRE(root_r0_p0.value().contains(a_idx));
        REQUIRE(root_r0_p0.value().contains(b_idx));

        // S -> a (r=0, symbols: a)
        const auto& s_r0_p0 = f.get_rside_part_firsts(s_idx, s_r0, 0);
        REQUIRE(s_r0_p0.has_value());
        REQUIRE(s_r0_p0.value().get_count() == 1);
        REQUIRE(s_r0_p0.value().contains(a_idx));

        // S -> b (r=1, symbols: b)
        const auto& s_r1_p0 = f.get_rside_part_firsts(s_idx, s_r1, 0);
        REQUIRE(s_r1_p0.has_value());
        REQUIRE(s_r1_p0.value().get_count() == 1);
        REQUIRE(s_r1_p0.value().contains(b_idx));
    }

    SECTION("grammar with epsilon production")
    {
        ptg::symbol_collection sc;
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        size_t x_idx = sc.add_term("x");
        size_t y_idx = sc.add_term("y");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"A", "y"});
        rs.add_rule("A", {"x"});
        rs.add_rule("A", {});

        ptg::firsts f(rs);
        f.calculate_all();

        size_t root_idx = 0;

        // FIRST($root) = FIRST(S) = {x, y}
        const auto& root_firsts = f.get_nterm_firsts(root_idx);
        REQUIRE(root_firsts.has_value());
        REQUIRE(root_firsts.value().contains(x_idx));
        REQUIRE(root_firsts.value().contains(y_idx));
        REQUIRE(root_firsts.value().get_count() == 2);

        // FIRST(S) = {x, y}
        const auto& s_firsts = f.get_nterm_firsts(s_idx);
        REQUIRE(s_firsts.has_value());
        REQUIRE(s_firsts.value().get_count() == 2);
        REQUIRE(s_firsts.value().contains(x_idx));
        REQUIRE(s_firsts.value().contains(y_idx));

        // FIRST(A) = {x}
        const auto& a_firsts = f.get_nterm_firsts(a_idx);
        REQUIRE(a_firsts.has_value());
        REQUIRE(a_firsts.value().get_count() == 1);
        REQUIRE(a_firsts.value().contains(x_idx));

        // $root -> S (r=0, p0: S)
        const auto& root_r0_p0 = f.get_rside_part_firsts(root_idx, 0, 0);
        REQUIRE(root_r0_p0.has_value());
        REQUIRE(root_r0_p0.value().get_count() == 2);
        REQUIRE(root_r0_p0.value().contains(x_idx));
        REQUIRE(root_r0_p0.value().contains(y_idx));

        // S -> A y (r=0, symbols: A y)
        // p0: A y -> {x, y} (since A nullable)
        const auto& s_r0_p0 = f.get_rside_part_firsts(s_idx, 0, 0);
        REQUIRE(s_r0_p0.has_value());
        REQUIRE(s_r0_p0.value().get_count() == 2);
        REQUIRE(s_r0_p0.value().contains(x_idx));
        REQUIRE(s_r0_p0.value().contains(y_idx));

        // p1: y -> {y}
        const auto& s_r0_p1 = f.get_rside_part_firsts(s_idx, 0, 1);
        REQUIRE(s_r0_p1.has_value());
        REQUIRE(s_r0_p1.value().get_count() == 1);
        REQUIRE(s_r0_p1.value().contains(y_idx));

        // A -> x (r=0, p0: x) -> {x}
        const auto& a_r0_p0 = f.get_rside_part_firsts(a_idx, 0, 0);
        REQUIRE(a_r0_p0.has_value());
        REQUIRE(a_r0_p0.value().get_count() == 1);
        REQUIRE(a_r0_p0.value().contains(x_idx));

        // A -> eps (r=1, no symbols, but cannot query p0 since symbol_count=0)
    }

    SECTION("chain of nullable non-terminals")
    {
        ptg::symbol_collection sc;
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        size_t b_idx = sc.add_nterm("B");
        size_t x_idx = sc.add_term("x");
        size_t y_idx = sc.add_term("y");
        size_t z_idx = sc.add_term("z");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"A", "B", "z"});
        rs.add_rule("A", {"x"});
        rs.add_rule("A", {});
        rs.add_rule("B", {"y"});
        rs.add_rule("B", {});

        ptg::firsts f(rs);
        f.calculate_all();

        size_t root_idx = 0;

        // FIRST($root) = FIRST(S) = {x, y, z}
        const auto& root_firsts = f.get_nterm_firsts(root_idx);
        REQUIRE(root_firsts.has_value());
        REQUIRE(root_firsts.value().get_count() == 3);
        REQUIRE(root_firsts.value().contains(x_idx));
        REQUIRE(root_firsts.value().contains(y_idx));
        REQUIRE(root_firsts.value().contains(z_idx));

        // FIRST(S) = {x, y, z}
        const auto& s_firsts = f.get_nterm_firsts(s_idx);
        REQUIRE(s_firsts.has_value());
        REQUIRE(s_firsts.value().get_count() == 3);
        REQUIRE(s_firsts.value().contains(x_idx));
        REQUIRE(s_firsts.value().contains(y_idx));
        REQUIRE(s_firsts.value().contains(z_idx));

        // FIRST(A) = {x}
        const auto& a_firsts = f.get_nterm_firsts(a_idx);
        REQUIRE(a_firsts.has_value());
        REQUIRE(a_firsts.value().get_count() == 1);
        REQUIRE(a_firsts.value().contains(x_idx));

        // FIRST(B) = {y}
        const auto& b_firsts = f.get_nterm_firsts(b_idx);
        REQUIRE(b_firsts.has_value());
        REQUIRE(b_firsts.value().get_count() == 1);
        REQUIRE(b_firsts.value().contains(y_idx));

        // S -> A B z (r=0)
        // p0: A B z -> {x, y, z}
        const auto& s_r0_p0 = f.get_rside_part_firsts(s_idx, s_r0, 0);
        REQUIRE(s_r0_p0.has_value());
        REQUIRE(s_r0_p0.value().get_count() == 3);
        REQUIRE(s_r0_p0.value().contains(x_idx));
        REQUIRE(s_r0_p0.value().contains(y_idx));
        REQUIRE(s_r0_p0.value().contains(z_idx));

        // p1: B z -> {y, z}
        const auto& s_r0_p1 = f.get_rside_part_firsts(s_idx, s_r0, 1);
        REQUIRE(s_r0_p1.has_value());
        REQUIRE(s_r0_p1.value().get_count() == 2);
        REQUIRE(s_r0_p1.value().contains(y_idx));
        REQUIRE(s_r0_p1.value().contains(z_idx));

        // p2: z -> {z}
        const auto& s_r0_p2 = f.get_rside_part_firsts(s_idx, s_r0, 2);
        REQUIRE(s_r0_p2.has_value());
        REQUIRE(s_r0_p2.value().get_count() == 1);
        REQUIRE(s_r0_p2.value().contains(z_idx));
    }

    SECTION("non-nullable in chain")
    {
        ptg::symbol_collection sc;
        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("A");
        sc.add_nterm("B");
        size_t x_idx = sc.add_term("x");
        size_t y_idx = sc.add_term("y");
        size_t z_idx = sc.add_term("z");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"A", "B", "z"});
        rs.add_rule("A", {"x"});
        rs.add_rule("A", {});
        rs.add_rule("B", {"y"});  // B not nullable

        ptg::firsts f(rs);
        f.calculate_all();

        // FIRST(S) = {x, y}
        const auto& s_firsts = f.get_nterm_firsts(s_idx);
        REQUIRE(s_firsts.has_value());
        REQUIRE(s_firsts.value().get_count() == 2);
        REQUIRE(s_firsts.value().contains(x_idx));
        REQUIRE(s_firsts.value().contains(y_idx));

        // S -> A B z
        // p0: A B z -> {x, y} (A null, then B first {y}, B not null, stop)
        const auto& s_r0_p0 = f.get_rside_part_firsts(s_idx, s_r0, 0);
        REQUIRE(s_r0_p0.has_value());
        REQUIRE(s_r0_p0.value().get_count() == 2);
        REQUIRE(s_r0_p0.value().contains(x_idx));
        REQUIRE(s_r0_p0.value().contains(y_idx));

        // p1: B z -> {y}
        const auto& s_r0_p1 = f.get_rside_part_firsts(s_idx, s_r0, 1);
        REQUIRE(s_r0_p1.has_value());
        REQUIRE(s_r0_p1.value().get_count() == 1);
        REQUIRE(s_r0_p1.value().contains(y_idx));

        // p2: z -> {z}
        const auto& s_r0_p2 = f.get_rside_part_firsts(s_idx, s_r0, 2);
        REQUIRE(s_r0_p2.has_value());
        REQUIRE(s_r0_p2.value().get_count() == 1);
        REQUIRE(s_r0_p2.value().contains(z_idx));
    }

    SECTION("expression grammar")
    {
        ptg::symbol_collection sc;
        size_t expr_idx = sc.add_nterm("Expr");
        size_t term_idx = sc.add_nterm("Term");
        size_t factor_idx = sc.add_nterm("Factor");
        size_t plus_idx = sc.add_term("+");
        sc.add_term("*");
        size_t lparen_idx = sc.add_term("(");
        size_t rparen_idx = sc.add_term(")");
        size_t id_idx = sc.add_term("id");

        ptg::ruleset rs(sc);
        rs.set_root("Expr");
        size_t expr_r0 = rs.add_rule("Expr", {"Expr", "+", "Term"});
        rs.add_rule("Expr", {"Term"});
        rs.add_rule("Term", {"Term", "*", "Factor"});
        rs.add_rule("Term", {"Factor"});
        size_t factor_r0 = rs.add_rule("Factor", {"(", "Expr", ")"});
        rs.add_rule("Factor", {"id"});

        ptg::firsts f(rs);
        f.calculate_all();

        // Common FIRST for Expr, Term, Factor = {(, id}
        const auto& expr_firsts = f.get_nterm_firsts(expr_idx);
        REQUIRE(expr_firsts.has_value());
        REQUIRE(expr_firsts.value().get_count() == 2);
        REQUIRE(expr_firsts.value().contains(lparen_idx));
        REQUIRE(expr_firsts.value().contains(id_idx));

        const auto& term_firsts = f.get_nterm_firsts(term_idx);
        REQUIRE(term_firsts.has_value());
        REQUIRE(term_firsts.value().get_count() == 2);
        REQUIRE(term_firsts.value().contains(lparen_idx));
        REQUIRE(term_firsts.value().contains(id_idx));

        const auto& factor_firsts = f.get_nterm_firsts(factor_idx);
        REQUIRE(factor_firsts.has_value());
        REQUIRE(factor_firsts.value().get_count() == 2);
        REQUIRE(factor_firsts.value().contains(lparen_idx));
        REQUIRE(factor_firsts.value().contains(id_idx));

        // Expr -> Expr + Term (r=0)
        // p0: Expr + Term -> {(, id} (since Expr not nullable)
        const auto& expr_r0_p0 = f.get_rside_part_firsts(expr_idx, expr_r0, 0);
        REQUIRE(expr_r0_p0.has_value());
        REQUIRE(expr_r0_p0.value().contains(lparen_idx));
        REQUIRE(expr_r0_p0.value().contains(id_idx));
        REQUIRE(expr_r0_p0.value().get_count() == 2);

        // p1: + Term -> {+}
        const auto& expr_r0_p1 = f.get_rside_part_firsts(expr_idx, expr_r0, 1);
        REQUIRE(expr_r0_p1.has_value());
        REQUIRE(expr_r0_p1.value().get_count() == 1);
        REQUIRE(expr_r0_p1.value().contains(plus_idx));

        // p2: Term -> {(, id}
        const auto& expr_r0_p2 = f.get_rside_part_firsts(expr_idx, expr_r0, 2);
        REQUIRE(expr_r0_p2.has_value());
        REQUIRE(expr_r0_p2.value().get_count() == 2);
        REQUIRE(expr_r0_p2.value().contains(lparen_idx));
        REQUIRE(expr_r0_p2.value().contains(id_idx));

        // Factor -> ( Expr ) (r=0)
        // p0: ( Expr ) -> {(}
        const auto& factor_r0_p0 = f.get_rside_part_firsts(factor_idx, factor_r0, 0);
        REQUIRE(factor_r0_p0.has_value());
        REQUIRE(factor_r0_p0.value().get_count() == 1);
        REQUIRE(factor_r0_p0.value().contains(lparen_idx));

        // p1: Expr ) -> {(, id}
        const auto& factor_r0_p1 = f.get_rside_part_firsts(factor_idx, factor_r0, 1);
        REQUIRE(factor_r0_p1.has_value());
        REQUIRE(factor_r0_p1.value().get_count() == 2);
        REQUIRE(factor_r0_p1.value().contains(lparen_idx));
        REQUIRE(factor_r0_p1.value().contains(id_idx));

        // p2: ) -> {)}
        const auto& factor_r0_p2 = f.get_rside_part_firsts(factor_idx, factor_r0, 2);
        REQUIRE(factor_r0_p2.has_value());
        REQUIRE(factor_r0_p2.value().get_count() == 1);
        REQUIRE(factor_r0_p2.value().contains(rparen_idx));
    }

    SECTION("invalid indices")
    {
        ptg::symbol_collection sc;
        sc.add_nterm("S");
        sc.add_term("a");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"a"});

        ptg::firsts f(rs);
        f.calculate_all();

        REQUIRE_THROWS_MATCHES(
            f.get_nterm_firsts(2),
            std::out_of_range,
            Message("Nterm index out of range")
        );
        
        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(2, 0, 0),
            std::out_of_range,
            Message("Nterm index out of range")
        );
        
        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(0, 1, 0),
            std::out_of_range,
            Message("Rside index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(1, 0, 1),
            std::out_of_range,
            Message("Symbol index out of range")
        );
    }

    SECTION("cycle detection (left recursion) should leave the set as nullopt")
    {
        ptg::symbol_collection sc;
        size_t s_idx = sc.add_nterm("S");
        sc.add_term("a");

        ptg::ruleset rs(sc);
        size_t r_idx = rs.add_rule("S", {"S", "a"});  // Left recursive

        ptg::firsts f(rs);
        f.calculate_all();
        
        REQUIRE_FALSE(f.get_nterm_firsts(s_idx).has_value());
        REQUIRE_FALSE(f.get_rside_part_firsts(s_idx, r_idx, 0).has_value());
    }
}
