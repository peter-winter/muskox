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

    SECTION("get_suffix_first invalid nterm")
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

    SECTION("get_suffix_first on epsilon production")
    {
        symbol_collection sc_eps;
        [[maybe_unused]] size_t eps_idx = sc_eps.add_nterm("Eps");
        sc_eps.validate();

        ruleset rs_eps(sc_eps);
        [[maybe_unused]] size_t eps_r0 = rs_eps.add_rule("Eps", {});
        rs_eps.validate();

        REQUIRE_THROWS_MATCHES(
            rs_eps.get_suffix_first(eps_idx, eps_r0, 0),
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

    SECTION("get_nterm_first multiple productions")
    {
        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(a_term_idx));
        REQUIRE(s_first.contains(b_term_idx));
    }

    SECTION("get_suffix_first terminal")
    {
        const auto& part0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(part0.value().get_count() == 1);
        REQUIRE(part0.value().contains(a_term_idx));

        const auto& part1 = rs.get_suffix_first(s_idx, s_r1, 0);
        REQUIRE(part1.value().get_count() == 1);
        REQUIRE(part1.value().contains(b_term_idx));
    }

    SECTION("get_suffix_first mid rule")
    {
        symbol_collection sc_multi;
        [[maybe_unused]] size_t s_multi_idx = sc_multi.add_nterm("S");
        [[maybe_unused]] size_t a_multi = sc_multi.add_term("a");
        [[maybe_unused]] size_t b_multi = sc_multi.add_term("b");
        sc_multi.validate();

        ruleset rs_multi(sc_multi);
        [[maybe_unused]] size_t s_multi_r0 = rs_multi.add_rule("S", {"a", "b"});
        rs_multi.validate();

        const auto& part0 = rs_multi.get_suffix_first(s_multi_idx, s_multi_r0, 0);
        REQUIRE(part0.value().get_count() == 1);
        REQUIRE(part0.value().contains(a_multi));

        const auto& part1 = rs_multi.get_suffix_first(s_multi_idx, s_multi_r0, 1);
        REQUIRE(part1.value().get_count() == 1);
        REQUIRE(part1.value().contains(b_multi));
    }
}

TEST_CASE("firsts chain propagation", "[firsts]")
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

    SECTION("chain propagation top-down add order")
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

        // Suffix firsts
        const auto& s_suff0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(s_suff0.value().get_count() == 1);
        REQUIRE(s_suff0.value().contains(d_term_idx));

        const auto& a_suff0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(a_suff0.value().get_count() == 1);
        REQUIRE(a_suff0.value().contains(d_term_idx));

        const auto& b_suff0 = rs.get_suffix_first(b_idx, b_r0, 0);
        REQUIRE(b_suff0.value().get_count() == 1);
        REQUIRE(b_suff0.value().contains(d_term_idx));

        const auto& c_suff0 = rs.get_suffix_first(c_idx, c_r0, 0);
        REQUIRE(c_suff0.value().get_count() == 1);
        REQUIRE(c_suff0.value().contains(d_term_idx));
    }
}

TEST_CASE("firsts epsilon and nullable cases", "[firsts]")
{
    SECTION("pure epsilon non-terminal")
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

    SECTION("epsilon productions with terminals")
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

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(b_term_idx));

        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(b_term_idx));
        REQUIRE(s_first.contains(c_term_idx));

        const auto& part0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(part0.value().get_count() == 2);
        REQUIRE(part0.value().contains(b_term_idx));
        REQUIRE(part0.value().contains(c_term_idx));

        const auto& part1 = rs.get_suffix_first(s_idx, s_r0, 1);
        REQUIRE(part1.value().get_count() == 1);
        REQUIRE(part1.value().contains(c_term_idx));
    }

    SECTION("multiple nullables in chain")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        [[maybe_unused]] size_t d_term_idx = sc.add_term("d");
        [[maybe_unused]] size_t e_term_idx = sc.add_term("e");

        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "B", "e"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"c"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"d"});
        [[maybe_unused]] size_t b_r1 = rs.add_rule("B", {});

        rs.validate();

        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 3);
        REQUIRE(s_first.contains(c_term_idx));
        REQUIRE(s_first.contains(d_term_idx));
        REQUIRE(s_first.contains(e_term_idx));

        const auto& part0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(part0.value().get_count() == 3);
        REQUIRE(part0.value().contains(c_term_idx));
        REQUIRE(part0.value().contains(d_term_idx));
        REQUIRE(part0.value().contains(e_term_idx));

        const auto& part1 = rs.get_suffix_first(s_idx, s_r0, 1);
        REQUIRE(part1.value().get_count() == 2);
        REQUIRE(part1.value().contains(d_term_idx));
        REQUIRE(part1.value().contains(e_term_idx));

        const auto& part2 = rs.get_suffix_first(s_idx, s_r0, 2);
        REQUIRE(part2.value().get_count() == 1);
        REQUIRE(part2.value().contains(e_term_idx));
    }
}

