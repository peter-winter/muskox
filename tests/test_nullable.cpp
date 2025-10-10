#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <nullable.h>
#include <ruleset.h>
#include <symbol_collection.h>

TEST_CASE("nullable computation", "[nullable]")
{
    ptg::symbol_collection sc;
    
    SECTION("no nullables")
    {
        size_t s_idx = sc.add_nterm("S");
        sc.add_term("a");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"a"});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE_FALSE(n.is_nullable_nterm(root_idx));
        REQUIRE_FALSE(n.is_nullable_nterm(s_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> a (r=0, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 0));
    }

    SECTION("direct empty production")
    {
        size_t s_idx = sc.add_nterm("S");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE(n.is_nullable_nterm(root_idx));
        REQUIRE(n.is_nullable_nterm(s_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(root_idx, 0, 0));
    }

    SECTION("indirect nullable through chain")
    {
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        size_t b_idx = sc.add_nterm("B");
        size_t c_idx = sc.add_nterm("C");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {});
        size_t b_r0 = rs.add_rule("B", {"A"});
        size_t c_r0 = rs.add_rule("C", {"B", "A"});
        size_t s_r0 = rs.add_rule("S", {"C"});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE(n.is_nullable_nterm(root_idx));
        REQUIRE(n.is_nullable_nterm(s_idx));
        REQUIRE(n.is_nullable_nterm(a_idx));
        REQUIRE(n.is_nullable_nterm(b_idx));
        REQUIRE(n.is_nullable_nterm(c_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> C (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0));

        // B -> A (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(b_idx, b_r0, 0));

        // C -> B A (r=0, scount=2)
        REQUIRE(n.is_nullable_rside_part(c_idx, c_r0, 0));
        REQUIRE(n.is_nullable_rside_part(c_idx, c_r0, 1));
    }

    SECTION("partial nullables")
    {
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        size_t b_idx = sc.add_nterm("B");
        sc.add_term("c");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {});
        rs.add_rule("B", {});
        size_t s_r0 = rs.add_rule("S", {"A", "c", "B"});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE_FALSE(n.is_nullable_nterm(root_idx));
        REQUIRE_FALSE(n.is_nullable_nterm(s_idx));
        REQUIRE(n.is_nullable_nterm(a_idx));
        REQUIRE(n.is_nullable_nterm(b_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(root_idx, 0, 0));
        
        // S -> A c B (r=0, scount=3)
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 0));
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 1));
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 2));
    }

    SECTION("multiple productions, some nullable")
    {
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        sc.add_term("b");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {});
        size_t a_r1 = rs.add_rule("A", {"b"});
        size_t s_r0 = rs.add_rule("S", {"A"});
        size_t s_r1 = rs.add_rule("S", {"b"});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE(n.is_nullable_nterm(root_idx));
        REQUIRE(n.is_nullable_nterm(s_idx));
        REQUIRE(n.is_nullable_nterm(a_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> A (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0));

        // S -> b (r=1, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r1, 0));

        // A -> b (r=1, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(a_idx, a_r1, 0));
    }

    SECTION("suffixes with mixed terms and nullables")
    {
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        size_t b_idx = sc.add_nterm("B");
        sc.add_term("x");
        sc.add_term("y");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {});
        rs.add_rule("B", {});
        size_t s_r0 = rs.add_rule("S", {"A", "x", "B", "y"});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE_FALSE(n.is_nullable_nterm(root_idx));
        REQUIRE_FALSE(n.is_nullable_nterm(s_idx));
        REQUIRE(n.is_nullable_nterm(a_idx));
        REQUIRE(n.is_nullable_nterm(b_idx));

        // $root -> S
        REQUIRE_FALSE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> A x B y (r=0, scount=4)
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 0));
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 1));
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 2));
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 3));
    }

    SECTION("suffixes with trailing nullables")
    {
        size_t s_idx = sc.add_nterm("S");
        size_t a_idx = sc.add_nterm("A");
        size_t b_idx = sc.add_nterm("B");
        sc.add_term("x");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {});
        rs.add_rule("B", {});
        size_t s_r0 = rs.add_rule("S", {"x", "A", "B"});

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE_FALSE(n.is_nullable_nterm(root_idx));
        REQUIRE_FALSE(n.is_nullable_nterm(s_idx));
        REQUIRE(n.is_nullable_nterm(a_idx));
        REQUIRE(n.is_nullable_nterm(b_idx));

        // $root -> S
        REQUIRE_FALSE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> x A B (scount=3)
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 0));
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 1));
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 2));
    }

    SECTION("complex grammar with varying lengths")
    {
        size_t s_idx = sc.add_nterm("S");
        size_t expr_idx = sc.add_nterm("Expr");
        size_t term_idx = sc.add_nterm("Term");
        sc.add_term("+");
        sc.add_term("id");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"Expr"});
        size_t expr_r0 = rs.add_rule("Expr", {"Expr", "+", "Term"});
        size_t expr_r1 = rs.add_rule("Expr", {"Term"});
        size_t term_r0 = rs.add_rule("Term", {"id"});
        rs.add_rule("Term", {});  // Make Term nullable for test

        ptg::nullable n(rs);
        n.calculate_all();

        size_t root_idx = 0;

        REQUIRE(n.is_nullable_nterm(root_idx));
        REQUIRE(n.is_nullable_nterm(s_idx));
        REQUIRE(n.is_nullable_nterm(expr_idx));
        REQUIRE(n.is_nullable_nterm(term_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> Expr (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0));

        // Expr -> Expr + Term (r=0, scount=3)
        REQUIRE_FALSE(n.is_nullable_rside_part(expr_idx, expr_r0, 0));
        REQUIRE_FALSE(n.is_nullable_rside_part(expr_idx, expr_r0, 1));
        REQUIRE(n.is_nullable_rside_part(expr_idx, expr_r0, 2));

        // Expr -> Term (r=1, scount=1)
        REQUIRE(n.is_nullable_rside_part(expr_idx, expr_r1, 0));

        // Term -> id (r=0, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(term_idx, term_r0, 0));
    }
}
