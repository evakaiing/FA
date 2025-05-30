//
// Created by Des Caldnd on 2/28/2025.
//

#ifndef B_TREE_DISK_HPP
#define B_TREE_DISK_HPP

#include <iterator>
#include <utility>
#include <vector>
#include <concepts>
#include <stack>
#include <fstream>
#include <optional>
#include <cstddef>
#include <filesystem>
#include "serialization_traits.hpp"
#include <iostream>
#include <logger_guardant.h>

template<typename compare, typename tkey>
concept compator = requires(const compare c, const tkey& lhs, const tkey& rhs)
{
    {c(lhs, rhs)} -> std::same_as<bool>;
} && std::copyable<compare> && std::default_initializable<compare>;

template<typename f_iter, typename tkey, typename tval>
concept input_iterator_for_pair = std::input_iterator<f_iter> && std::same_as<typename std::iterator_traits<f_iter>::value_type, std::pair<tkey, tval>>;

template <serializable tkey, serializable tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 2>
class B_tree_disk final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<tkey, tvalue>;
    using disk_key_type  = std::pair<tkey, size_t>;


private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_size_of_node =
        sizeof(size_t) // size
      + sizeof(bool) // _is_leaf
      + sizeof(size_t) // position_in_disk
      + (2 * t - 1) * (sizeof(tkey) + sizeof(size_t)) // ключ + offset значения
      + (2 * t) * sizeof(size_t); // pointers
    static constexpr const size_t invalid_value = std::numeric_limits<size_t>::max();

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;

    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

public:

    struct btree_disk_node
    {
        size_t size; // кол-во заполненных ячеек
        bool _is_leaf;
        size_t position_in_disk;
        std::vector<disk_key_type> keys; // ключ + смещение в файле .data для получения значения
        std::vector<size_t> pointers;

        void serialize(std::fstream& tree_stream, std::fstream& data_stream) const;

        static btree_disk_node deserialize(std::fstream& tree_stream, std::fstream& data_stream);

        explicit btree_disk_node(bool is_leaf);
        btree_disk_node();
    };

private:

    friend btree_disk_node;

    logger* _logger;

    std::fstream _file_for_tree;

    std::fstream _file_for_key_value;

    btree_disk_node _root;

    compare _compare;

public:

    size_t _position_root;

private:


    btree_disk_node _current_node;

    logger* get_logger() const noexcept override;



public:

    static size_t _count_of_node; // только растет

    // region constructors declaration

    explicit B_tree_disk(const std::string& file_path, const compare& cmp = compare(), logger* logger = nullptr);

    // endregion constructors declaration

    // region five declaration

    B_tree_disk(B_tree_disk&& other) noexcept =default;
    B_tree_disk& operator=(B_tree_disk&& other) noexcept =default;

    B_tree_disk(const B_tree_disk& other) =delete;
    B_tree_disk& operator=(const B_tree_disk& other) =delete;

    ~B_tree_disk() noexcept = default;

    // endregion five declaration

    // region iterators declaration

    class btree_disk_const_iterator
    {
        std::stack<std::pair<size_t , size_t>> _path;
        size_t _index;
        B_tree_disk<tkey,tvalue, compare, t>& _tree;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;

        mutable value_type _cached;


        using self = btree_disk_const_iterator;

        friend class B_tree_disk;

        reference operator*() noexcept;
        pointer operator->() noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        explicit btree_disk_const_iterator(B_tree_disk<tkey, tvalue, compare, t>& tree,
            const std::stack<std::pair<size_t, size_t>>& path = std::stack<std::pair<size_t, size_t>>(),
            size_t index = 0);

    };

    friend class btree_disk_const_iterator;

    std::optional<tvalue> at(const tkey&);

    btree_disk_const_iterator begin();
    btree_disk_const_iterator end() ;

    std::pair<btree_disk_const_iterator, btree_disk_const_iterator> find_range(const tkey& lower, const tkey& upper, bool include_lower = true, bool include_upper = false);

    tvalue read_value_at_offset(size_t offset);

    /*
     * Does nothing if key exists
     * Second return value is true, when inserted
     */
    bool insert(const tree_data_type& data);

    /*
     * Updates value if key exists
     */
    bool update(const tree_data_type& data);

    /*
     * Return true if deleted
     */

    bool erase(const tkey& key);

    bool erase_recursive(size_t node_pos, const tkey& k);

    bool is_valid() const noexcept;

    // путь до ноды + {ind, founded}, где ind == номер позиции, на которую можно вставить
    std::pair<std::stack<std::pair<size_t, size_t>>, std::pair<size_t, bool>>  find_path(const tkey& key);

