// test_parse_result.cpp
#include <catch2/catch_test_macros.hpp>

#include <parse_result.h>

#include <string>
#include <vector>

TEST_CASE("parse_result basics", "[parse_result]")
{
    SECTION("ok")
    {
        auto res = ptg::parse_result::ok();
        REQUIRE(res.is_success());
        REQUIRE(res.get_errors().empty());
        REQUIRE(res.errors_to_string().empty());
    }

    SECTION("error empty")
    {
        std::vector<std::string> errs{};
        auto res = ptg::parse_result::error(std::move(errs));
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().empty());
        REQUIRE(res.errors_to_string().empty());
    }

    SECTION("error single")
    {
        std::vector<std::string> errs{"error1"};
        auto res = ptg::parse_result::error(std::move(errs));
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "error1");
        REQUIRE(res.errors_to_string() == "error1");
    }

    SECTION("error multiple")
    {
        std::vector<std::string> errs{"error1", "error2"};
        auto res = ptg::parse_result::error(std::move(errs));
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 2);
        REQUIRE(res.get_errors()[0] == "error1");
        REQUIRE(res.get_errors()[1] == "error2");
        REQUIRE(res.errors_to_string() == "error1\nerror2");
    }
}
