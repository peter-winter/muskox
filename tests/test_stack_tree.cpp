#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <stack_tree.h>

using Catch::Matchers::Message;

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

TEST_CASE("stack tree class basics", "[stack]")
{
    using namespace muskox;

    SECTION("root node creation")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        REQUIRE(root->is_leaf());
        REQUIRE(root->empty());
        REQUIRE(root->data_size() == 0);
        REQUIRE(root->get_parent() == nullptr);
        REQUIRE(root->get_prefix_length() == 0);
        REQUIRE_THROWS_MATCHES(root->get_leaf_element(), std::runtime_error, Message("cannot get leaf element from empty node"));
        REQUIRE_THROWS_MATCHES(root->pop(), std::runtime_error, Message("cannot pop from empty node"));
    }

    SECTION("push_back and basic iterator")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it1 = root->push_back(42);
        REQUIRE(!root->empty());
        REQUIRE(root->data_size() == 1);
        REQUIRE(root->is_leaf());
        REQUIRE(it1.is_valid());
        REQUIRE(it1.is_leaf_node());
        REQUIRE(it1.is_leaf_element());
        REQUIRE(*it1 == 42);
        REQUIRE(it1.get_offset() == 0);
        REQUIRE(it1.get_current() == root);

        auto it2 = root->push_back(100);
        REQUIRE(root->data_size() == 2);
        REQUIRE(*it2 == 100);
        REQUIRE(it2.get_offset() == 1);

        auto back1 = st.traverse_back(it2, 1);
        REQUIRE(back1.is_valid());
        REQUIRE(*back1 == 42);
        REQUIRE(back1.get_offset() == 0);

        auto invalid = st.traverse_back(back1, 1);
        REQUIRE(!invalid.is_valid());
        REQUIRE_THROWS_MATCHES(*invalid, std::runtime_error, Message("invalid iterator"));
        REQUIRE_THROWS_MATCHES(--invalid, std::runtime_error, Message("cannot decrement invalid iterator"));
    }

    SECTION("push_back and basic const_iterator")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it1 = root->push_back(42);
        REQUIRE(!root->empty());
        REQUIRE(root->data_size() == 1);
        REQUIRE(root->is_leaf());

        auto cit1 = root->get_leaf_element();
        REQUIRE(cit1.is_valid());
        REQUIRE(cit1.is_leaf_node());
        REQUIRE(cit1.is_leaf_element());
        REQUIRE(*cit1 == 42);
        REQUIRE(cit1.get_offset() == 0);
        REQUIRE(cit1.get_current() == root);

        // Test conversion from iterator to const_iterator
        const_iterator<int> cit_from_it(it1);
        REQUIRE(cit_from_it == cit1);
        REQUIRE(*cit_from_it == 42);

        auto it2 = root->push_back(100);
        REQUIRE(root->data_size() == 2);
        REQUIRE(*it2 == 100);

        auto cit2 = root->get_leaf_element();
        REQUIRE(cit2.get_offset() == 1);

        auto back1 = st.traverse_back(cit2, 1);
        REQUIRE(back1.is_valid());
        REQUIRE(*back1 == 42);
        REQUIRE(back1.get_offset() == 0);

        auto invalid = st.traverse_back(back1, 1);
        REQUIRE(!invalid.is_valid());
        REQUIRE_THROWS_MATCHES(*invalid, std::runtime_error, Message("invalid iterator"));
        REQUIRE_THROWS_MATCHES(--invalid, std::runtime_error, Message("cannot decrement invalid iterator"));

        // Ensure *cit is const (compile-time check implied; runtime can't assign)
    }

    SECTION("branching")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it = root->push_back(1);
        it = root->push_back(2);

        REQUIRE(root->is_leaf());

        auto branch_it = st.branch(it, 3);
        auto child = branch_it.get_current();
        REQUIRE(child->get_parent() == root);
        REQUIRE(child->get_prefix_length() == 2);
        REQUIRE(child->data_size() == 1);
        REQUIRE(*branch_it == 3);
        REQUIRE(branch_it.get_offset() == 0);

        auto back1 = st.traverse_back(branch_it, 1);
        REQUIRE(*back1 == 2);
        REQUIRE(back1.get_current() == root);
        REQUIRE(back1.get_offset() == 1);

        auto back2 = st.traverse_back(back1, 1);
        REQUIRE(*back2 == 1);
        REQUIRE(back2.get_offset() == 0);

        auto invalid = st.traverse_back(back2, 1);
        REQUIRE(!invalid.is_valid());

        REQUIRE(!root->is_leaf());
    }

    SECTION("pop and prune")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it = root->push_back(1);
        it = root->push_back(2);
        auto branch_it = st.branch(it, 3);
        auto child = branch_it.get_current();
        REQUIRE(!root->is_leaf());

        auto pop_it = child->pop();
        REQUIRE(pop_it.is_valid());
        REQUIRE(*pop_it == 2);
        REQUIRE(pop_it.get_current() == root);
        REQUIRE(root->is_leaf());
    }

    SECTION("reduce")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it = root->push_back(1);
        it = root->push_back(2);
        it = root->push_back(3);

        auto red0 = st.reduce(it, 0);
        REQUIRE(*red0 == 3);

        auto red1 = st.reduce(it, 1);
        REQUIRE(red1.is_valid());
        REQUIRE(*red1 == 2);
        REQUIRE(root->data_size() == 2);

        auto red2 = st.reduce(red1, 1);
        REQUIRE(red2.is_valid());
        REQUIRE(*red2 == 1);
        REQUIRE(root->data_size() == 1);

        auto red3 = st.reduce(red2, 1);
        REQUIRE(!red3.is_valid());
        REQUIRE(root->empty());
    }

    SECTION("reduce with callable")
    {
        struct call_counter
        {
            int const_calls_ = 0;
            int non_const_calls_ = 0;
        } cc;
        
        overloaded f
        {
            [&cc](int&){ ++cc.non_const_calls_; },
            [&cc](const int&){ ++cc.const_calls_; }
        };

        // Linear case: all pops, all non-const
        stack_tree<int> st_linear;
        auto root_linear = st_linear.get_root();
        auto it_linear = root_linear->push_back(1);
        it_linear = root_linear->push_back(2);
        it_linear = root_linear->push_back(3);

        auto red_linear = st_linear.reduce(it_linear, 3, f);
        REQUIRE(!red_linear.is_valid());
        REQUIRE(root_linear->empty());
        REQUIRE(cc.const_calls_ == 0);
        REQUIRE(cc.non_const_calls_ == 3);
    
        cc = {};
        
        // Mixed case: traverses only
        stack_tree<int> st_mixed;
        auto root_mixed = st_mixed.get_root();
        auto it_mixed = root_mixed->push_back(10);
        it_mixed = root_mixed->push_back(20);
        it_mixed = root_mixed->push_back(30);

        auto mid = st_mixed.traverse_back(it_mixed, 1);  // at 20, not end
        REQUIRE(*mid == 20);
        REQUIRE(!mid.is_leaf_element());

        auto red_mixed = st_mixed.reduce(mid, 2, f);
        REQUIRE(!red_mixed.is_valid());
        REQUIRE(root_mixed->data_size() == 3);
        REQUIRE(cc.const_calls_ == 2);
        REQUIRE(cc.non_const_calls_ == 0);

        cc = {};
        
        // Now reduce from end with pop
        auto red_end = st_mixed.reduce(it_mixed, 1, f);
        REQUIRE(red_end.is_valid());
        REQUIRE(*red_end == 20);
        REQUIRE(root_mixed->data_size() == 2);
        REQUIRE(cc.const_calls_ == 0);
        REQUIRE(cc.non_const_calls_ == 1);

        // Default no-op
        auto red_default = st_mixed.reduce(red_end, 1);
        REQUIRE(red_default.is_valid());
        REQUIRE(*red_default == 10);
        REQUIRE(root_mixed->data_size() == 1);
    }

    SECTION("reduce with branch")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it = root->push_back(1);
        it = root->push_back(2);
        auto branch_it = st.branch(it, 3);
        branch_it = branch_it.get_current()->push_back(4);

        auto red1 = st.reduce(branch_it, 1);
        REQUIRE(*red1 == 3);
        REQUIRE(red1.get_current()->data_size() == 1);

        auto red2 = st.reduce(red1, 1);
        REQUIRE(*red2 == 2);
        REQUIRE(root->is_leaf());

        auto red3 = st.reduce(red2, 1);
        REQUIRE(*red3 == 1);
    }

    SECTION("traverse_back no change")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it = root->push_back(1);
        auto trav = st.traverse_back(it, 0);
        REQUIRE(*trav == 1);
    }

    SECTION("exceptions")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it = root->push_back(1);
        auto branch_it = st.branch(it, 2);
        REQUIRE_THROWS_MATCHES(root->push_back(3), std::runtime_error, Message("can only push to leaf nodes"));

        REQUIRE_THROWS_MATCHES(st.branch(iterator<int>(), 0), std::runtime_error, Message("invalid iterator for branching"));

        REQUIRE_THROWS_MATCHES(st.reduce(iterator<int>(), 1), std::runtime_error, Message("invalid iterator for reduce"));

        REQUIRE_THROWS_MATCHES(st.traverse_back(iterator<int>(), 1), std::runtime_error, Message("invalid iterator for traverse_back"));

        auto empty_it = iterator<int>();
        REQUIRE(!empty_it.is_leaf_node());
        REQUIRE(!empty_it.is_leaf_element());
    }

    SECTION("branch from offset 0")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it0 = root->push_back(0);
        auto back = st.traverse_back(it0, 1);
        REQUIRE(!back.is_valid());

        auto it_first = root->get_leaf_element();
        auto branch_first = st.branch(it_first, 100);
        REQUIRE(branch_first.get_current()->get_prefix_length() == 1);
        auto back_branch = st.traverse_back(branch_first, 1);
        REQUIRE(*back_branch == 0);
    }

    SECTION("prefix length 0")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        REQUIRE(root->get_prefix_length() == 0);

        auto it = root->push_back(10);
        REQUIRE(it.get_offset() == 0);
        REQUIRE(*it == 10);

        auto back = st.traverse_back(it, 1);
        REQUIRE(!back.is_valid());

        iterator<int> root_it(root, 0);
        REQUIRE(root_it.is_valid());
        REQUIRE(*root_it == 10);

        auto old_it = root_it;
        auto post_dec = root_it--;
        REQUIRE(post_dec == old_it);
        REQUIRE(!root_it.is_valid());

        auto pre_dec = --old_it;
        REQUIRE(!pre_dec.is_valid());
        REQUIRE(pre_dec == old_it);
    }

    SECTION("iterator decrement operators")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it1 = root->push_back(1);
        auto it2 = root->push_back(2);
        auto it3 = root->push_back(3);

        auto pre = --it3;
        REQUIRE(pre == it3);
        REQUIRE(*it3 == 2);
        REQUIRE(it3.get_offset() == 1);

        auto old_it = it3;
        auto post = it3--;
        REQUIRE(post == old_it);
        REQUIRE(*post == 2);
        REQUIRE(*it3 == 1);
        REQUIRE(it3.get_offset() == 0);

        auto pre_inv = --it3;
        REQUIRE(!pre_inv.is_valid());
        REQUIRE(pre_inv == it3);

        REQUIRE_THROWS_MATCHES(--it3, std::runtime_error, Message("cannot decrement invalid iterator"));

        auto it_near = iterator<int>(root, 0);
        REQUIRE(*it_near == 1);

        auto old_near = it_near;
        auto post_near = it_near--;
        REQUIRE(*post_near == 1);
        REQUIRE(post_near.get_offset() == 0);
        REQUIRE(!it_near.is_valid());

        auto it_at_0 = iterator<int>(root, 0);
        REQUIRE(*it_at_0 == 1);

        auto branch_at_0 = st.branch(it_at_0, 4);
        REQUIRE(branch_at_0.get_current()->get_prefix_length() == 1);

        auto br_back = branch_at_0;
        auto pre_br = --br_back;
        REQUIRE(*pre_br == 1);
        REQUIRE(pre_br.get_current() == root);
        REQUIRE(pre_br.get_offset() == 0);

        auto post_br = br_back--;
        REQUIRE(*post_br == 1);
        REQUIRE(!br_back.is_valid());
    }

    SECTION("chain pruning with reduce")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        auto it1 = root->push_back(1);
        REQUIRE(*it1 == 1);

        std::weak_ptr<tree_node<int>> wp1;
        std::weak_ptr<tree_node<int>> wp2;
        std::weak_ptr<tree_node<int>> wp3;

        {
            auto it2 = st.branch(it1, 2);
            auto level1 = it2.get_current();
            wp1 = level1;
            REQUIRE(*it2 == 2);
            REQUIRE(level1->get_prefix_length() == 1);

            auto it3 = st.branch(it2, 3);
            auto level2 = it3.get_current();
            wp2 = level2;
            REQUIRE(*it3 == 3);
            REQUIRE(level2->get_prefix_length() == 1);

            auto it4 = st.branch(it3, 4);
            auto level3 = it4.get_current();
            wp3 = level3;
            REQUIRE(*it4 == 4);
            REQUIRE(level3->get_prefix_length() == 1);

            auto back1 = st.traverse_back(it4, 1);
            REQUIRE(back1.is_valid());
            REQUIRE(*back1 == 3);
            REQUIRE(back1.get_current() == level2);

            auto back2 = st.traverse_back(back1, 1);
            REQUIRE(*back2 == 2);
            REQUIRE(back2.get_current() == level1);

            auto back3 = st.traverse_back(back2, 1);
            REQUIRE(*back3 == 1);
            REQUIRE(back3.get_current() == root);

            auto back4 = st.traverse_back(back3, 1);
            REQUIRE(!back4.is_valid());

            auto reduced = st.reduce(it4, 4);
            REQUIRE(!reduced.is_valid());
        }

        REQUIRE(wp3.expired());
        REQUIRE(wp2.expired());
        REQUIRE(wp1.expired());

        REQUIRE(root->empty());
        REQUIRE(root->is_leaf());
        REQUIRE(root->data_size() == 0);
    }

    SECTION("at() access")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        REQUIRE_THROWS_MATCHES(root->at(0), std::out_of_range, Message("index out of bounds"));
        REQUIRE_THROWS_MATCHES(root->at(1), std::out_of_range, Message("index out of bounds"));

        const auto const_root = root;
        REQUIRE_THROWS_MATCHES(const_root->at(0), std::out_of_range, Message("index out of bounds"));

        auto it = root->push_back(10);
        REQUIRE(root->at(0) == 10);
        REQUIRE_THROWS_MATCHES(root->at(1), std::out_of_range, Message("index out of bounds"));

        root->at(0) = 20;
        REQUIRE(root->at(0) == 20);
        REQUIRE(*it == 20);  // Since it points to the same element

        const auto const_root_filled = root;
        REQUIRE(const_root_filled->at(0) == 20);
        REQUIRE_THROWS_MATCHES(const_root_filled->at(1), std::out_of_range, Message("index out of bounds"));

        root->push_back(30);
        root->push_back(40);
        REQUIRE(root->at(1) == 30);
        REQUIRE(root->at(2) == 40);
        REQUIRE_THROWS_MATCHES(root->at(3), std::out_of_range, Message("index out of bounds"));
    }

    SECTION("iterator equality")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        iterator<int> invalid1;
        iterator<int> invalid2;
        REQUIRE(invalid1 == invalid2);

        auto it1 = root->push_back(1);
        auto it2 = root->push_back(2);

        REQUIRE(it1 == it1);
        REQUIRE(it2 == it2);
        REQUIRE(!(it1 == it2));

        auto it1_copy = it1;
        REQUIRE(it1_copy == it1);
        REQUIRE(!(it1_copy == it2));

        auto back = st.traverse_back(it2, 1);
        REQUIRE(back == it1);
        REQUIRE(!(back == it2));

        REQUIRE(!(invalid1 == it1));

        auto branch_it = st.branch(it2, 3);
        REQUIRE(!(branch_it == it2));

        auto branch_back = st.traverse_back(branch_it, 1);
        REQUIRE(branch_back == it2);
        REQUIRE(!(branch_back == branch_it));

        // Different nodes but same logical position? (not possible since positions are node+offset)
        // But test different nodes with same offset
        stack_tree<int> another_st;
        auto another_root = another_st.get_root();
        auto another_it = another_root->push_back(1);
        REQUIRE(!(another_it == it1));  // Different nodes
    }

    SECTION("const_iterator equality")
    {
        stack_tree<int> st;
        auto root = st.get_root();
        const_iterator<int> invalid1;
        const_iterator<int> invalid2;
        REQUIRE(invalid1 == invalid2);

        auto it1 = root->push_back(1);
        auto cit1 = root->get_leaf_element();
        auto it2 = root->push_back(2);
        auto cit2 = root->get_leaf_element();

        REQUIRE(cit1 == cit1);
        REQUIRE(cit2 == cit2);
        REQUIRE(!(cit1 == cit2));

        auto cit1_copy = cit1;
        REQUIRE(cit1_copy == cit1);
        REQUIRE(!(cit1_copy == cit2));

        auto back = st.traverse_back(cit2, 1);
        REQUIRE(back == cit1);
        REQUIRE(!(back == cit2));

        REQUIRE(!(invalid1 == cit1));

        auto branch_it = st.branch(it2, 3);
        auto branch_cit = branch_it.get_current()->get_leaf_element();
        REQUIRE(!(branch_cit == cit2));

        auto branch_back = st.traverse_back(branch_cit, 1);
        REQUIRE(branch_back == cit2);
        REQUIRE(!(branch_back == branch_cit));
    }
}
