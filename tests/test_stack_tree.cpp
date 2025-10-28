#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <stack_tree.h>

using Catch::Matchers::Message;

TEST_CASE("stack tree class basics", "[stack]")
{
    using namespace ptg;

    SECTION("root node creation")
    {
        auto root = tree_node<int>::create_root();
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
        auto root = tree_node<int>::create_root();
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

        auto back1 = traverse_back(it2, 1);
        REQUIRE(back1.is_valid());
        REQUIRE(*back1 == 42);
        REQUIRE(back1.get_offset() == 0);

        auto invalid = traverse_back(back1, 1);
        REQUIRE(!invalid.is_valid());
        REQUIRE_THROWS_MATCHES(*invalid, std::runtime_error, Message("invalid iterator"));
        REQUIRE_THROWS_MATCHES(--invalid, std::runtime_error, Message("cannot decrement invalid iterator"));
    }

    SECTION("branching")
    {
        auto root = tree_node<int>::create_root();
        auto it = root->push_back(1);
        it = root->push_back(2);

        REQUIRE(root->is_leaf());

        auto branch_it = branch(it, 3);
        auto child = branch_it.get_current();
        REQUIRE(child->get_parent() == root);
        REQUIRE(child->get_prefix_length() == 2);
        REQUIRE(child->data_size() == 1);
        REQUIRE(*branch_it == 3);
        REQUIRE(branch_it.get_offset() == 0);

        auto back1 = traverse_back(branch_it, 1);
        REQUIRE(*back1 == 2);
        REQUIRE(back1.get_current() == root);
        REQUIRE(back1.get_offset() == 1);

        auto back2 = traverse_back(back1, 1);
        REQUIRE(*back2 == 1);
        REQUIRE(back2.get_offset() == 0);

        auto invalid = traverse_back(back2, 1);
        REQUIRE(!invalid.is_valid());

        REQUIRE(!root->is_leaf());
    }

    SECTION("pop and prune")
    {
        auto root = tree_node<int>::create_root();
        auto it = root->push_back(1);
        it = root->push_back(2);
        auto branch_it = branch(it, 3);
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
        auto root = tree_node<int>::create_root();
        auto it = root->push_back(1);
        it = root->push_back(2);
        it = root->push_back(3);

        auto red0 = reduce(it, 0);
        REQUIRE(*red0 == 3);

        auto red1 = reduce(it, 1);
        REQUIRE(red1.is_valid());
        REQUIRE(*red1 == 2);
        REQUIRE(root->data_size() == 2);

        auto red2 = reduce(red1, 1);
        REQUIRE(red2.is_valid());
        REQUIRE(*red2 == 1);
        REQUIRE(root->data_size() == 1);

        auto red3 = reduce(red2, 1);
        REQUIRE(!red3.is_valid());
        REQUIRE(root->empty());
    }

    SECTION("reduce with branch")
    {
        auto root = tree_node<int>::create_root();
        auto it = root->push_back(1);
        it = root->push_back(2);
        auto branch_it = branch(it, 3);
        branch_it = branch_it.get_current()->push_back(4);

        auto red1 = reduce(branch_it, 1);
        REQUIRE(*red1 == 3);
        REQUIRE(red1.get_current()->data_size() == 1);

        auto red2 = reduce(red1, 1);
        REQUIRE(*red2 == 2);
        REQUIRE(root->is_leaf());

        auto red3 = reduce(red2, 1);
        REQUIRE(*red3 == 1);
    }

    SECTION("traverse_back no change")
    {
        auto root = tree_node<int>::create_root();
        auto it = root->push_back(1);
        auto trav = traverse_back(it, 0);
        REQUIRE(*trav == 1);
    }

    SECTION("exceptions")
    {
        auto root = tree_node<int>::create_root();
        auto it = root->push_back(1);
        auto branch_it = branch(it, 2);
        REQUIRE_THROWS_MATCHES(root->push_back(3), std::runtime_error, Message("can only push to leaf nodes"));

        REQUIRE_THROWS_MATCHES(branch(iterator<int>(), 0), std::runtime_error, Message("invalid iterator for branching"));

        REQUIRE_THROWS_MATCHES(reduce(iterator<int>(), 1), std::runtime_error, Message("invalid iterator for reduce"));

        REQUIRE_THROWS_MATCHES(traverse_back(iterator<int>(), 1), std::runtime_error, Message("invalid iterator for traverse_back"));

        auto empty_it = iterator<int>();
        REQUIRE(!empty_it.is_leaf_node());
        REQUIRE(!empty_it.is_leaf_element());
    }

    SECTION("branch from offset 0")
    {
        auto root = tree_node<int>::create_root();
        auto it0 = root->push_back(0);
        auto back = traverse_back(it0, 1);
        REQUIRE(!back.is_valid());

        auto it_first = root->get_leaf_element();
        auto branch_first = branch(it_first, 100);
        REQUIRE(branch_first.get_current()->get_prefix_length() == 1);
        auto back_branch = traverse_back(branch_first, 1);
        REQUIRE(*back_branch == 0);
    }

    SECTION("prefix length 0")
    {
        auto root = tree_node<int>::create_root();
        REQUIRE(root->get_prefix_length() == 0);

        auto it = root->push_back(10);
        REQUIRE(it.get_offset() == 0);
        REQUIRE(*it == 10);

        auto back = traverse_back(it, 1);
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
        auto root = tree_node<int>::create_root();
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

        auto branch_at_0 = branch(it_at_0, 4);
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
        auto root = tree_node<int>::create_root();
        auto it1 = root->push_back(1);
        REQUIRE(*it1 == 1);

        std::weak_ptr<tree_node<int>> wp1;
        std::weak_ptr<tree_node<int>> wp2;
        std::weak_ptr<tree_node<int>> wp3;

        {
            auto it2 = branch(it1, 2);
            auto level1 = it2.get_current();
            wp1 = level1;
            REQUIRE(*it2 == 2);
            REQUIRE(level1->get_prefix_length() == 1);

            auto it3 = branch(it2, 3);
            auto level2 = it3.get_current();
            REQUIRE(*it3 == 3);
            REQUIRE(level2->get_prefix_length() == 1);

            auto it4 = branch(it3, 4);
            auto level3 = it4.get_current();
            REQUIRE(*it4 == 4);
            REQUIRE(level3->get_prefix_length() == 1);

            auto back1 = traverse_back(it4, 1);
            REQUIRE(back1.is_valid());
            REQUIRE(*back1 == 3);
            REQUIRE(back1.get_current() == level2);

            auto back2 = traverse_back(back1, 1);
            REQUIRE(*back2 == 2);
            REQUIRE(back2.get_current() == level1);

            auto back3 = traverse_back(back2, 1);
            REQUIRE(*back3 == 1);
            REQUIRE(back3.get_current() == root);

            auto back4 = traverse_back(back3, 1);
            REQUIRE(!back4.is_valid());

            auto reduced = reduce(it4, 4);
            REQUIRE(!reduced.is_valid());
        }

        REQUIRE(wp3.expired());
        REQUIRE(wp2.expired());
        REQUIRE(wp1.expired());

        REQUIRE(root->empty());
        REQUIRE(root->is_leaf());
        REQUIRE(root->data_size() == 0);
    }
}
