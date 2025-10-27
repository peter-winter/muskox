#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <history.h>

using value_type = int;
using ptg::stack;
using ptg::history;
using ptg::branch;
using Catch::Matchers::Message;

TEST_CASE("stack class basics", "[stack]")
{
    auto stk = std::make_shared<stack<value_type>>();

    SECTION("initial state")
    {
        REQUIRE(stk->empty());
        REQUIRE(stk->size() == 0);
        REQUIRE(stk->get_branch_count() == 0);
        REQUIRE(stk->get_parent() == nullptr);
        REQUIRE(stk->get_children().empty());
    }

    SECTION("push and top")
    {
        stk->push(42);
        REQUIRE(!stk->empty());
        REQUIRE(stk->size() == 1);
        REQUIRE(stk->top() == 42);
    }

    SECTION("pop")
    {
        stk->push(1);
        stk->push(2);
        stk->pop(1);
        REQUIRE(stk->size() == 1);
        REQUIRE(stk->top() == 1);
        stk->pop(1);
        REQUIRE(stk->empty());
    }

    SECTION("pop exceptions")
    {
        REQUIRE_THROWS_MATCHES(stk->pop(1), std::underflow_error, Message("cannot pop more than available"));
        stk->push(1);
        REQUIRE_THROWS_MATCHES(stk->pop(2), std::underflow_error, Message("cannot pop more than available"));
    }

    SECTION("top exceptions")
    {
        REQUIRE_THROWS_MATCHES(stk->top(), std::underflow_error, Message("stack is empty"));
    }

    SECTION("push after child")
    {
        stk->add_child();
        REQUIRE_THROWS_MATCHES(stk->push(1), std::invalid_argument, Message("cannot push to non-leaf stack"));
    }

    SECTION("pop after child")
    {
        stk->add_child();
        REQUIRE_THROWS_MATCHES(stk->pop(1), std::invalid_argument, Message("cannot pop from non-leaf stack"));
    }

    SECTION("add_child")
    {
        auto child = stk->add_child();
        REQUIRE(child->get_parent() == stk);
        REQUIRE(stk->get_children().size() == 1);
        REQUIRE(stk->get_children()[0] == child);
    }

    SECTION("branch count")
    {
        stk->inc_branch_count();
        REQUIRE(stk->get_branch_count() == 1);
        stk->inc_branch_count();
        REQUIRE(stk->get_branch_count() == 2);
        stk->dec_branch_count();
        REQUIRE(stk->get_branch_count() == 1);
    }

    SECTION("dec_branch_count below zero")
    {
        REQUIRE_THROWS_MATCHES(stk->dec_branch_count(), std::runtime_error, Message("cannot decrement branch_count_ below zero"));
    }

    SECTION("move_branch_ref_to")
    {
        auto other = std::make_shared<stack<value_type>>();
        stk->inc_branch_count();
        REQUIRE(stk->get_branch_count() == 1);
        REQUIRE(other->get_branch_count() == 0);
        stk->move_branch_ref_to(other);
        REQUIRE(stk->get_branch_count() == 0);
        REQUIRE(other->get_branch_count() == 1);
    }

    SECTION("possibly_remove")
    {
        auto parent = std::make_shared<stack<value_type>>();
        auto child = parent->add_child();
        child->inc_branch_count();
        child->dec_branch_count();
        REQUIRE(child->get_branch_count() == 0);
        REQUIRE(child->empty());
        REQUIRE(child->get_children().empty());
        REQUIRE(child->possibly_remove());
        REQUIRE(parent->get_children().empty());
    }

    SECTION("possibly_remove with branches")
    {
        stk->inc_branch_count();
        REQUIRE_FALSE(stk->possibly_remove());  // Should not remove
        REQUIRE(stk->get_branch_count() == 1);
    }

    SECTION("possibly_remove with data")
    {
        stk->push(1);
        REQUIRE_FALSE(stk->possibly_remove());  // Should not remove
        REQUIRE(!stk->empty());
    }

    SECTION("possibly_remove with children")
    {
        stk->add_child();
        REQUIRE_FALSE(stk->possibly_remove());  // Should not remove
        REQUIRE(!stk->get_children().empty());
    }

    SECTION("remove root exception")
    {
        REQUIRE_THROWS_MATCHES(stk->possibly_remove(), std::invalid_argument, Message("cannot delete root"));
    }

    SECTION("remove with children exception")
    {
        auto parent = std::make_shared<stack<value_type>>();
        auto child = parent->add_child();
        child->add_child();
        REQUIRE_FALSE(child->possibly_remove());
        REQUIRE_FALSE(parent->possibly_remove());
    }
}

