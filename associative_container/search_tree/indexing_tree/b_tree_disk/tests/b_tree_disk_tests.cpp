//
// Created by Des Caldnd on 2/28/2025.
//
#include <gtest/gtest.h>
#include <b_tree_disk.hpp>

const std::string TMP_PATH = "/tmp/b_tree_disk_test";

void clear_files(const std::string& base) {
    std::filesystem::remove(base + ".tree");
    std::filesystem::remove(base + ".data");
}

TEST(BTreeDiskTest, SerializeAndDeserialize) {
    using tree_type = B_tree_disk<int, std::string, std::less<int>, 2>;

    std::string filename = "btree_disk_test";
    clear_files(filename);
    {
        tree_type tree(filename);

        EXPECT_TRUE(tree.insert({10, "ten"}));
        EXPECT_TRUE(tree.insert({20, "twenty"}));
        EXPECT_TRUE(tree.insert({5, "five"}));
        EXPECT_TRUE(tree.insert({17, "seventeen"}));

    }

    {
        tree_type tree2(filename);

        auto [path1, found1] = tree2.find_path(10);
        EXPECT_TRUE(found1.second);

        auto [path2, found2] = tree2.find_path(20);
        EXPECT_TRUE(found2.second);

        auto [path3, found3] = tree2.find_path(5);
        EXPECT_TRUE(found3.second);

        auto [path4, found4] = tree2.find_path(17);
        EXPECT_TRUE(found4.second);
    }
}


TEST(BTreeDiskEraseTest, RemoveFromLeafWithoutViolation) {
    clear_files(TMP_PATH + "1");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH +"1");

    tree.insert({10, "a"});
    tree.insert({20, "b"});
    tree.insert({30, "c"});
    tree.insert({40, "d"});
    tree.insert({50, "e"});

    // Удаляем ключ из листа, где после удаления останется больше min_keys
    ASSERT_TRUE(tree.erase(20));
    ASSERT_TRUE(tree.erase(40));// несуществующий ключ
}

TEST(BTreeDiskEraseTest, RemoveFromLeafWithBorrowLeft) {
    clear_files(TMP_PATH+  "2");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH + "2");

    tree.insert({10, "a"});
    tree.insert({20, "b"});
    tree.insert({30, "c"});
    tree.insert({40, "d"});

    // Удаляем ключ из листа, и ожидаем borrow из левого брата
    ASSERT_TRUE(tree.erase(40));
}

TEST(BTreeDiskEraseTest, RemoveFromLeafWithMerge) {
    clear_files(TMP_PATH + "4");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH +"4");

    tree.insert({10, "a"});
    tree.insert({30, "b"});
    tree.insert({40, "c"});

    // слияние
    ASSERT_TRUE(tree.erase(10));
}

TEST(BTreeDiskEraseTest, RemoveFromInternalPredecessor) {
    clear_files(TMP_PATH + "5");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH + "5");

    tree.insert({10, "a"});
    tree.insert({20, "b"});
    tree.insert({30, "c"});
    tree.insert({40, "d"});
    tree.insert({50, "e"});

    ASSERT_TRUE(tree.erase(30)); // заменится на predecessor
}

TEST(BTreeDiskEraseTest, RemoveFromInternalSuccessor) {
    clear_files(TMP_PATH + "6");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH +  "6");

    tree.insert({10, "a"});
    tree.insert({20, "b"});
    tree.insert({30, "c"});
    tree.insert({35, "d"});
    tree.insert({40, "e"});
    tree.insert({45, "f"});

    ASSERT_TRUE(tree.erase(30)); // заменится на successor
}

TEST(BTreeDiskEraseTest, RemoveInternalAndMergeChildren) {
    clear_files(TMP_PATH + "7");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH + "7");

    tree.insert({10, "a"});
    tree.insert({20, "b"});
    tree.insert({30, "c"});
    tree.insert({40, "d"});
    tree.insert({50, "e"});

    ASSERT_TRUE(tree.erase(20));
    ASSERT_TRUE(tree.erase(40));
    ASSERT_TRUE(tree.erase(30)); // merge
}

TEST(BTreeDiskEraseTest, EraseRootBecomeEmpty) {
    clear_files(TMP_PATH + "8");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH  +"8");

    tree.insert({10, "a"});
    tree.insert({20, "b"});
    tree.insert({30, "c"});

    ASSERT_TRUE(tree.erase(10));
    ASSERT_TRUE(tree.erase(20));
    ASSERT_TRUE(tree.erase(30)); // root обнулится
}

TEST(BTreeDiskIteratorTest, BeginEndTraversal) {
    clear_files(TMP_PATH + "iter1");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH + "iter1");

    tree.insert({10, "ten"});
    tree.insert({20, "twenty"});
    tree.insert({5, "five"});
    tree.insert({15, "fifteen"});

    std::vector<std::pair<int, std::string>> expected = {
        {5, "five"},
        {10, "ten"},
        {15, "fifteen"},
        {20, "twenty"}
    };

    auto it = tree.begin();
    for (const auto& [key, value] : expected) {
        ASSERT_EQ((*it).first, key);
        ASSERT_EQ((*it).second, value);
        ++it;
    }
}