TEST_CASE("firsts with implicit $root", "[firsts]")
{
    constexpr size_t root_idx = 0;  // $root is always nterm 0

    SECTION("user root non-nullable")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_term_idx = sc.add_term("a");

        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
        
        rs.validate();

        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 1);
        REQUIRE(s_first.contains(a_term_idx));

        const auto& root_first = rs.get_nterm_first(root_idx);
        REQUIRE(root_first.get_count() == 1);
        REQUIRE(root_first.contains(a_term_idx));
    }

    SECTION("user root nullable with terminals")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_term_idx = sc.add_term("a");

        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {});
        
        rs.validate();

        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 1);
        REQUIRE(s_first.contains(a_term_idx));

        const auto& root_first = rs.get_nterm_first(root_idx);
        REQUIRE(root_first.get_count() == 1);
        REQUIRE(root_first.contains(a_term_idx));
    }

    SECTION("user root pure epsilon")
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
        
        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(root_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );
    }

    SECTION("suffix firsts for $root")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_term_idx = sc.add_term("a");

        sc.validate();
        
        ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {});
        
        rs.validate();

        // $root has one rside: S 
        REQUIRE(rs.get_nterm_rside_count(root_idx) == 1);
        [[maybe_unused]] size_t root_r0 = 0;

        const auto& root_part0 = rs.get_suffix_first(root_idx, root_r0, 0);
        REQUIRE(root_part0.value().get_count() == 1);
        REQUIRE(root_part0.value().contains(a_term_idx));
    }
}

TEST_CASE("firsts left recursion handling", "[firsts]")
{
    SECTION("direct left recursion with base")
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

        // Suffix checks
        const auto& suff0_r0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(suff0_r0.value().get_count() == 1);
        REQUIRE(suff0_r0.value().contains(b_term_idx));

        const auto& suff1_r0 = rs.get_suffix_first(a_idx, a_r0, 1);
        REQUIRE(suff1_r0.value().get_count() == 1);
        REQUIRE(suff1_r0.value().contains(b_term_idx));
    }
    
    SECTION("direct left recursion no base")
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

        // Suffix checks
        REQUIRE_FALSE(rs.get_suffix_first(a_idx, a_r0, 0).has_value());

        const auto& suff1 = rs.get_suffix_first(a_idx, a_r0, 1);
        REQUIRE(suff1.value().get_count() == 1);
        REQUIRE(suff1.value().contains(b_term_idx));
    }

    SECTION("indirect left recursion no base")
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

        // Suffix checks
        REQUIRE_FALSE(rs.get_suffix_first(a_idx, a_r0, 0).has_value());
        REQUIRE_FALSE(rs.get_suffix_first(b_idx, b_r0, 0).has_value());
        
        const auto& suff1_b = rs.get_suffix_first(b_idx, b_r0, 1);
        REQUIRE(suff1_b.value().get_count() == 1);
        REQUIRE(suff1_b.value().contains(c_term_idx));
    }

    SECTION("indirect left recursion with base")
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

        // Suffix checks
        const auto& suff0_a0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(suff0_a0.value().get_count() == 1);
        REQUIRE(suff0_a0.value().contains(c_term_idx));

        const auto& suff0_a1 = rs.get_suffix_first(a_idx, a_r1, 0);
        REQUIRE(suff0_a1.value().get_count() == 1);
        REQUIRE(suff0_a1.value().contains(c_term_idx));

        const auto& suff0_b0 = rs.get_suffix_first(b_idx, b_r0, 0);
        REQUIRE(suff0_b0.value().get_count() == 1);
        REQUIRE(suff0_b0.value().contains(c_term_idx));

        const auto& suff1_b0 = rs.get_suffix_first(b_idx, b_r0, 1);
        REQUIRE(suff1_b0.value().get_count() == 1);
        REQUIRE(suff1_b0.value().contains(c_term_idx));
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

        // Suffix checks
        const auto& suff0_a = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(suff0_a.value().get_count() == 1);
        REQUIRE(suff0_a.value().contains(c_term_idx));

        const auto& suff1_a = rs.get_suffix_first(a_idx, a_r0, 1);
        REQUIRE(suff1_a.value().get_count() == 1);
        REQUIRE(suff1_a.value().contains(d_term_idx));

        const auto& suff0_b = rs.get_suffix_first(b_idx, b_r0, 0);
        REQUIRE(suff0_b.value().get_count() == 1);
        REQUIRE(suff0_b.value().contains(d_term_idx));

        const auto& suff1_b = rs.get_suffix_first(b_idx, b_r0, 1);
        REQUIRE(suff1_b.value().get_count() == 1);
        REQUIRE(suff1_b.value().contains(c_term_idx));
    }
}