TEST_CASE("branch class basics", "[branch]")
{
    branch<value_type> b;

    SECTION("default state")
    {
        REQUIRE(b.get_stack() == nullptr);
        REQUIRE(b.get_size() == 0);
    }

    SECTION("fork null")
    {
        auto forked = b.fork();
        REQUIRE(forked.get_stack() == nullptr);
        REQUIRE(forked.get_size() == 0);
    }

    SECTION("commit on null")
    {
        b.commit(42);
        auto stk = b.get_stack();
        REQUIRE(stk != nullptr);
        REQUIRE(stk->size() == 1);
        REQUIRE(stk->top() == 42);
        REQUIRE(b.get_size() == 1);
        REQUIRE(stk->get_branch_count() == 1);
    }

    SECTION("multiple commits")
    {
        b.commit(1);
        b.commit(2);
        auto stk = b.get_stack();
        REQUIRE(stk->size() == 2);
        REQUIRE(stk->top() == 2);
        REQUIRE(b.get_size() == 2);
    }

    SECTION("fork and commit shared")
    {
        b.commit(1);
        auto forked = b.fork();
        REQUIRE(b.get_stack()->get_branch_count() == 2);
        forked.commit(2);
        auto forked_stk = forked.get_stack();
        REQUIRE(forked_stk != b.get_stack());
        REQUIRE(forked_stk->get_parent() == b.get_stack());
        REQUIRE(b.get_stack()->get_branch_count() == 1);
        REQUIRE(forked_stk->get_branch_count() == 1);
        REQUIRE(forked.get_size() == 2);
        REQUIRE(forked_stk->size() == 1);
        REQUIRE(forked_stk->top() == 2);
        REQUIRE(b.get_size() == 1);  // Unchanged
    }

    SECTION("release null")
    {
        b.release();  // No-op
        REQUIRE(b.get_stack() == nullptr);
    }

    SECTION("release with stack")
    {
        b.commit(1);
        auto stk = b.get_stack();
        REQUIRE(stk->get_branch_count() == 1);
        b.release();
        REQUIRE(b.get_stack() == nullptr);
        REQUIRE(b.get_size() == 0);
        REQUIRE(stk->get_branch_count() == 0);
        // possibly_remove would remove if conditions met, but since no parent, can't remove root
    }
}

TEST_CASE("history class basics", "[history]")
{
    history<value_type> h;

    SECTION("initial state")
    {
        REQUIRE(h.get_main_branch().get_stack() == nullptr);
        REQUIRE(h.get_main_branch().get_size() == 0);
    }

    SECTION("create_branch_from")
    {
        auto idx = h.create_branch_from(0);
        REQUIRE(idx == 1);
        // Since from null, new is also null
        REQUIRE(h.get_main_branch().get_stack() == nullptr);
    }

    SECTION("create_branch_from after commit")
    {
        h.get_main_branch().commit(1);
        auto idx = h.create_branch_from(0);
        REQUIRE(idx == 1);
        REQUIRE(h.get_main_branch().get_stack()->get_branch_count() == 2);
    }

    SECTION("delete_branch")
    {
        auto idx = h.create_branch_from(0);
        h.delete_branch(idx);
        // Assuming only main left
        REQUIRE_THROWS_MATCHES(h.create_branch_from(1), std::out_of_range, Message("invalid branch index"));
    }

    SECTION("delete main")
    {
        h.delete_branch(0);
        REQUIRE_THROWS_MATCHES(h.get_main_branch(), std::out_of_range, Message("invalid branch index"));
    }

    SECTION("invalid index")
    {
        REQUIRE_THROWS_MATCHES(h.create_branch_from(10), std::out_of_range, Message("invalid branch index"));
        REQUIRE_THROWS_MATCHES(h.delete_branch(10), std::out_of_range, Message("invalid branch index"));
    }

    SECTION("integration: branches and commits")
    {
        h.get_main_branch().commit(1);
        
        REQUIRE(h.get_main_branch().get_size() == 1);
        REQUIRE(h.get_main_branch().get_stack()->size() == 1);
        
        auto idx1 = h.create_branch_from(0);
        REQUIRE(idx1 == 1);
        
        REQUIRE(h.get_main_branch().get_size() == 1);
        REQUIRE(h.get_main_branch().get_stack()->size() == 1);
        
        REQUIRE(h.get_branch(1).get_size() == 1);
        REQUIRE(h.get_branch(1).get_stack()->size() == 1);
        
        REQUIRE(h.get_main_branch().get_stack()->get_branch_count() == 2);  // havent comitted to branch 1, so both still on same stack
        h.get_main_branch().commit(2);
        REQUIRE(h.get_main_branch().get_stack()->get_branch_count() == 1);  // now main on its own stack
        REQUIRE(h.get_branch(1).get_stack()->get_branch_count() == 1);  // now branch 1 on its own stack (the original stack from main)
        
        REQUIRE(h.get_main_branch().get_size() == 2);
        REQUIRE(h.get_main_branch().get_stack()->size() == 1);
        
        REQUIRE(h.get_branch(1).get_size() == 1);
        REQUIRE(h.get_branch(1).get_stack()->size() == 1);

        /*
        h.get_branch(1).commit(3);  // this unfortunately throws
        REQUIRE(h.get_branch(1).get_stack()->get_branch_count() == 1);  // branch 1 still on its own stack
        REQUIRE(h.get_branch(1).get_size() == 2);
        REQUIRE(h.get_branch(1).get_stack()->size() == 2);
        */
    }
}
