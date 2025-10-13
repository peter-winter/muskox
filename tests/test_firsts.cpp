#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <firsts.h>
#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("firsts computation", "[firsts]")
{
    ptg::symbol_collection sc;

    SECTION("basic firsts")
    {
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");
        size_t c_idx = sc.add_term("c");

        size_t s_idx = sc.add_nterm("S");
        size_t expr_idx = sc.add_nterm("Expr");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"Expr"});
        rs.add_rule("Expr", {"a"});
        rs.add_rule("Expr", {"b", "Expr"});
        rs.add_rule("Expr", {"c"});

        ptg::firsts f(rs);
        f.calculate_all();

        auto& s_first = f.get_nterm_firsts(s_idx);
        REQUIRE(s_first.has_value());
        std::vector<size_t> s_got = s_first.value().get_indices();
        REQUIRE(s_got == std::vector<size_t>{a_idx, b_idx, c_idx});

        auto& expr_first = f.get_nterm_firsts(expr_idx);
        REQUIRE(expr_first.has_value());
        std::vector<size_t> expr_got = expr_first.value().get_indices();
        REQUIRE(expr_got == std::vector<size_t>{a_idx, b_idx, c_idx});
    }

    SECTION("epsilon production")
    {
        size_t a_idx = sc.add_term("a");

        size_t s_idx = sc.add_nterm("S");
        size_t a_nt_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"A"});
        rs.add_rule("A", {"a"});
        rs.add_rule("A", {});

        ptg::firsts f(rs);
        f.calculate_all();

        auto& s_first = f.get_nterm_firsts(s_idx);
        REQUIRE(s_first.has_value());
        REQUIRE(s_first.value().get_count() == 1);
        REQUIRE(s_first.value().contains(a_idx));

        auto& a_first = f.get_nterm_firsts(a_nt_idx);
        REQUIRE(a_first.has_value());
        REQUIRE(a_first.value().get_count() == 1);
        REQUIRE(a_first.value().contains(a_idx));
    }

    SECTION("nullable chain")
    {
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");

        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("X");
        sc.add_nterm("Y");
        sc.add_nterm("Z");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"X", "b"});
        rs.add_rule("X", {"Y"});
        rs.add_rule("Y", {"Z"});
        rs.add_rule("Z", {"a"});
        rs.add_rule("Z", {});

        ptg::firsts f(rs);
        f.calculate_all();

        auto& s_first = f.get_nterm_firsts(s_idx);
        REQUIRE(s_first.has_value());
        std::vector<size_t> s_got = s_first.value().get_indices();
        REQUIRE(s_got == std::vector<size_t>{a_idx, b_idx});
    }

    SECTION("invalid indices")
    {
        sc.add_nterm("S");
        sc.add_term("a");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"a"});

        ptg::firsts f(rs);

        REQUIRE_THROWS_MATCHES(
            f.get_nterm_firsts(rs.get_nterm_count()),
            std::out_of_range,
            Message("Nterm index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(rs.get_nterm_count(), 0, 0),
            std::out_of_range,
            Message("Nterm index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(0, rs.get_nterm_rside_count(0), 0),
            std::out_of_range,
            Message("Rside index out of range")
        );

        REQUIRE_THROWS_MATCHES(
            f.get_rside_part_firsts(0, 0, rs.get_symbol_count(0, 0)),
            std::out_of_range,
            Message("Symbol index out of range")
        );
    }

    SECTION("individual calculate_nterm - simple")
    {
        size_t a_idx = sc.add_term("a");

        size_t s_idx = sc.add_nterm("S");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"a"});

        ptg::firsts f(rs);

        auto& res = f.calculate_nterm(s_idx);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(a_idx));
    }

    SECTION("individual calculate_nterm - epsilon, empty first")
    {
        size_t s_idx = sc.add_nterm("S");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {});

        ptg::firsts f(rs);

        auto& res = f.calculate_nterm(s_idx);
        REQUIRE_FALSE(res.has_value());
    }

    SECTION("individual calculate_nterm - through nterm")
    {
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");

        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("Expr");

        ptg::ruleset rs(sc);
        rs.add_rule("S", {"Expr"});
        rs.add_rule("Expr", {"a"});
        rs.add_rule("Expr", {"b"});

        ptg::firsts f(rs);

        auto& res = f.calculate_nterm(s_idx);
        REQUIRE(res.has_value());
        std::vector<size_t> got = res.value().get_indices();
        REQUIRE(got == std::vector<size_t>{a_idx, b_idx});
    }

    SECTION("individual calculate_rside_part - terms")
    {
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");
        size_t c_idx = sc.add_term("c");

        size_t s_idx = sc.add_nterm("S");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"a", "b", "c"});

        ptg::firsts f(rs);

        auto& res0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE(res0.has_value());
        REQUIRE(res0.value().get_count() == 1);
        REQUIRE(res0.value().contains(a_idx));

        auto& res1 = f.calculate_rside_part(s_idx, s_r0, 1);
        REQUIRE(res1.has_value());
        REQUIRE(res1.value().get_count() == 1);
        REQUIRE(res1.value().contains(b_idx));

        auto& res2 = f.calculate_rside_part(s_idx, s_r0, 2);
        REQUIRE(res2.has_value());
        REQUIRE(res2.value().get_count() == 1);
        REQUIRE(res2.value().contains(c_idx));
    }

    SECTION("individual calculate_rside_part - with nullable")
    {
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");

        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("A");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"A", "b"});
        rs.add_rule("A", {"a"});
        rs.add_rule("A", {});

        ptg::firsts f(rs);

        auto& res0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE(res0.has_value());
        std::vector<size_t> got0 = res0.value().get_indices();
        REQUIRE(got0 == std::vector<size_t>{a_idx, b_idx});

        auto& res1 = f.calculate_rside_part(s_idx, s_r0, 1);
        REQUIRE(res1.has_value());
        REQUIRE(res1.value().get_count() == 1);
        REQUIRE(res1.value().contains(b_idx));
    }

    SECTION("left recursion")
    {
        sc.add_term("a");
        size_t b_idx = sc.add_term("b");

        size_t a_nt_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {"A", "a"});
        rs.add_rule("A", {"b"});

        ptg::firsts f(rs);

        auto& res = f.calculate_nterm(a_nt_idx);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(b_idx));
    }

    SECTION("left recursion with epsilon")
    {
        size_t a_idx = sc.add_term("a");

        size_t a_nt_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {"A", "a"});
        rs.add_rule("A", {});

        ptg::firsts f(rs);

        auto& res = f.calculate_nterm(a_nt_idx);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(a_idx));
    }

    SECTION("unresolved left recursion no terminal")
    {
        sc.add_term("a");

        size_t a_nt_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {"A", "a"});

        ptg::firsts f(rs);

        auto& res = f.calculate_nterm(a_nt_idx);
        REQUIRE_FALSE(res.has_value());
    }

    SECTION("individual calculate_rside_part - unresolved left recursion")
    {
        size_t a_idx = sc.add_term("a");

        size_t a_nt_idx = sc.add_nterm("A");

        ptg::ruleset rs(sc);
        size_t a_r0 = rs.add_rule("A", {"A", "a"});

        ptg::firsts f(rs);

        auto& res0 = f.calculate_rside_part(a_nt_idx, a_r0, 0);
        REQUIRE_FALSE(res0.has_value());

        auto& res1 = f.calculate_rside_part(a_nt_idx, a_r0, 1);
        REQUIRE(res1.has_value());
        REQUIRE(res1.value().get_count() == 1);
        REQUIRE(res1.value().contains(a_idx));
    }

    SECTION("calculate_nterm with unresolved cycle through chain no epsilon")
    {
        size_t a_nt_idx = sc.add_nterm("A");
        size_t b_nt_idx = sc.add_nterm("B");
        size_t c_nt_idx = sc.add_nterm("C");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {"B"});
        rs.add_rule("B", {"C"});
        rs.add_rule("C", {"A"});

        ptg::firsts f(rs);

        auto& res_a = f.calculate_nterm(a_nt_idx);
        REQUIRE_FALSE(res_a.has_value());

        auto& res_b = f.calculate_nterm(b_nt_idx);
        REQUIRE_FALSE(res_b.has_value());

        auto& res_c = f.calculate_nterm(c_nt_idx);
        REQUIRE_FALSE(res_c.has_value());
    }

    SECTION("calculate_nterm with unresolved cycle through chain with epsilon")
    {
        size_t a_nt_idx = sc.add_nterm("A");
        size_t b_nt_idx = sc.add_nterm("B");
        size_t c_nt_idx = sc.add_nterm("C");

        ptg::ruleset rs(sc);
        rs.add_rule("A", {"B"});
        rs.add_rule("B", {"C"});
        rs.add_rule("C", {"A"});
        rs.add_rule("C", {});

        ptg::firsts f(rs);

        auto& res_a = f.calculate_nterm(a_nt_idx);
        REQUIRE_FALSE(res_a.has_value());

        auto& res_b = f.calculate_nterm(b_nt_idx);
        REQUIRE_FALSE(res_b.has_value());

        auto& res_c = f.calculate_nterm(c_nt_idx);
        REQUIRE_FALSE(res_c.has_value());
    }

    SECTION("calculate_rside_part with unresolved cycle in prefix no epsilon")
    {
        size_t b_idx = sc.add_term("b");

        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("A");
        sc.add_nterm("C");
        sc.add_nterm("D");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"A", "b"});
        rs.add_rule("A", {"C"});
        rs.add_rule("C", {"D"});
        rs.add_rule("D", {"A"});

        ptg::firsts f(rs);

        auto& res0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE_FALSE(res0.has_value());

        auto& res1 = f.calculate_rside_part(s_idx, s_r0, 1);
        REQUIRE(res1.has_value());
        REQUIRE(res1.value().get_count() == 1);
        REQUIRE(res1.value().contains(b_idx));
    }

    SECTION("calculate_rside_part with unresolved cycle in prefix with epsilon")
    {
        size_t b_idx = sc.add_term("b");

        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("A");
        sc.add_nterm("C");
        sc.add_nterm("D");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"A", "b"});
        rs.add_rule("A", {"C"});
        rs.add_rule("C", {"D"});
        rs.add_rule("D", {"A"});
        rs.add_rule("D", {});

        ptg::firsts f(rs);

        auto& res0 = f.calculate_rside_part(s_idx, s_r0, 0);
        REQUIRE(res0.has_value());
        REQUIRE(res0.value().get_count() == 1);
        REQUIRE(res0.value().contains(b_idx));

        auto& res1 = f.calculate_rside_part(s_idx, s_r0, 1);
        REQUIRE(res1.has_value());
        REQUIRE(res1.value().get_count() == 1);
        REQUIRE(res1.value().contains(b_idx));
    }

    SECTION("mix individual and all")
    {
        size_t a_idx = sc.add_term("a");
        size_t b_idx = sc.add_term("b");
        size_t c_idx = sc.add_term("c");

        size_t s_idx = sc.add_nterm("S");
        sc.add_nterm("X");
        size_t y_idx = sc.add_nterm("Y");

        ptg::ruleset rs(sc);
        size_t s_r0 = rs.add_rule("S", {"X", "b"});
        rs.add_rule("X", {"Y"});
        rs.add_rule("Y", {"a"});
        rs.add_rule("Y", {"c"});

        ptg::firsts f(rs);

        auto& res_y = f.calculate_nterm(y_idx);
        REQUIRE(res_y.has_value());
        std::vector<size_t> got_y = res_y.value().get_indices();
        REQUIRE(got_y == std::vector<size_t>{a_idx, c_idx});

        auto& res_part = f.calculate_rside_part(s_idx, s_r0, 1);
        REQUIRE(res_part.has_value());
        REQUIRE(res_part.value().get_count() == 1);
        REQUIRE(res_part.value().contains(b_idx));

        f.calculate_all();

        auto& res_s = f.get_nterm_firsts(s_idx);
        REQUIRE(res_s.has_value());
        std::vector<size_t> got_s = res_s.value().get_indices();
        REQUIRE(got_s == std::vector<size_t>{a_idx, c_idx});
    }
}
