#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("nullable computation", "[nullable]")
{
    symbol_collection sc;
    
    SECTION("no nullables")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");

        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        [[maybe_unused]] size_t root_idx = 0;
        
        rs.validate();
        
        REQUIRE_FALSE(rs.is_suffix_nullable(root_idx, 0, 0));

        REQUIRE_FALSE(rs.is_nterm_nullable(root_idx));
        REQUIRE_FALSE(rs.is_nterm_nullable(s_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE_FALSE(rs.is_suffix_nullable(root_idx, 0, 0));

        // S -> a (r=0, scount=1)
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 0));
    }

    SECTION("direct empty production")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        
        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {});

        rs.validate();
        
        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(rs.is_nterm_nullable(root_idx));
        REQUIRE(rs.is_nterm_nullable(s_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(root_idx, 0, 0));
    }

    SECTION("indirect nullable through chain")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_nterm("C");

        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A"});
        [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"B", "A"});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"C"});
        rs.validate();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(rs.is_nterm_nullable(root_idx));
        REQUIRE(rs.is_nterm_nullable(s_idx));
        REQUIRE(rs.is_nterm_nullable(a_idx));
        REQUIRE(rs.is_nterm_nullable(b_idx));
        REQUIRE(rs.is_nterm_nullable(c_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(root_idx, 0, 0));

        // S -> C (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 0));

        // B -> A (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(b_idx, b_r0, 0));

        // C -> B A (r=0, scount=2)
        REQUIRE(rs.is_suffix_nullable(c_idx, c_r0, 0));
        REQUIRE(rs.is_suffix_nullable(c_idx, c_r0, 1));
    }

    SECTION("partial nullables")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_term("c");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "c", "B"});

        rs.validate();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE_FALSE(rs.is_nterm_nullable(root_idx));
        REQUIRE_FALSE(rs.is_nterm_nullable(s_idx));
        REQUIRE(rs.is_nterm_nullable(a_idx));
        REQUIRE(rs.is_nterm_nullable(b_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE_FALSE(rs.is_suffix_nullable(root_idx, 0, 0));
        
        // S -> A c B (r=0, scount=3)
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 0));
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 1));
        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 2));
    }

    SECTION("multiple productions, some nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {"b"});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"b"});

        rs.validate();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(rs.is_nterm_nullable(root_idx));
        REQUIRE(rs.is_nterm_nullable(s_idx));
        REQUIRE(rs.is_nterm_nullable(a_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(root_idx, 0, 0));

        // S -> A (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 0));

        // S -> b (r=1, scount=1)
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r1, 0));

        // A -> b (r=1, scount=1)
        REQUIRE_FALSE(rs.is_suffix_nullable(a_idx, a_r1, 0));
    }

    SECTION("suffixes with mixed terms and nullables")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t x_idx = sc.add_term("x");
        [[maybe_unused]] size_t y_idx = sc.add_term("y");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "x", "B", "y"});

        rs.validate();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE_FALSE(rs.is_nterm_nullable(root_idx));
        REQUIRE_FALSE(rs.is_nterm_nullable(s_idx));
        REQUIRE(rs.is_nterm_nullable(a_idx));
        REQUIRE(rs.is_nterm_nullable(b_idx));

        // $root -> S
        REQUIRE_FALSE(rs.is_suffix_nullable(root_idx, 0, 0));

        // S -> A x B y (r=0, scount=4)
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 0));
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 1));
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 2));
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 3));
    }

    SECTION("suffixes with trailing nullables")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t x_idx = sc.add_term("x");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"x", "A", "B"});

        rs.validate();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE_FALSE(rs.is_nterm_nullable(root_idx));
        REQUIRE_FALSE(rs.is_nterm_nullable(s_idx));
        REQUIRE(rs.is_nterm_nullable(a_idx));
        REQUIRE(rs.is_nterm_nullable(b_idx));

        // $root -> S
        REQUIRE_FALSE(rs.is_suffix_nullable(root_idx, 0, 0));

        // S -> x A B (scount=3)
        REQUIRE_FALSE(rs.is_suffix_nullable(s_idx, s_r0, 0));
        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 1));
        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 2));
    }

    SECTION("complex grammar with varying lengths")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
        [[maybe_unused]] size_t term_idx = sc.add_nterm("Term");
        [[maybe_unused]] size_t plus_idx = sc.add_term("+");
        [[maybe_unused]] size_t id_idx = sc.add_term("id");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
        [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"Expr", "+", "Term"});
        [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"Term"});
        [[maybe_unused]] size_t term_r0 = rs.add_rule("Term", {"id"});
        [[maybe_unused]] size_t term_r1 = rs.add_rule("Term", {});  // Make Term nullable for test

        rs.validate();

        [[maybe_unused]] size_t root_idx = 0;

        REQUIRE(rs.is_nterm_nullable(root_idx));
        REQUIRE(rs.is_nterm_nullable(s_idx));
        REQUIRE(rs.is_nterm_nullable(expr_idx));
        REQUIRE(rs.is_nterm_nullable(term_idx));

        // $root -> S (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(root_idx, 0, 0));

        // S -> Expr (r=0, scount=1)
        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 0));

        // Expr -> Expr + Term (r=0, scount=3)
        REQUIRE_FALSE(rs.is_suffix_nullable(expr_idx, expr_r0, 0));
        REQUIRE_FALSE(rs.is_suffix_nullable(expr_idx, expr_r0, 1));
        REQUIRE(rs.is_suffix_nullable(expr_idx, expr_r0, 2));

        // Expr -> Term (r=1, scount=1)
        REQUIRE(rs.is_suffix_nullable(expr_idx, expr_r1, 0));

        // Term -> id (r=0, scount=1)
        REQUIRE_FALSE(rs.is_suffix_nullable(term_idx, term_r0, 0));
    }
    
    SECTION("invalid indices in is_nterm_nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.is_nterm_nullable(rs.get_nterm_count()),  // Invalid nterm_idx
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("invalid indices in is_suffix_nullable")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_term("a");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.is_suffix_nullable(rs.get_nterm_count(), 0, 0),
            std::out_of_range,
            Message("Nterm index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            rs.is_suffix_nullable(0, rs.get_nterm_rside_count(0), 0),
            std::out_of_range,
            Message("Rside index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            rs.is_suffix_nullable(0, 0, rs.get_symbol_count(0, 0)),
            std::out_of_range,
            Message("Symbol index out of range")
        );
    }

    SECTION("nterm with left recursion")
    {
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

        rs.validate();

        REQUIRE(rs.is_nterm_nullable(a_idx) == true);
    }

    SECTION("nterm with left recursion no epsilon - returns false")
    {
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_term("b");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {"b"});

        rs.validate();

        REQUIRE(rs.is_nterm_nullable(a_idx) == false);
    }

    SECTION("suffix with recursion")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"S"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

        rs.validate();

        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 0) == true);

        REQUIRE(rs.is_nterm_nullable(s_idx) == true);
    }

    SECTION("suffix with unresolved recursion returns false")
    {
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"S"});

        rs.validate();

        REQUIRE(rs.is_suffix_nullable(s_idx, s_r0, 0) == false);

        REQUIRE(rs.is_nterm_nullable(s_idx) == false);
    }
}
