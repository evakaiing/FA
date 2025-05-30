#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include "b_plus_tree.h"

class BPlusTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Подготавливаем тестовые данные
        simple_data = {
            {1, "one"},
            {2, "two"},
            {3, "three"},
            {4, "four"},
            {5, "five"}
        };

        random_data.clear();
        for (int i = 0; i < 100; i++) {
            random_data.push_back({i, "value_" + std::to_string(i)});
        }
        // Перемешиваем данные для имитации случайных вставок
        std::shuffle(random_data.begin(), random_data.end(), std::mt19937{std::random_device{}()});
    }

    std::vector<std::pair<int, std::string>> simple_data;
    std::vector<std::pair<int, std::string>> random_data;
};

TEST_F(BPlusTreeTest, ConstructorAndEmpty) {
    BP_tree<int, std::string, std::less<int>, 3> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

TEST_F(BPlusTreeTest, InsertAndSize) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    auto result = tree.insert(simple_data[0]);
    EXPECT_TRUE(result.second);
    EXPECT_EQ(tree.size(), 1);
    EXPECT_FALSE(tree.empty());

    result = tree.insert(simple_data[0]);
    EXPECT_FALSE(result.second);
    EXPECT_EQ(tree.size(), 1);

    for (size_t i = 1; i < simple_data.size(); i++) {
        result = tree.insert(simple_data[i]);
        EXPECT_TRUE(result.second);
        EXPECT_EQ(tree.size(), i + 1);
    }
}

TEST_F(BPlusTreeTest, Find) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    for (const auto& item : simple_data) {
        tree.insert(item);
    }

    for (const auto& item : simple_data) {
        auto it = tree.find(item.first);
        EXPECT_NE(it, tree.end());
        EXPECT_EQ(it->first, item.first);
        EXPECT_EQ(it->second, item.second);
    }

    EXPECT_EQ(tree.find(999), tree.end());
}

// Тест на удаление элементов
TEST_F(BPlusTreeTest, Erase) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    // Заполняем дерево
    for (const auto& item : simple_data) {
        tree.insert(item);
    }

    // Удаляем элементы по одному и проверяем размер
    for (size_t i = 0; i < simple_data.size() - 1; i++) {
        auto it = tree.erase(simple_data[i].first);
        EXPECT_NE(it, tree.end());
        EXPECT_EQ(tree.size(), simple_data.size() - i - 1);

        // Проверяем, что элемент действительно удален
        EXPECT_EQ(tree.find(simple_data[i].first), tree.end());
    }

    // Дерево должно содержать только 1 элемент
    EXPECT_TRUE(tree.size() == 1);
}

// Тест на итераторы и порядок обхода
TEST_F(BPlusTreeTest, Iterators) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    // Заполняем дерево
    for (const auto& item : simple_data) {
        tree.insert(item);
    }

    // Проверяем обход в порядке возрастания ключей
    std::vector<int> keys;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        keys.push_back(it->first);
    }

    std::vector<int> expected_keys = {1, 2, 3, 4, 5};
    EXPECT_EQ(keys, expected_keys);

    // Тест на константный итератор
    std::vector<int> const_keys;
    for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
        const_keys.push_back(it->first);
    }
    EXPECT_EQ(const_keys, expected_keys);
}

// Тест на вставку и удаление большого количества элементов (проверка балансировки)
TEST_F(BPlusTreeTest, LargeDatasetTest) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    // Вставка перемешанных данных
    for (const auto& item : random_data) {
        tree.insert(item);
    }

    EXPECT_EQ(tree.size(), random_data.size());

    // Проверяем порядок обхода
    int prev_key = -1;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        EXPECT_GT(it->first, prev_key);
        prev_key = it->first;
    }

    // Удаляем случайный элемент
    if (!random_data.empty()) {
        int key_to_remove = random_data[random_data.size()/2].first;
        tree.erase(key_to_remove);
        EXPECT_EQ(tree.find(key_to_remove), tree.end());
        EXPECT_EQ(tree.size(), random_data.size() - 1);
    }
}

// Тест на оператор доступа по индексу
TEST_F(BPlusTreeTest, AccessOperatorTest) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    // Заполняем дерево
    for (const auto& item : simple_data) {
        tree.insert(item);
    }

    // Проверяем получение значений по ключу
    for (const auto& item : simple_data) {
        EXPECT_EQ(tree[item.first], item.second);
    }

    // Изменение значения по ключу
    tree[3] = "updated_three";
    EXPECT_EQ(tree[3], "updated_three");

    // Вставка нового элемента через оператор
    tree[10] = "ten";
    EXPECT_EQ(tree[10], "ten");
    EXPECT_EQ(tree.size(), simple_data.size() + 1);
}

// Тест на граничные случаи (минимальное и максимальное количество элементов в узле)
TEST_F(BPlusTreeTest, EdgeCasesTest) {
    // Дерево с минимальным порядком (t=2)
    BP_tree<int, std::string, std::less<int>, 2> small_tree;

    // Вставляем элементы, которые вызовут разделение узлов
    for (int i = 0; i < 10; i++) {
        small_tree.insert({i, "value_" + std::to_string(i)});
    }

    // Проверяем, что все элементы были вставлены
    EXPECT_EQ(small_tree.size(), 10);

    // Проверяем, что порядок обхода правильный
    int prev_key = -1;
    for (auto it = small_tree.begin(); it != small_tree.end(); ++it) {
        EXPECT_GT(it->first, prev_key);
        prev_key = it->first;
    }

    // Удаляем элементы и проверяем, что дерево остается сбалансированным
    for (int i = 0; i < 5; i++) {
        small_tree.erase(i);
    }

    EXPECT_EQ(small_tree.size(), 5);

    // Проверяем, что порядок обхода правильный после удаления
    prev_key = 4;
    for (auto it = small_tree.begin(); it != small_tree.end(); ++it) {
        EXPECT_GT(it->first, prev_key - 1);
        prev_key = it->first;
    }
}