TEST(BTreeDiskIteratorTest, DecrementFromEnd) {
    clear_files(TMP_PATH + "iter2");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH + "iter2");

    tree.insert({1, "one"});
    tree.insert({2, "two"});
    tree.insert({3, "three"});
    tree.insert({4, "four"});

    auto it = tree.end();
    --it; ASSERT_EQ(it->first, 4); ASSERT_EQ(it->second, "four");
    --it; ASSERT_EQ(it->first, 3); ASSERT_EQ(it->second, "three");
    --it; ASSERT_EQ(it->first, 2); ASSERT_EQ(it->second, "two");
    --it; ASSERT_EQ(it->first, 1); ASSERT_EQ(it->second, "one");
}

TEST(BTreeDiskIteratorTest, IteratorComparison) {
    clear_files(TMP_PATH + "iter3");
    B_tree_disk<int, std::string, std::less<int>, 2> tree(TMP_PATH + "iter3");

    tree.insert({100, "hundred"});

    auto it1 = tree.begin();
    auto it2 = tree.begin();
    auto it3 = tree.end();

    ASSERT_TRUE(it1 == it2);
    ASSERT_FALSE(it1 != it2);
    ASSERT_TRUE(it1 != it3);
}

TEST(BTreeDiskIteratorTest, WithT5) {
    clear_files(TMP_PATH + "t5");
    B_tree_disk<int, std::string, std::less<int>, 5> tree(TMP_PATH + "t5");

    std::vector<std::pair<int, std::string>> input = {
        {1, "a"}, {2, "b"}, {15, "c"}, {3, "d"}, {4, "e"}, {100, "f"},
        {24, "g"}, {456, "h"}, {101, "j"}, {45, "k"}, {193, "l"}, {534, "m"}
    };

    for (auto& [key, val] : input) {
        tree.insert({key, val});
    }

    std::sort(input.begin(), input.end());

    auto it = tree.begin();
    for (const auto& [key, val] : input) {
        ASSERT_EQ((*it).first, key);
        ASSERT_EQ((*it).second, val);
        ++it;
    }
}

TEST(BTreeDiskIteratorTest, WithT7) {
    clear_files(TMP_PATH + "t7");
    B_tree_disk<int, std::string, std::less<int>, 7> tree(TMP_PATH + "t7");

    std::vector<std::pair<int, std::string>> input = {
        {1, "a"},
        {2, "b"},
        {3, "d"},
        {4, "e"},
        {15, "c"},
        {24, "g"},
        {45, "k"},
        {100, "f"},
        {101, "j"},
        {193, "l"},
        {456, "h"},
        {534, "m"}
    };

    for (const auto& [key, val] : input) {
        tree.insert({key, val});
    }

    std::sort(input.begin(), input.end());

    auto it = tree.begin();
    for (const auto& [key, val] : input) {
        ASSERT_EQ((*it).first, key);
        ASSERT_EQ((*it).second, val);
        ++it;
    }

    ASSERT_EQ(it, tree.end());
}

TEST(BTreeDiskIteratorTest, FindRange) {
    clear_files(TMP_PATH + "find_range_test");
    B_tree_disk<int, std::string, std::less<int>, 3> tree(TMP_PATH + "find_range_test");

    std::vector<std::pair<int, std::string>> input = {
        {1, "a"}, {2, "b"}, {3, "c"}, {4, "d"},
        {5, "e"}, {6, "f"}, {7, "g"}, {8, "h"}
    };

    for (const auto& [key, val] : input) {
        tree.insert({key, val});
    }

    // Диапазон [3, 6)
    auto [begin_it, end_it] = tree.find_range(3, 6, true, false);

    std::vector<std::pair<int, std::string>> expected = {
        {3, "c"}, {4, "d"}, {5, "e"}
    };

    size_t idx = 0;
    for (auto it = begin_it; it != end_it; ++it, ++idx) {
        ASSERT_LT(idx, expected.size());
        ASSERT_EQ((*it).first, expected[idx].first);
        ASSERT_EQ((*it).second, expected[idx].second);
    }

    ASSERT_EQ(idx, expected.size() - 1);
}

TEST(BTreeDiskIteratorTest, At) {
    clear_files(TMP_PATH + "at");
    B_tree_disk<int, std::string, std::less<int>, 3> tree(TMP_PATH + "at");

    std::vector<std::pair<int, std::string>> input = {
        {10, "ten"}, {20, "twenty"}, {30, "thirty"}
    };

    for (const auto& [key, val] : input) {
        tree.insert({key, val});
    }

    auto result1 = tree.at(20);
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(result1.value(), "twenty");

    auto result2 = tree.at(25);
    ASSERT_FALSE(result2.has_value());
}


int main(
    int argc,
    char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}