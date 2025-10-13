#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <closure.h>
#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>
#include <lr1_set_item.h>

#include <algorithm>

using Catch::Matchers::Message;


TEST_CASE("closure computation", "[closure]")
{
    ptg::symbol_collection sc;

    [[maybe_unused]] size_t eof_idx = sc.get_symbol_ref("$eof").index_;

    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t plus_idx = sc.add_term("+");

    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t term_idx = sc.add_nterm("Term");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"Expr", "+", "Term"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"Term"});
    [[maybe_unused]] size_t term_r0 = rs.add_rule("Term", {"a"});
    [[maybe_unused]] size_t term_r1 = rs.add_rule("Term", {"b"});
    [[maybe_unused]] size_t term_r2 = rs.add_rule("Term", {});

    [[maybe_unused]] size_t root_idx = 0;

    SECTION("item at end")
    {
        ptg::lr1_set_item item(term_idx, term_r0, 1, b_idx);  // Term -> a . / b
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 1);
        REQUIRE(got == std::vector<std::array<size_t, 4>>{{term_idx, term_r0, 1, b_idx}});
    }

    SECTION("dot before terminal")
    {
        ptg::lr1_set_item item(term_idx, term_r0, 0, a_idx);  // Term -> . a / a
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 1);
        REQUIRE(got == std::vector<std::array<size_t, 4>>{{term_idx, term_r0, 0, a_idx}});
    }

    SECTION("dot before nterm, empty suffix")
    {
        ptg::lr1_set_item item(s_idx, s_r0, 0, eof_idx);  // S -> . Expr / $eof
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 3);  // original + 2 prods for Expr
        REQUIRE(got == std::vector<std::array<size_t, 4>>{
            {s_idx, s_r0, 0, eof_idx},
            {expr_idx, expr_r0, 0, eof_idx},
            {expr_idx, expr_r1, 0, eof_idx}
        });
    }

    SECTION("dot before nterm, non-nullable suffix")
    {
        ptg::lr1_set_item item(expr_idx, expr_r0, 0, b_idx);  // Expr -> . Expr + Term / b
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 3);
        REQUIRE(got == std::vector<std::array<size_t, 4>>{
            {expr_idx, expr_r0, 0, b_idx},
            {expr_idx, expr_r0, 0, plus_idx},
            {expr_idx, expr_r1, 0, plus_idx}
        });
    }

    SECTION("dot before nterm, nullable suffix")
    {
        ptg::lr1_set_item item(expr_idx, expr_r0, 2, a_idx);  // Expr -> Expr + . Term / a
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 4);
        REQUIRE(got == std::vector<std::array<size_t, 4>>{
            {expr_idx, expr_r0, 2, a_idx},
            {term_idx, term_r0, 0, a_idx},
            {term_idx, term_r1, 0, a_idx},
            {term_idx, term_r2, 0, a_idx},
        });
    }

    SECTION("epsilon production item")
    {
        ptg::lr1_set_item item(term_idx, term_r2, 0, plus_idx);  // Term -> . / +
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 1);
        REQUIRE(got == std::vector<std::array<size_t, 4>>{{term_idx, term_r2, 0, plus_idx}});
    }

    SECTION("root item")
    {
        ptg::lr1_set_item item(root_idx, 0, 0, eof_idx);  // $root -> . S / $eof
        ptg::closure cl(rs);

        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        auto got = res.value().get_indices();
        REQUIRE(got.size() == 2);  
        REQUIRE(got == std::vector<std::array<size_t, 4>>{
            {root_idx, 0, 0, eof_idx},
            {s_idx, s_r0, 0, eof_idx}
        });
    }
}
