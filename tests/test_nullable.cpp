#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <nullable.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("nullable computation", "[nullable]")
{
    ptg::symbol_collection sc;
    
    SECTION("no nullables")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE_FALSE(n.is_nullable_nterm(root_idx));
        REQUIRE_FALSE(n.is_nullable_nterm(s_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(root_idx, 0, 0));

        // S -> a (r=0, scount=1)
        REQUIRE_FALSE(n.is_nullable_rside_part(s_idx, s_r0, 0));
    }

    SECTION("direct empty production")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(n.is_nullable_nterm(root_idx));
        REQUIRE(n.is_nullable_nterm(s_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(n.is_nullable_rside_part(root_idx, 0, 0));
    }

    SECTION("indirect nullable through chain")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_nterm("C");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A"});
        [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"B", "A"});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"C"});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

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
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_term("c");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "c", "B"});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

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
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {"b"});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"b"});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

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
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t x_idx = sc.add_term("x");
        [[maybe_unused]] size_t y_idx = sc.add_term("y");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "x", "B", "y"});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

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
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t x_idx = sc.add_term("x");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"x", "A", "B"});

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

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
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
        [[maybe_unused]] size_t term_idx = sc.add_nterm("Term");
        [[maybe_unused]] size_t plus_idx = sc.add_term("+");
        [[maybe_unused]] size_t id_idx = sc.add_term("id");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
        [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"Expr", "+", "Term"});
        [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"Term"});
        [[maybe_unused]] size_t term_r0 = rs.add_rule("Term", {"id"});
        [[maybe_unused]] size_t term_r1 = rs.add_rule("Term", {});  // Make Term nullable for test

        ptg::nullable n(rs);
        n.calculate_all();

        [[maybe_unused]] size_t root_idx = 0;

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
    
    SECTION("invalid indices in is_nullable_nterm")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        ptg::nullable null(rs);

        REQUIRE_THROWS_MATCHES(
            null.is_nullable_nterm(rs.get_nterm_count()),  // Invalid nterm_idx
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("invalid indices in is_nullable_rside_part")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        ptg::nullable null(rs);

        REQUIRE_THROWS_MATCHES(
            null.is_nullable_rside_part(rs.get_nterm_count(), 0, 0),
            std::out_of_range,
            Message("Nterm index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            null.is_nullable_rside_part(0, rs.get_nterm_rside_count(0), 0),
            std::out_of_range,
            Message("Rside index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            null.is_nullable_rside_part(0, 0, rs.get_symbol_count(0, 0)),
            std::out_of_range,
            Message("Symbol index out of range")
        );
    }

    SECTION("individual calculate_nterm - nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {});

        ptg::nullable n(rs);

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(n.calculate_nterm(s_idx) == true);
        REQUIRE(n.is_nullable_nterm(s_idx) == true);

        REQUIRE(n.calculate_nterm(root_idx) == true);
        REQUIRE(n.is_nullable_nterm(root_idx) == true);
    }

    SECTION("individual calculate_nterm - not nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        ptg::nullable n(rs);

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(n.calculate_nterm(s_idx) == false);
        REQUIRE(n.is_nullable_nterm(s_idx) == false);

        REQUIRE(n.calculate_nterm(root_idx) == false);
        REQUIRE(n.is_nullable_nterm(root_idx) == false);
    }

    SECTION("individual calculate_rside_part - nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "B"});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 0) == true);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0) == true);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 1) == true);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 1) == true);
    }

    SECTION("individual calculate_rside_part - not nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "b"});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 0) == false);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0) == false);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 1) == false);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 1) == false);
    }

    SECTION("calculate_nterm with left recursion")
    {
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_nterm(a_idx) == true);  // Because of epsilon rule
        REQUIRE(n.is_nullable_nterm(a_idx) == true);
    }

    SECTION("calculate_nterm with left recursion no epsilon - returns false")
    {
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {"b"});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_nterm(a_idx) == false);
        REQUIRE(n.is_nullable_nterm(a_idx) == false);
    }

    SECTION("calculate_rside_part with recursion")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"S"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 0) == true);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0) == true);

        REQUIRE(n.calculate_nterm(s_idx) == true);
        REQUIRE(n.is_nullable_nterm(s_idx) == true);
    }

    SECTION("calculate_rside_part with unresolved recursion returns false")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"S"});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 0) == false);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0) == false);

        REQUIRE(n.calculate_nterm(s_idx) == false);
        REQUIRE(n.is_nullable_nterm(s_idx) == false);
    }

    SECTION("mix individual and all")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");

        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A"});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"B"});

        ptg::nullable n(rs);

        REQUIRE(n.calculate_nterm(a_idx) == true);
        REQUIRE(n.is_nullable_nterm(a_idx) == true);

        REQUIRE(n.calculate_rside_part(s_idx, s_r0, 0) == true);
        REQUIRE(n.is_nullable_rside_part(s_idx, s_r0, 0) == true);

        n.calculate_all();

        REQUIRE(n.is_nullable_nterm(s_idx) == true);
        REQUIRE(n.is_nullable_nterm(b_idx) == true);
    }
}