public:

    btree_disk_node disk_read(size_t position);


    void check_tree(size_t pos, size_t depth);

    void disk_write(btree_disk_node& node);
private:

    std::pair<size_t, bool> find_index(const tkey &key, btree_disk_node& node) const noexcept;

    void insert_array(btree_disk_node& node, size_t right_node, const disk_key_type& data, size_t index);

    void split_node(std::stack<std::pair<size_t, size_t>>& path);

    void remove_from_internal(
    btree_disk_node& node, size_t node_pos, size_t key_idx);

    void remove_from_leaf(size_t node_pos, size_t key_idx);

    tree_data_type get_predecessor(size_t pos);

    tree_data_type get_successor(size_t pos);

    void merge_children(btree_disk_node& parent, size_t parent_pos, size_t key_idx);

    void fill_child(btree_disk_node& parent, size_t parent_pos, size_t key_idx);

    void borrow_from_prev(btree_disk_node& parent, size_t parent_pos, size_t key_idx);

    void borrow_from_next(btree_disk_node& parent, size_t parent_pos, size_t key_idx);


    void print_root_position() noexcept;

};

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::is_valid() const noexcept {
    try {
        check_tree(_position_root, 0);
        return true;
    } catch (...) {
        return false;
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::erase_recursive(size_t node_pos, const tkey& k) {
    constexpr size_t min_keys = t - 1;

    btree_disk_node node = disk_read(node_pos);
    size_t idx = 0;

    while (idx < node.size && _compare(node.keys[idx].first, k)) {
        ++idx;
    }

    bool found = (idx < node.size &&
                 !_compare(k, node.keys[idx].first) &&
                 !_compare(node.keys[idx].first, k));

    if (found) {
        if (node._is_leaf) {
            node.keys.erase(node.keys.begin() + idx);
            --node.size;
            disk_write(node);
            return true;
        } else {
            remove_from_internal(node, node_pos, idx);
            return true;
        }
    }

    if (node._is_leaf) {
        return false;
    }

    size_t child_pos = node.pointers[idx];
    btree_disk_node child = disk_read(child_pos);

    if (child.size == min_keys) {
        fill_child(node, node_pos, idx);
        node = disk_read(node_pos);
        child_pos = node.pointers[idx];
    }

    return erase_recursive(child_pos, k);
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::remove_from_internal(
    btree_disk_node& node, size_t node_pos, size_t key_idx
) {
    /*
    *             pos 4  [30]
                        /    \
        pos 5  [10, 20]   pos 6  [40, 50]

    *             pos 4  [20]
                        /    \
            pos 5  [10]     pos 6  [40, 50]

     *
     */
    constexpr size_t min_keys = t - 1;

    tkey k = node.keys[key_idx].first;
    size_t pred_pos = node.pointers[key_idx];
    size_t succ_pos = node.pointers[key_idx + 1];

    btree_disk_node pred_child = disk_read(pred_pos);
    btree_disk_node succ_child = disk_read(succ_pos);

    if (pred_child.size >= t) {
        // Получаем предшественника: (tkey, tvalue)
        tree_data_type pred = get_predecessor(pred_pos);

        // Сохраняем новое значение в data-файл
        _file_for_key_value.seekp(0, std::ios::end);
        size_t pred_offset = _file_for_key_value.tellp();
        serialization_traits<tvalue>::serialize(pred.second, _file_for_key_value);

        // Обновляем ключ в узле
        node.keys[key_idx] = {pred.first, pred_offset};
        disk_write(node);

        erase_recursive(pred_pos, pred.first);
    }
    else if (succ_child.size >= t) {
        tree_data_type succ = get_successor(succ_pos);

        _file_for_key_value.seekp(0, std::ios::end);
        size_t succ_offset = _file_for_key_value.tellp();
        serialization_traits<tvalue>::serialize(succ.second, _file_for_key_value);

        node.keys[key_idx] = {succ.first, succ_offset};
        disk_write(node);

        erase_recursive(succ_pos, succ.first);
    }
    else {
        merge_children(node, node_pos, key_idx);
        btree_disk_node parent_updated = disk_read(node_pos);

        size_t next_pos;
        if (parent_updated.size == 0) {
            next_pos = parent_updated.pointers[0];
        } else {
            next_pos = parent_updated.pointers[key_idx];
        }

        erase_recursive(next_pos, k);
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::remove_from_leaf(size_t node_pos, size_t key_idx)
{
    btree_disk_node node = disk_read(node_pos);

    if (key_idx >= node.keys.size()) {
        throw std::logic_error("remove_from_leaf_disk: invalid key index");
    }

    node.keys.erase(node.keys.begin() + key_idx);
    --node.size;

    disk_write(node); // обновляем узел на диске
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::tree_data_type B_tree_disk<tkey, tvalue, compare, t>::get_predecessor(size_t pos) {
    btree_disk_node current = disk_read(pos);
    while (!current._is_leaf) {
        pos = current.pointers[current.size];
        current = disk_read(pos);
    }
    const auto& [key, offset] = current.keys.back();
    tvalue val = read_value_at_offset(offset);
    return {key, val};
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::tree_data_type B_tree_disk<tkey, tvalue, compare, t>::get_successor(size_t pos) {
    btree_disk_node current = disk_read(pos);
    size_t current_pos;

    while(!current._is_leaf) {
        pos = current.pointers[0];
        current = disk_read(pos);
    }

    const auto& [key, offset] = current.keys.front();
    tvalue val = read_value_at_offset(offset);
    return {key, val};
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::merge_children(btree_disk_node& parent, size_t parent_pos, size_t key_idx)
{
    size_t left_pos = parent.pointers[key_idx];
    size_t right_pos = parent.pointers[key_idx + 1];

    btree_disk_node left_child = disk_read(left_pos);
    btree_disk_node right_child = disk_read(right_pos);

    // Спускаем родителя к левому ребенку
    left_child.keys.push_back(std::move(parent.keys[key_idx]));
    ++left_child.size;

    // Объединяем детей, присоединяя к левому
    left_child.keys.insert(left_child.keys.end(),
        std::make_move_iterator(right_child.keys.begin()),
        std::make_move_iterator(right_child.keys.end()));
    left_child.size += right_child.size;
    right_child.keys.clear();
    right_child.size = 0;

    // Переносим все указатели
    if (!left_child._is_leaf) {
        left_child.pointers.insert(left_child.pointers.end(),
            std::make_move_iterator(right_child.pointers.begin()),
            std::make_move_iterator(right_child.pointers.end()));
        right_child.pointers.clear();
    }

    // Сохранение удаленных узлов
    disk_write(left_child);
    disk_write(right_child);

    parent.keys.erase(parent.keys.begin() + parent_pos);
    parent.pointers.erase(parent.pointers.begin() + key_idx + 1);
    --parent.size;

    // Обновление root, если parent — это корень и он опустел
    if (parent_pos == _position_root && parent.size == 0) {
        _position_root = left_child.position_in_disk;
        _file_for_tree.seekp(0);
        _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(_position_root));
    }

    disk_write(parent);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::fill_child(btree_disk_node& parent, size_t parent_pos, size_t child_idx)
{
    // Каждый узел, кроме корня, должен содержать не менее t - 1 ключей. Нельзя удалять узел с 1 ключом, нужно слить детей и потом удалить
    // Так что перед тем, как спуститься в этот узел, нужно либо одолжить ключ у брата, либо у родителя, превращая три элемента в один узел

    constexpr size_t min_keys = t - 1;

    size_t child_pos = parent.pointers[child_idx];
    btree_disk_node child = disk_read(child_pos);

    bool has_left = (child_idx > 0);
    bool has_right = (child_idx + 1 < parent.pointers.size());

    // Проверка на левый брата
    if (has_left) {
        size_t left_pos = parent.pointers[child_idx - 1];
        btree_disk_node left = disk_read(left_pos);
        if (left.size > min_keys) {
            borrow_from_prev(parent, parent_pos, child_idx);
            return;
        }
    }

    // Проверка на правый брат
    if (has_right) {
        size_t right_pos = parent.pointers[child_idx + 1];
        btree_disk_node right = disk_read(right_pos);
        if (right.size > min_keys) {
            borrow_from_next(parent, parent_pos, child_idx);
            return;
        }
    }

    // Ни один не может одолжить - мержим
    /*
     *   [10]
     * [3] [12]
     *
     * [3] [10] [12]
     */
    if (has_right) {
        merge_children(parent, parent_pos, child_idx);
    } else if (has_left) {
        merge_children(parent, parent_pos, child_idx - 1);
    } else {
        throw std::logic_error("fill_child_disk: no suitable siblings to borrow or merge");
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::borrow_from_prev(btree_disk_node& parent, size_t parent_pos, size_t child_idx)
{
    /*
     *
     *
    *                [30]

            [10, 20]        [40]

                    to

                    [20]

             [10]       [30, 40]

    */

    size_t child_pos = parent.pointers[child_idx];
    size_t sibling_pos = parent.pointers[child_idx - 1];

    btree_disk_node child = disk_read(child_pos);
    btree_disk_node sibling = disk_read(sibling_pos);

    child.keys.insert(child.keys.begin(), parent.keys[child_idx - 1]);
    ++child.size;

    parent.keys[child_idx - 1] = sibling.keys.back();
    sibling.keys.pop_back();
    --sibling.size;

    if (!child._is_leaf) {
        child.pointers.insert(child.pointers.begin(), sibling.pointers.back());
        sibling.pointers.pop_back();
    }

    disk_write(child);
    disk_write(sibling);
    disk_write(parent);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::borrow_from_next(btree_disk_node& parent, size_t parent_pos, size_t child_idx)
{
    /*
    *
    *
   *                [30]

           [10]            [40, 50]

                   to

                   [40]

            [10, 30]       [50]

   */

    size_t child_pos = parent.pointers[child_idx];
    size_t sibling_pos = parent.pointers[child_idx + 1];

    btree_disk_node child = disk_read(child_pos);
    btree_disk_node sibling = disk_read(sibling_pos);

    child.keys.push_back(parent.keys[child_idx]);
    ++child.size;

    parent.keys[child_idx - 1] = sibling.keys.front();
    sibling.keys.erase(sibling.keys.begin());
    --sibling.size;

    if (!child._is_leaf) {
        child.pointers.insert(child.pointers.begin(), sibling.pointers.back());
        sibling.pointers.pop_back();
    }

    disk_write(child);
    disk_write(sibling);
    disk_write(parent);
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::erase(const tkey& key) {
    if (this->_logger) this->_logger->debug("erase() started.");

    if (_position_root == invalid_value) {
        return false; // Дерево пустое
    }

    bool erased = erase_recursive(_position_root, key);
    if (!erased) return false;

    btree_disk_node root = disk_read(_position_root);

    if (root.size == 0) {
        if (root._is_leaf) {
            _position_root = invalid_value;
        } else {
            _position_root = root.pointers[0];
        }

        _file_for_tree.seekp(0);
        _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(_position_root));
    }

    return true;
    if (this->_logger) this->_logger->debug("erase() finished.");
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::print_root_position() noexcept
{
    if(!_file_for_tree.is_open())
    {
        return;
    }
    _file_for_tree.seekg(sizeof(size_t), std::ios::beg);
    _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(size_t));
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::update(const B_tree_disk::tree_data_type &data)
{
    if (this->_logger) this->_logger->debug("update() started.");

    auto [path, found_info] = find_path(data.first);
    if (!found_info.second) {
        return false;
    }

    // Записываем новое значение в конец .data
    _file_for_key_value.seekp(0, std::ios::end);
    size_t new_offset = _file_for_key_value.tellp();
    serialization_traits<tvalue>::serialize(data.second, _file_for_key_value);

    // Обновляем offset у ключа
    auto [node_pos, index] = path.top();
    btree_disk_node node = disk_read(node_pos);
    node.keys[index].second = new_offset;


    disk_write(node);

    if (this->_logger) this->_logger->debug("update() started.");

    return true;
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::insert(const B_tree_disk::tree_data_type &data) {
    // Записываем значение в конец .data и получаем offset
    if (this->_logger) this->_logger->debug("insert() started.");

    _file_for_key_value.seekp(0, std::ios::end);
    size_t value_offset = _file_for_key_value.tellp();
    serialization_traits<tvalue>::serialize(data.second, _file_for_key_value);

    std::pair<tkey, size_t> key_offset_pair = {data.first, value_offset};

    if (_position_root == std::numeric_limits<size_t>::max()) {
        // Первый узел — создаём корень
        btree_disk_node root(true);
        root.position_in_disk = ++_count_of_node;
        root.size = 1;

        root.keys.push_back(key_offset_pair);
        disk_write(root);
        _position_root = root.position_in_disk;

        _file_for_tree.seekp(0);
        _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(_position_root));

        return true;
    }

    auto [path, found_info] = find_path(data.first);
    if (found_info.second) {
        return false;
    }

    // Вставляем в отсортированный массив на нужный индекс
    auto [leaf_pos, index] = path.top();
    btree_disk_node leaf = disk_read(leaf_pos);
    insert_array(leaf, 0, key_offset_pair, index);
    disk_write(leaf);

    // Сплит, если нужно
    while (leaf.size > 2 * t - 1) {
        split_node(path);
        if (path.empty()) break;
        auto [p, _] = path.top();
        leaf = disk_read(p);
    }

    if (this->_logger) this->_logger->debug("insert() finished.");

    return true;
}



template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::split_node(std::stack<std::pair<size_t, size_t>>& path)
{
    if (path.empty()) return;

    auto [pos, index] = path.top();
    path.pop();

    btree_disk_node node = disk_read(pos);

    size_t median_index = t; // Центральный элемент справа

    disk_key_type median_element = node.keys[median_index]; // Элемент, который пойдет навех

    // Создаем узел, который будет правым ребенком медианы, копируем все
    btree_disk_node right_node(node._is_leaf);
    right_node.keys.assign(node.keys.begin() + median_index + 1, node.keys.end());
    if (!right_node._is_leaf) {
        right_node.pointers.assign(node.pointers.begin() + median_index + 1, node.pointers.end());
    }
    right_node.size = right_node.keys.size();
    right_node.position_in_disk = ++_count_of_node;

    disk_write(right_node);

    // Обрезаем левый
    node.keys.resize(median_index);
    if (!node._is_leaf) node.pointers.resize(median_index + 1);
    node.size = node.keys.size();
    disk_write(node);

    if (path.empty()) {
        // Создание нового корня
        btree_disk_node new_root(false);
        new_root.keys.push_back(median_element);
        new_root.pointers.push_back(pos);
        new_root.pointers.push_back(right_node.position_in_disk);
        new_root.size = 1;
        new_root.position_in_disk = ++_count_of_node;

        disk_write(new_root);
        _position_root = new_root.position_in_disk;

        // Перезаписываем позицию корня
        _file_for_tree.seekp(0);
        _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(_position_root));
    } else {
        auto [parent_pos, idx] = path.top();
        btree_disk_node parent = disk_read(parent_pos);
        insert_array(parent, right_node.position_in_disk, median_element, idx);
        disk_write(parent);
    }


}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::insert_array(btree_disk_node& node, size_t right_node, const disk_key_type& data, size_t index)
{
    /* keys     = [10, 30]
    pointers = [A, B, C] // позиции в файле

    вставляем key = 20, index = 1
    right_node = D (указатель на подузел, который >= 20, < 30)

    pointers = [A, B, D, C]
        key:  10  20  30
    поддеревья:
            A   B   D(right_node, idx = index + 1) C
    */

    node.keys.insert(node.keys.begin() + index, data);

    if (!node._is_leaf) {
        node.pointers.insert(node.pointers.begin() + index + 1, right_node);
    }

    ++node.size;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<std::stack<std::pair<size_t, size_t>>, std::pair<size_t,bool>>  B_tree_disk<tkey, tvalue, compare, t>::find_path(const tkey& key)
{
    // position_in_disk, index in node
    std::stack<std::pair<size_t, size_t>> path;
    size_t current_pos = _position_root;

    while (true) {
        btree_disk_node node = disk_read(current_pos);

        size_t index = 0;
        while (index < node.size  && compare_keys(node.keys[index].first, key)) {
            ++index;
        }

        path.emplace(current_pos, index);

        if (index < node.size && !compare_keys(key, node.keys[index].first) && !compare_keys(node.keys[index].first, key)) {
            return {path, {index, true}};
        }

        if (node._is_leaf) {
            return {path, {index, false}};
        }

        current_pos = node.pointers[index];

    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<size_t, bool> B_tree_disk<tkey, tvalue, compare, t>::find_index(const tkey &key, btree_disk_node& node) const noexcept {
    size_t index = 0;

    while (index < node.size && compare_keys(node.keys[index].first, key)) {
        ++index;
    }

    if (index < node.size &&
        !compare_keys(key, node.keys[index].first) &&
        !compare_keys(node.keys[index].first, key)) {
        return {index, true};
        }

    return {index, false};
}


template <serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::serialize(
    std::fstream& tree_stream,
    std::fstream& data_stream
) const {
    tree_stream.seekp(position_in_disk * maximum_size_of_node);

    tree_stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
    tree_stream.write(reinterpret_cast<const char*>(&_is_leaf), sizeof(_is_leaf));
    tree_stream.write(reinterpret_cast<const char*>(&position_in_disk), sizeof(position_in_disk));

    for (const auto& [key, offset] : keys) {
        serialization_traits<tkey>::serialize(key, tree_stream);
        tree_stream.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    for (size_t ptr : pointers) {
        tree_stream.write(reinterpret_cast<const char*>(&ptr), sizeof(ptr));
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::disk_write(btree_disk_node& node)
{
    node.serialize(_file_for_tree, _file_for_key_value);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::deserialize(std::fstream &tree_stream, std::fstream& data_stream) {
    btree_disk_node node;

    tree_stream.read(reinterpret_cast<char*>(&node.size), sizeof(node.size));
    tree_stream.read(reinterpret_cast<char*>(&node._is_leaf), sizeof(node._is_leaf));
    tree_stream.read(reinterpret_cast<char*>(&node.position_in_disk), sizeof(node.position_in_disk));

    for (size_t i = 0; i < node.size; ++i) {
        tkey key = serialization_traits<tkey>::deserialize(tree_stream);
        size_t offset;
        tree_stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));

        node.keys.emplace_back(std::move(key), offset);
    }

    size_t num_ptrs = node._is_leaf ? 0 : node.size + 1;
    for (size_t i = 0; i < num_ptrs; ++i) {
        size_t ptr;
        tree_stream.read(reinterpret_cast<char*>(&ptr), sizeof(ptr));
        node.pointers.push_back(ptr);
    }

    return node;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node
B_tree_disk<tkey, tvalue, compare, t>::disk_read(size_t node_position)
{
    _file_for_tree.seekg(node_position * maximum_size_of_node);  // Переходим на начало нужного узла
    return btree_disk_node::deserialize(_file_for_tree, _file_for_key_value);
}


template <serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::btree_disk_node(bool is_leaf)
    : _is_leaf(is_leaf), size(0), position_in_disk(_count_of_node) {}


template <serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::btree_disk_node()
    : _is_leaf(true), size(0), position_in_disk(_count_of_node) {}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::compare_pairs(
    const tree_data_type& lhs,
    const tree_data_type& rhs
) const {
    return compare_keys(lhs.first, rhs.first) && (lhs.second == rhs.second);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
size_t B_tree_disk<tkey, tvalue, compare, t>::_count_of_node = 0;

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::B_tree_disk(const std::string& file_path, const compare& cmp, logger* logger_p)
    : compare(cmp), _logger(logger_p)
{
    std::string tree_file = file_path + ".tree";
    std::string data_file = file_path + ".data";

    _file_for_tree.open(tree_file, std::ios::in | std::ios::out | std::ios::binary);
    _file_for_key_value.open(data_file, std::ios::in | std::ios::out | std::ios::binary);

    if (!_file_for_tree.is_open()) {
        // Файл не существует — создаём
        // std::fstream не создаёт файл, если открываешь его в std::ios::in | std::ios::out, поэтому приходится делать так:
        _file_for_tree.open(tree_file, std::ios::out | std::ios::binary);
        _file_for_tree.close();
        _file_for_tree.open(tree_file, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!_file_for_key_value.is_open()) {
        _file_for_key_value.open(data_file, std::ios::out | std::ios::binary);
        _file_for_key_value.close();
        _file_for_key_value.open(data_file, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!_file_for_tree.is_open()) {
        throw std::runtime_error("Не удалось открыть или создать файл дерева: " + tree_file);
    }

    if (!_file_for_key_value.is_open()) {
        throw std::runtime_error("Не удалось открыть или создать файл данных: " + data_file);
    }



    // Если .tree пустой
    if (std::filesystem::file_size(tree_file) < sizeof(size_t)) {
        // Первый запуск — ещё нет дерева
        _position_root = invalid_value;
        _count_of_node = 0;
        // Записываем значение корня (пока не инициализирован) в начало
        _file_for_tree.seekp(0);
        _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(_position_root));
    } else {
        // Дерево уже есть — читаем позицию корня
        _file_for_tree.seekg(0);
        _file_for_tree.read(reinterpret_cast<char*>(&_position_root), sizeof(_position_root));
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::check_tree(size_t pos, size_t depth) {
    static std::optional<size_t> expected_leaf_depth;

    if (pos == invalid_value) {
        throw std::logic_error("check_tree: invalid node position (invalid_value)");
    }

    btree_disk_node node = disk_read(pos);

    if (node.size > 2 * t - 1) {
        throw std::logic_error("check_tree: too many keys in node");
    }
    if (pos != _position_root && node.size < t - 1) {
        throw std::logic_error("check_tree: too few keys in non-root node");
    }

    for (size_t i = 1; i < node.size; ++i) {
        if (!compare_keys(node.keys[i - 1].first, node.keys[i].first)) {
            throw std::logic_error("check_tree: keys are not strictly ordered in node");
        }
    }

    if (!node._is_leaf) {
        if (node.pointers.size() != node.size + 1) {
            throw std::logic_error("check_tree: number of pointers != number of keys + 1");
        }

        for (size_t i = 0; i < node.pointers.size(); ++i) {
            if (node.pointers[i] == invalid_value) {
                throw std::logic_error("check_tree: invalid child pointer found (invalid_value)");
            }
        }

        for (size_t ptr : node.pointers) {
            check_tree(ptr, depth + 1);
        }
    } else {
        if (!expected_leaf_depth.has_value()) {
            expected_leaf_depth = depth;
        } else if (depth != expected_leaf_depth.value()) {
            throw std::logic_error("check_tree: leaf nodes are not at the same depth");
        }
    }

    if (depth == 0) {
        expected_leaf_depth.reset();
    }
}



template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::btree_disk_const_iterator(
    B_tree_disk<tkey, tvalue, compare, t>& tree,
    const std::stack<std::pair<size_t, size_t>>& path,
    size_t index
) : _tree(tree), _path(path), _index(index) {}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator
B_tree_disk<tkey, tvalue, compare, t>::begin()
{
    std::stack<std::pair<size_t, size_t>> path;

    if (_position_root == invalid_value) {
        return btree_disk_const_iterator(*this); // пустой итератор == end()
    }

    size_t current_pos = _position_root;
    btree_disk_node current_node = disk_read(current_pos);

    size_t child_idx_from_parent = static_cast<size_t>(-1);
    while (!current_node._is_leaf) {
        path.push({current_pos, child_idx_from_parent});
        child_idx_from_parent = 0;
        current_pos = current_node.pointers[0];
        current_node = disk_read(current_pos);
    }

    // Мы в листе
    if (!current_node.keys.empty()) {
        path.push({current_pos, child_idx_from_parent});
        return btree_disk_const_iterator(*this, path, 0);
    } else {
        // Пустой лист — возвращаем end()
        return btree_disk_const_iterator(*this); // _path будет пустой
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator B_tree_disk<tkey, tvalue, compare, t>::end() {
    return btree_disk_const_iterator(*this); // пустой path, это end()
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator&
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator++()
{
    if (_path.empty()) {
        return *this;
    }

    auto [node_pos, idx] = _path.top();
    btree_disk_node current_node = _tree.disk_read(node_pos);

    // Если у текущего узла есть потомки, пытаемся пойти вправо
    if (!current_node._is_leaf) {
        size_t right_child_idx = _index + 1;

        if (right_child_idx < current_node.pointers.size()) {
            size_t next_node_pos = current_node.pointers[right_child_idx];
            btree_disk_node next_node = _tree.disk_read(next_node_pos);
            _path.push({next_node_pos, 0});

            while (!next_node._is_leaf) {
                size_t leftmost_child = next_node.pointers[0];
                _path.push({leftmost_child, 0});
                next_node = _tree.disk_read(leftmost_child);
            }

            _index = 0;
            return *this;
        }
    }

    _index++;
    if (_index < current_node.size) {
        return *this;
    }

    // Поднимаемся вверх по стеку
    while (!_path.empty()) {
        size_t parent_child_idx = _path.top().second;
        _path.pop();

        if (_path.empty()) {
            // Мы были в поддереве root и поднялись к нему, но стек стал пуст
            // Это возможно только если root — текущий узел
            btree_disk_node root = _tree.disk_read(_tree._position_root);

            if (parent_child_idx < root.size) {
                _path.push({_tree._position_root, parent_child_idx});
                _index = parent_child_idx;
                return *this;
            }

            break;
        }

        auto [parent_pos, parent_idx] = _path.top();
        btree_disk_node parent_node = _tree.disk_read(parent_pos);

        if (parent_child_idx < parent_node.size) {
            _index = parent_child_idx;
            return *this;
        }
    }

    _path = {};
    _index = 0;
    return *this;
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator&
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator--() {
    if (_path.empty()) {
        // end
        size_t current_pos = _tree._position_root;
        while (true) {
            btree_disk_node node = _tree.disk_read(current_pos);
            size_t index = node.size - 1;
            _path.emplace(current_pos, index);

            if (node._is_leaf) {
                _index = index;
                return *this;
            }
            current_pos = node.pointers[node.size]; // самый правый потомок
        }
    }

    auto& [node_pos, idx] = _path.top();
    btree_disk_node node = _tree.disk_read(node_pos);

    // Спускаемся в левое поддерево
    if (!node._is_leaf) {
        size_t left_child_pos = node.pointers[idx];
        while (true) {
            btree_disk_node child = _tree.disk_read(left_child_pos);
            size_t index = child.size - 1;
            _path.emplace(left_child_pos, index);

            if (child._is_leaf) {
                _index = index;
                return *this;
            }
            left_child_pos = child.pointers[child.size];
        }
    }

    // Можно двигаться влево по текущему узлу
    if (idx > 0) {
        idx--;
        _index = idx;
        return *this;
    }

    // Поднимаемся: текущий узел был первым в своём родителе
    _path.pop();
    while (!_path.empty()) {
        auto& [parent_pos, parent_idx] = _path.top();
        _index = parent_idx;
        return *this;
    }

    // Вышли за пределы дерева (begin - 1)
    _index = static_cast<size_t>(-1);
    return *this;
}



template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return tmp;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator==(
    const self& other
) const noexcept {
    if (_path.empty() && other._path.empty()) {
        return true; // оба — end()
    }

    if (_path.empty() || other._path.empty()) {
        return false; // только один — end()
    }

    const auto [this_node_pos, _] = _path.top();
    const auto [other_node_pos, __] = other._path.top();

    return this_node_pos == other_node_pos && _index == other._index;
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator!=(
    const self& other
) const noexcept {
    return !(*this == other);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::reference
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator*() noexcept
{
    if (_path.empty()) {
        static value_type empty{};
        return empty;
    }

    auto [node_pos, idx] = _path.top();
    btree_disk_node node = _tree.disk_read(node_pos);

    if (_index >= node.size) {
        static value_type invalid{};
        return invalid;
    }

    const auto& [key, offset] = node.keys[_index];
    _tree._file_for_key_value.seekg(offset);
    tvalue value = serialization_traits<tvalue>::deserialize(_tree._file_for_key_value);

    _cached = {key, std::move(value)};
    return _cached;
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::pointer B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator->() noexcept
{
    return &operator*(); // безопасно, тк operator* вернёт _cached
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<
    typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator,
    typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator
>
B_tree_disk<tkey, tvalue, compare, t>::find_range(
    const tkey& lower,
    const tkey& upper,
    bool include_lower,
    bool include_upper
) {
    std::stack<std::pair<size_t, size_t>> path_begin, path_end;
    size_t index_begin = 0, index_end = 0;

    // Нижняя граница
    {
        auto [path, found_info] = find_path(lower);
        if (!path.empty()) {
            path_begin = path;
            index_begin = path.top().second;
            if (found_info.second && !include_lower) {
                ++index_begin;
            }
        }
    }

    // Верхняя граница
    {
        auto [path, found_info] = find_path(upper);
        if (!path.empty()) {
            path_end = path;
            index_end = path.top().second;
            if (found_info.second && !include_upper) {
                --index_end;
            }
        }
    }

    return {
        btree_disk_const_iterator(*this, path_begin, index_begin),
        btree_disk_const_iterator(*this, path_end, index_end)
    };
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::optional<tvalue> B_tree_disk<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto [path, found_info] = find_path(key);

    if (found_info.second) {
        auto [node_pos, index] = path.top();
        btree_disk_node node = disk_read(node_pos);

        size_t offset = node.keys[index].second;
        tvalue val = read_value_at_offset(offset);

        return val;
    }

    return std::nullopt;
}



template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
tvalue B_tree_disk<tkey, tvalue, compare, t>::read_value_at_offset(size_t offset) {
    _file_for_key_value.seekg(offset);
    return serialization_traits<tvalue>::deserialize(_file_for_key_value);
}


#endif //B_TREE_DISK_HPP
