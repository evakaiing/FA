#include <iostream>
#include <cassert>
#include "binary_search_tree.h"
#include <ranges>

int main() {
    using key_t = int;
    using val_t = std::string;
    using value_type = std::pair<const key_t, val_t>;
    using compare_t = std::less<key_t>;
    using tree_t = binary_search_tree<int, std::string>;

    /*
    compare_t comp;
    pp_allocator<value_type> alloc;
    logger* log = nullptr;

    // конструкторы
    binary_search_tree<int, std::string> tree;

    // binary_search_tree<int, std::string> tree2(std::less<int>);
    // std::vector<std::pair<int, std::string>> data = {
    //        {1, "one"}, {2, "two"}, {3, "three"}
    //};
    // binary_search_tree<int, std::string> tree3(std::from_range, data);
    binary_search_tree<int, std::string> tree4({ {1, "one"}, {2, "two"} });


    // copy constructor
    tree_t original;
    original.insert({10, "ten"});
    original.insert({5, "five"});
    original.insert({15, "fifteen"});


    tree_t copy = original;

    // move constructor
    tree_t original2;
    original2.insert({10, "ten"});
    original2.insert({5, "five"});
    original2.insert({15, "fifteen"});

    assert(original2.size() == 3);

    tree_t moved = std::move(original2);

    // copy and move assigment
    tree_t original3;
    original3.insert({11, "ten"});
    original3.insert({7, "five"});
    original3.insert({13, "fifteen"});

    // iterators

    auto it = original3.begin_prefix();
    auto it_end = original3.end_prefix();


     */
    using bst = binary_search_tree<int, std::string>;

    bst tree2;
    tree2.emplace(10, "ten");
    tree2.emplace(5, "five");
    tree2.emplace(15, "fifteen");
    tree2.emplace(3, "three");
    tree2.emplace(7, "seven");

    for (auto it = tree2.rbegin_postfix(); it != tree2.rend_postfix(); ++it) {
        std::cout << it->first <<  "\n";
    }

    /*std::cout << "\nprefix_reverse_iterator:\n";
    for (auto it = tree2.rbegin(); it != tree2.rend(); ++it) {
        std::cout << it->first <<  "\n";
    }

    std::cout << "\nCheck base() and conversion:\n";
    auto r = tree2.rbegin(); // reverse итератор на последний элемент
    auto i = r.base();
    --i; // должен указывать туда же, куда r

    std::cout << "*r: " << r->first << " = " << r->second << "\n";
    std::cout << "*(r.base() - 1): " << i->first << " = " << i->second << "\n";

    return 0;*/


/*
    moved = original3;

    moved = std::move(original);

    auto it = original3.find(11);
    tree2_t::infix_const_iterator it2 = original3.end();
    auto it3 = original.cend();
    auto it4 = original.cbegin();
    std::cout << it2->first << '\n';
    std::cout << original3.at(11) << '\n';
    // works!!!
    // std::cout << original3.at(12) << '\n';

    const tree2_t const_tree = original3;
    std::cout << const_tree.at(13);
    std::pair<int, int> p = {13, 1};
    original3[p.first] = '2';
    original3[1] = "88";
    std::cout << original3[1] << '\n';

    std::pair<int, std::string> p2 = {19, "hi"};
    original3.insert_or_assign(p2);
    std::cout << original3.contains(10);
    tree_t t;
    t.emplace(1, "1");

    std::vector<std::pair<int, std::string>> values = {{1, "1"}, {2, "2"}};
    t.insert_range(values);

    for (auto it = t.begin(); it != t.end(); ++it) {
        std::cout << it->first << ' ';
    }

    auto it5 = original3.lower_bound(7);
    std::cout << it5->first << '\n';

    auto it6 = original3.upper_bound(7);
    std::cout << it6->first << '\n';

    auto it7 = original3.erase(11); */
}