TEST(BPlusTreeStressTest, LargeDatasetTest) {
    BP_tree<int, std::string, std::less<int>, 5> tree;

    // Вставка большого количества элементов в случайном порядке
    const int NUM_ELEMENTS = 100000;
    std::vector<int> keys(NUM_ELEMENTS);

    // Генерируем последовательные ключи
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        keys[i] = i;
    }

    // Перемешиваем ключи
    std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});

    // Измеряем время вставки
    auto start = std::chrono::high_resolution_clock::now();

    for (int key : keys) {
        tree.insert({key, "value_" + std::to_string(key)});
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Insertion of " << NUM_ELEMENTS << " elements took " << duration << " ms" << std::endl;

    // Проверяем, что все элементы были вставлены корректно
    EXPECT_EQ(tree.size(), NUM_ELEMENTS);

    // Проверяем наличие всех ключей
    for (int key : keys) {
        EXPECT_NE(tree.find(key), tree.end());
    }

    // Проверяем упорядоченность при обходе
    int prev_key = -1;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        EXPECT_GT(it->first, prev_key);
        prev_key = it->first;
    }

    // Измеряем время удаления
    start = std::chrono::high_resolution_clock::now();

    // Перемешиваем порядок удаления
    std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});

    for (int key : keys) {
        tree.erase(key);
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Deletion of " << NUM_ELEMENTS << " elements took " << duration << " ms" << std::endl;

    // Проверяем, что все элементы были удалены
    EXPECT_TRUE(tree.empty());
}

TEST(BPlusTreeStressTest, ComplexInsertionDeletionSequences) {
    BP_tree<int, std::string, std::less<int>, 3> tree;

    // Тест 1: Вставка в возрастающем порядке, удаление в убывающем
    for (int i = 0; i < 1000; ++i) {
        tree.insert({i, "value_" + std::to_string(i)});
    }

    for (int i = 999; i >= 0; --i) {
        tree.erase(i);
    }

    EXPECT_TRUE(tree.empty());

    // Тест 2: Вставка в убывающем порядке, удаление в возрастающем
    for (int i = 999; i >= 0; --i) {
        tree.insert({i, "value_" + std::to_string(i)});
    }

    for (int i = 0; i < 1000; ++i) {
        tree.erase(i);
    }

    EXPECT_TRUE(tree.empty());

    // Тест 3: Вставка в "пилообразном" порядке 0,999,1,998,...
    std::vector<int> sawtooth_keys;
    for (int i = 0; i < 500; ++i) {
        sawtooth_keys.push_back(i);
        sawtooth_keys.push_back(999 - i);
    }

    for (int key : sawtooth_keys) {
        tree.insert({key, "value_" + std::to_string(key)});
    }

    // Проверка упорядоченного обхода
    int prev = -1;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        EXPECT_GT(it->first, prev);
        prev = it->first;
    }

    // Удаление каждого второго элемента
    for (size_t i = 0; i < sawtooth_keys.size(); i += 2) {
        tree.erase(sawtooth_keys[i]);
    }

    EXPECT_EQ(tree.size(), 500);

    // Проверка оставшихся элементов
    for (size_t i = 1; i < sawtooth_keys.size(); i += 2) {
        EXPECT_NE(tree.find(sawtooth_keys[i]), tree.end());
    }
}

TEST(BPlusTreeStressTest, TreeOrderBoundaryTest) {
    // Тестирование для минимального порядка t=2 (2t-1=3 ключа максимум в узле)
    BP_tree<int, std::string, std::less<int>, 2> min_tree;

    // Последовательность, вызывающая максимальное разделение узлов
    for (int i = 0; i < 100; ++i) {
        min_tree.insert({i, "value_" + std::to_string(i)});
    }

    EXPECT_EQ(min_tree.size(), 100);

    // Проверка удаления, вызывающего слияние узлов
    for (int i = 0; i < 100; i += 2) {
        min_tree.erase(i);
    }

    EXPECT_EQ(min_tree.size(), 50);

    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            EXPECT_EQ(min_tree.find(i), min_tree.end());
        } else {
            EXPECT_NE(min_tree.find(i), min_tree.end());
        }
    }

    // Тестирование для большого порядка дерева (больше быстродействие, меньше разделений)
    BP_tree<int, std::string, std::less<int>, 64> max_tree;

    // Вставка большого количества элементов, должно быть мало разделений
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000; ++i) {
        max_tree.insert({i, "value_" + std::to_string(i)});
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Insertion of 10000 elements in high-order tree took " << duration << " ms" << std::endl;

    // Проверка всех элементов
    for (int i = 0; i < 10000; ++i) {
        EXPECT_NE(max_tree.find(i), max_tree.end());
    }
}

// Запускаем тесты
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