TEST_CASE("firsts suffix queries in pure epsilon cases", "[firsts]")
{
    SECTION("suffix starting with single pure epsilon")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t e_idx = sc.add_nterm("E");
        [[maybe_unused]] size_t f_idx = sc.add_nterm("F");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t e_r0 = rs.add_rule("E", {});
        [[maybe_unused]] size_t f_r0 = rs.add_rule("F", {"E"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(e_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(f_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );
        
        REQUIRE_FALSE(rs.get_suffix_first(f_idx, f_r0, 0));
    }

    SECTION("suffix with pure epsilon followed by terminal")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t e_idx = sc.add_nterm("E");
        [[maybe_unused]] size_t g_idx = sc.add_nterm("G");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t e_r0 = rs.add_rule("E", {});
        [[maybe_unused]] size_t g_r0 = rs.add_rule("G", {"E", "c"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(e_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        const auto& g_first = rs.get_nterm_first(g_idx);
        REQUIRE(g_first.get_count() == 1);
        REQUIRE(g_first.contains(c_term_idx));

        const auto& suff0 = rs.get_suffix_first(g_idx, g_r0, 0);
        REQUIRE(suff0.value().get_count() == 1);
        REQUIRE(suff0.value().contains(c_term_idx));

        const auto& suff1 = rs.get_suffix_first(g_idx, g_r0, 1);
        REQUIRE(suff1.value().get_count() == 1);
        REQUIRE(suff1.value().contains(c_term_idx));
    }

    SECTION("suffix with multiple pure epsilons")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t e1_idx = sc.add_nterm("E1");
        [[maybe_unused]] size_t e2_idx = sc.add_nterm("E2");
        [[maybe_unused]] size_t i_idx = sc.add_nterm("I");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t e1_r0 = rs.add_rule("E1", {});
        [[maybe_unused]] size_t e2_r0 = rs.add_rule("E2", {});
        [[maybe_unused]] size_t i_r0 = rs.add_rule("I", {"E1", "E2"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(e1_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(e2_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(i_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        REQUIRE_FALSE(rs.get_suffix_first(i_idx, i_r0, 0));
        REQUIRE_FALSE(rs.get_suffix_first(i_idx, i_r0, 1));
    }

    SECTION("suffix with multiple pure epsilons followed by terminal")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t e1_idx = sc.add_nterm("E1");
        [[maybe_unused]] size_t e2_idx = sc.add_nterm("E2");
        [[maybe_unused]] size_t j_idx = sc.add_nterm("J");
        [[maybe_unused]] size_t d_term_idx = sc.add_term("d");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t e1_r0 = rs.add_rule("E1", {});
        [[maybe_unused]] size_t e2_r0 = rs.add_rule("E2", {});
        [[maybe_unused]] size_t j_r0 = rs.add_rule("J", {"E1", "E2", "d"});

        rs.validate();

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(e1_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_first(e2_idx),
            std::runtime_error,
            Message("No FIRST set for non-terminal")
        );

        const auto& j_first = rs.get_nterm_first(j_idx);
        REQUIRE(j_first.get_count() == 1);
        REQUIRE(j_first.contains(d_term_idx));

        const auto& suff0 = rs.get_suffix_first(j_idx, j_r0, 0);
        REQUIRE(suff0.value().get_count() == 1);
        REQUIRE(suff0.value().contains(d_term_idx));

        const auto& suff1 = rs.get_suffix_first(j_idx, j_r0, 1);
        REQUIRE(suff1.value().get_count() == 1);
        REQUIRE(suff1.value().contains(d_term_idx));

        const auto& suff2 = rs.get_suffix_first(j_idx, j_r0, 2);
        REQUIRE(suff2.value().get_count() == 1);
        REQUIRE(suff2.value().contains(d_term_idx));
    }
}

TEST_CASE("firsts add rule order variations", "[firsts]")
{
    SECTION("chain bottom-up add order")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_nterm("C");
        [[maybe_unused]] size_t d_term_idx = sc.add_term("d");

        sc.validate();
        
        ruleset rs(sc);
        // Add from leaves to root
        [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"d"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"C"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});

        rs.validate();

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

        // Suffix firsts
        const auto& s_suff0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(s_suff0.value().get_count() == 1);
        REQUIRE(s_suff0.value().contains(d_term_idx));

        const auto& a_suff0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(a_suff0.value().get_count() == 1);
        REQUIRE(a_suff0.value().contains(d_term_idx));

        const auto& b_suff0 = rs.get_suffix_first(b_idx, b_r0, 0);
        REQUIRE(b_suff0.value().get_count() == 1);
        REQUIRE(b_suff0.value().contains(d_term_idx));

        const auto& c_suff0 = rs.get_suffix_first(c_idx, c_r0, 0);
        REQUIRE(c_suff0.value().get_count() == 1);
        REQUIRE(c_suff0.value().contains(d_term_idx));
    }

    SECTION("nullable bottom-up")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");

        sc.validate();
        
        ruleset rs(sc);
        // Add A rules first (leaf)
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});
        // Then S
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "c"});

        rs.validate();

        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(b_term_idx));
        REQUIRE(s_first.contains(c_term_idx));

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(b_term_idx));

        // Suffix firsts
        const auto& s_suff0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(s_suff0.value().get_count() == 2);
        REQUIRE(s_suff0.value().contains(b_term_idx));
        REQUIRE(s_suff0.value().contains(c_term_idx));

        const auto& s_suff1 = rs.get_suffix_first(s_idx, s_r0, 1);
        REQUIRE(s_suff1.value().get_count() == 1);
        REQUIRE(s_suff1.value().contains(c_term_idx));

        const auto& a_suff0_r0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(a_suff0_r0.value().get_count() == 1);
        REQUIRE(a_suff0_r0.value().contains(b_term_idx));
    }

    SECTION("nullable top-down")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");

        sc.validate();
        
        ruleset rs(sc);
        // Add S first
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "c"});
        // Then A
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"b"});
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {});

        rs.validate();

        const auto& s_first = rs.get_nterm_first(s_idx);
        REQUIRE(s_first.get_count() == 2);
        REQUIRE(s_first.contains(b_term_idx));
        REQUIRE(s_first.contains(c_term_idx));

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(b_term_idx));

        // Suffix firsts
        const auto& s_suff0 = rs.get_suffix_first(s_idx, s_r0, 0);
        REQUIRE(s_suff0.value().get_count() == 2);
        REQUIRE(s_suff0.value().contains(b_term_idx));
        REQUIRE(s_suff0.value().contains(c_term_idx));

        const auto& s_suff1 = rs.get_suffix_first(s_idx, s_r0, 1);
        REQUIRE(s_suff1.value().get_count() == 1);
        REQUIRE(s_suff1.value().contains(c_term_idx));

        const auto& a_suff0_r0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(a_suff0_r0.value().get_count() == 1);
        REQUIRE(a_suff0_r0.value().contains(b_term_idx));
    }

    SECTION("left recursion added in mixed order")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        sc.validate();

        ruleset rs(sc);
        // Add indirect first
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A", "c"});
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"B"});
        // Then base
        [[maybe_unused]] size_t a_r1 = rs.add_rule("A", {"c"});

        rs.validate();

        const auto& a_first = rs.get_nterm_first(a_idx);
        REQUIRE(a_first.get_count() == 1);
        REQUIRE(a_first.contains(c_term_idx));

        const auto& b_first = rs.get_nterm_first(b_idx);
        REQUIRE(b_first.get_count() == 1);
        REQUIRE(b_first.contains(c_term_idx));

        // Suffix firsts
        const auto& a_suff0_r0 = rs.get_suffix_first(a_idx, a_r0, 0);
        REQUIRE(a_suff0_r0.value().get_count() == 1);
        REQUIRE(a_suff0_r0.value().contains(c_term_idx));

        const auto& a_suff0_r1 = rs.get_suffix_first(a_idx, a_r1, 0);
        REQUIRE(a_suff0_r1.value().get_count() == 1);
        REQUIRE(a_suff0_r1.value().contains(c_term_idx));

        const auto& b_suff0 = rs.get_suffix_first(b_idx, b_r0, 0);
        REQUIRE(b_suff0.value().get_count() == 1);
        REQUIRE(b_suff0.value().contains(c_term_idx));

        const auto& b_suff1 = rs.get_suffix_first(b_idx, b_r0, 1);
        REQUIRE(b_suff1.value().get_count() == 1);
        REQUIRE(b_suff1.value().contains(c_term_idx));
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

    SECTION("nterm firsts in complex")
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

    SECTION("suffix firsts in complex")
    {
        const auto& expr_part1 = rs.get_suffix_first(expr_idx, expr_r1, 1);  // Expr -> Expr . + Term
        REQUIRE(expr_part1.value().get_count() == 1);
        REQUIRE(expr_part1.value().contains(plus_idx));

        const auto& factor_part1 = rs.get_suffix_first(factor_idx, factor_r1, 1);  // Factor -> ( . Expr )
        REQUIRE(factor_part1.value().get_count() == 2);
        REQUIRE(factor_part1.value().contains(num_idx));
        REQUIRE(factor_part1.value().contains(lpar_idx));
    }
}
