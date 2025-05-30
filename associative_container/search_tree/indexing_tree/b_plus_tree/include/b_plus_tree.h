#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <search_tree.h>
#include <initializer_list>
#include <logger_guardant.h>

#ifndef MP_OS_B_PLUS_TREE_H
#define MP_OS_B_PLUS_TREE_H

template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BP_tree final : private logger_guardant, private compare {
public:
    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:
    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey &lhs, const tkey &rhs) const;

    inline bool compare_pairs(const tree_data_type &lhs, const tree_data_type &rhs) const;

    // endregion comparators declaration

    struct bptree_node_base {
        bool _is_terminate;

        bptree_node_base() noexcept;

        virtual ~bptree_node_base() = default;
    };

    struct bptree_node_term : public bptree_node_base {
        bptree_node_term *_next;

        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;

        bptree_node_term() noexcept;
    };

    struct bptree_node_middle : public bptree_node_base {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bptree_node_base *, maximum_keys_in_node + 2> _pointers;

        bptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    logger *_logger;
    bptree_node_base *_root;
    size_t _size;

    logger *get_logger() const noexcept override;

    pp_allocator<value_type> get_allocator() const noexcept;

public:
    // region constructors declaration

    explicit BP_tree(const compare &cmp = compare(), pp_allocator<value_type>  = pp_allocator<value_type>(),
                     logger *logger = nullptr);

    explicit BP_tree(pp_allocator<value_type> alloc, const compare &comp = compare(), logger *logger = nullptr);

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BP_tree(iterator begin, iterator end, const compare &cmp = compare(),
                     pp_allocator<value_type>  = pp_allocator<value_type>(), logger *logger = nullptr);

    BP_tree(std::initializer_list<std::pair<tkey, tvalue> > data, const compare &cmp = compare(),
            pp_allocator<value_type>  = pp_allocator<value_type>(), logger *logger = nullptr);

    // endregion constructors declaration

    // region five declaration

    BP_tree(const BP_tree &other);

    BP_tree(BP_tree &&other) noexcept;

    BP_tree &operator=(const BP_tree &other);

    BP_tree &operator=(BP_tree &&other) noexcept;

    ~BP_tree() noexcept override;

    // endregion five declaration

    // region iterators declaration

    class bptree_iterator;
    class bptree_const_iterator;

    class bptree_iterator final {
        bptree_node_term *_node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type &;
        using pointer = value_type *;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_iterator;

        friend class BP_tree;
        friend class bptree_const_iterator;

        reference operator*() const noexcept;

        pointer operator->() const noexcept;

        self &operator++();

        self operator++(int);

        bool operator==(const self &other) const noexcept;

        bool operator!=(const self &other) const noexcept;

        size_t current_node_keys_count() const noexcept;

        size_t index() const noexcept;

        explicit bptree_iterator(bptree_node_term *node = nullptr, size_t index = 0);
    };

    class bptree_const_iterator final {
        bptree_node_term *_node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = const value_type &;
        using pointer = const value_type *;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_const_iterator;

        friend class BP_tree;
        friend class bptree_iterator;

        bptree_const_iterator(const bptree_iterator &it) noexcept;

        reference operator*() const noexcept;

        pointer operator->() const noexcept;

        self &operator++();

        self operator++(int);

        bool operator==(const self &other) const noexcept;

        bool operator!=(const self &other) const noexcept;

        size_t current_node_keys_count() const noexcept;

        size_t index() const noexcept;

        explicit bptree_const_iterator(bptree_node_term *node = nullptr, size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue &at(const tkey &);

    const tvalue &at(const tkey &) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue &operator[](const tkey &key);

    tvalue &operator[](tkey &&key);

    // endregion element access declaration
    // region iterator begins declaration

    bptree_iterator begin();

    bptree_iterator end();

    bptree_const_iterator begin() const;

    bptree_const_iterator end() const;

    bptree_const_iterator cbegin() const;

    bptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;

    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bptree_iterator find(const tkey &key);

    bptree_const_iterator find(const tkey &key) const;

    bptree_iterator lower_bound(const tkey &key);

    bptree_const_iterator lower_bound(const tkey &key) const;

    bptree_iterator upper_bound(const tkey &key);

    bptree_const_iterator upper_bound(const tkey &key) const;

    bool contains(const tkey &key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bptree_iterator, bool> insert(const tree_data_type &data);

    std::pair<bptree_iterator, bool> insert(tree_data_type &&data);

    template<typename... Args>
    std::pair<bptree_iterator, bool> emplace(Args &&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bptree_iterator insert_or_assign(const tree_data_type &data);

    bptree_iterator insert_or_assign(tree_data_type &&data);

    template<typename... Args>
    bptree_iterator emplace_or_assign(Args &&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bptree_iterator erase(bptree_iterator pos);

    bptree_iterator erase(bptree_const_iterator pos);

    bptree_iterator erase(bptree_iterator beg, bptree_iterator en);

    bptree_iterator erase(bptree_const_iterator beg, bptree_const_iterator en);


    bptree_iterator erase(const tkey &key);

    // endregion modifiers declaration

private:
    bptree_node_term *find_leaf(const tkey &key) const;

    void split_node(bptree_node_term *node);

    void split_internal_node(bptree_node_middle *node);

    void rebalance_after_delete(bptree_node_term *node);

    bptree_node_middle *find_parent(bptree_node_base *root, bptree_node_base *node);

    void rebalance_internal_after_delete(bptree_node_middle *node);

    void clear_recursive(bptree_node_base *node) noexcept;
};

template<std::input_iterator iterator, compator<typename std::iterator_traits<iterator>::value_type::first_type> compare
    = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
    std::size_t t = 5, typename U>
BP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U>  = pp_allocator<U>(),
        logger *logger = nullptr) -> BP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename
    std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BP_tree(std::initializer_list<std::pair<tkey, tvalue> > data, const compare &cmp = compare(),
        pp_allocator<U>  = pp_allocator<U>(),
        logger *logger = nullptr) -> BP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_pairs(const BP_tree::tree_data_type &lhs,
                                                      const BP_tree::tree_data_type &rhs) const {
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_base::bptree_node_base() noexcept {
    _is_terminate = false;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_term::bptree_node_term() noexcept {
    this->_is_terminate = true;
    _next = nullptr;
    _data = boost::container::static_vector<tree_data_type, maximum_keys_in_node>();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_middle::bptree_node_middle() noexcept {
    this->_is_terminate = false;
    _keys = boost::container::static_vector<tkey, maximum_keys_in_node>();
    _pointers = boost::container::static_vector<bptree_node_base *, maximum_keys_in_node + 1>();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
logger *BP_tree<tkey, tvalue, compare, t>::get_logger() const noexcept {
    return _logger;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::value_type> BP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept {
    return _allocator;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_iterator::operator*() const noexcept {
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::pointer BP_tree<tkey, tvalue, compare, t>::bptree_iterator
::operator->() const noexcept {
    return reinterpret_cast<pointer>(&(_node->_data[_index]));
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self &BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++() {
    if (_node == nullptr) {
        return *this;
    }

    if (_index + 1 < _node->_data.size()) {
        ++_index;
    } else {
        _node = _node->_next;
        _index = 0;
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator++(int) {
    self temp = *this; // Сохраняем текущее состояние
    ++(*this);         // Вызываем префиксный инкремент
    return temp;       // Возвращаем сохраненное состояние
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator==(const self &other) const noexcept {
    // Итераторы равны, если указывают на один и тот же узел и имеют одинаковый индекс
    // Также учитываем особый случай для end() итератора (nullptr)
    return _node == other._node && (_node == nullptr || _index == other._index);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator!=(const self &other) const noexcept {
    // Отрицание результата оператора равенства
    return !(*this == other);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::current_node_keys_count() const noexcept {
    // Возвращаем размер вектора данных в текущем узле, или 0, если узел nullptr
    if (_node == nullptr) {
        return 0;
    }
    return _node->_data.size();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::index() const noexcept {
    return _index;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::bptree_iterator(bptree_node_term *node, size_t index) {
    _node = node;
    _index = index;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_iterator &it) noexcept {
    _node = it._node;
    _index = it._index;
}
template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator*() const noexcept {
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::pointer BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator->() const noexcept {
    return reinterpret_cast<pointer>(&(_node->_data[_index]));
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self &BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++() {
    if (_node == nullptr) {
        return *this;
    }

    if (_index + 1 < _node->_data.size()) {
        ++_index;
    } else {
        _node = _node->_next;
        _index = 0;
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++(int) {
    auto copy = *this;
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator==(const self &other) const noexcept {
    // Итераторы равны, если указывают на один и тот же узел и имеют одинаковый индекс
    // Также учитываем особый случай для end() итератора (nullptr)
    return _node == other._node && (_node == nullptr || _index == other._index);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator!=(const self &other) const noexcept {
    // Отрицание результата оператора равенства
    return !(*this == other);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::current_node_keys_count() const noexcept {
    // Возвращаем размер вектора данных в текущем узле, или 0, если узел nullptr
    if (_node == nullptr) {
        return 0;
    }
    return _node->_data.size();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::index() const noexcept {
    return _index;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(bptree_node_term *node, size_t index) {
    _node = node;
    _index = index;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
tvalue &BP_tree<tkey, tvalue, compare, t>::at(const tkey &key) {
    bptree_iterator it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found in B+ tree");
    }
    // Возвращаем ссылку на значение
    return it->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
const tvalue &BP_tree<tkey, tvalue, compare, t>::at(const tkey &key) const {
    bptree_const_iterator it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found in B+ tree");
    }
    // Возвращаем константную ссылку на значение
    return it->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
tvalue &BP_tree<tkey, tvalue, compare, t>::operator[](const tkey &key) {
    bptree_iterator it = find(key);

    // Если ключ найден, возвращаем значение
    if (it != end()) {
        return it->second;
    }

    // Иначе вставляем новый элемент со значением по умолчанию
    auto result = insert(std::make_pair(key, tvalue()));
    // Возвращаем ссылку на новое значение
    return result.first->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
tvalue &BP_tree<tkey, tvalue, compare, t>::operator[](tkey &&key) {
    // Копируем ключ, так как он будет перемещен
    tkey key_copy = key;

    // Ищем элемент
    auto it = find(key_copy);
    if (it != end()) {
        return it->second;
    }

    // Вставляем новый элемент
    insert(std::make_pair(std::forward<tkey>(key), tvalue()));

    // Повторный поиск гарантирует валидность итератора
    it = find(key_copy);
    return it->second;
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_middle *
BP_tree<tkey, tvalue, compare, t>::find_parent(bptree_node_base *root, bptree_node_base *node) {
    if (root == nullptr || root == node || root->_is_terminate) {
        return nullptr;
    }

    bptree_node_middle *middle_node = static_cast<bptree_node_middle *>(root);

    // Проверяем прямых потомков
    for (size_t i = 0; i < middle_node->_pointers.size(); ++i) {
        if (middle_node->_pointers[i] == node) {
            return middle_node;
        }
    }

    // Рекурсивный поиск в подузлах
    for (size_t i = 0; i < middle_node->_pointers.size(); ++i) {
        if (!middle_node->_pointers[i]->_is_terminate) {
            bptree_node_middle *result = find_parent(middle_node->_pointers[i], node);
            if (result != nullptr) {
                return result;
            }
        }
    }

    return nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_node(bptree_node_term *node) {
    // Создаем новый узел
    bptree_node_term *new_node = get_allocator().template new_object<bptree_node_term>();

    // Находим середину
    size_t mid = t; // В конспекте указывается t как середина
    tkey mid_key = node->_data[mid].first;

    // Перемещаем данные из правой части в новый узел
    for (size_t i = mid; i < node->_data.size(); ++i) {
        new_node->_data.push_back(node->_data[i]);
    }

    // Обновляем размер исходного узла
    node->_data.resize(mid);

    // Обновляем указатели следующий/предыдущий
    new_node->_next = node->_next;
    node->_next = new_node;

    // Если это корень, создаем новый корень
    if (node == _root) {
        bptree_node_middle *new_root = get_allocator().template new_object<bptree_node_middle>();
        new_root->_keys.push_back(new_node->_data[0].first);
        new_root->_pointers.push_back(node);
        new_root->_pointers.push_back(new_node);
        _root = new_root;
    } else {
        // Находим родителя и обновляем его
        bptree_node_middle *parent = find_parent(_root, node);

        // Находим позицию для нового ключа
        size_t pos = 0;
        while (pos < parent->_keys.size() && compare_keys(parent->_keys[pos], mid_key)) {
            ++pos;
        }

        // Вставляем ключ и указатель
        parent->_keys.insert(parent->_keys.begin() + pos, mid_key);
        parent->_pointers.insert(parent->_pointers.begin() + pos + 1, new_node);

        // Проверяем, нужно ли разделить родительский узел
        if (parent->_keys.size() > maximum_keys_in_node) {
            split_internal_node(parent);
        }
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_internal_node(bptree_node_middle *node) {
    // Создаем новый узел
    bptree_node_middle *new_node = get_allocator().template new_object<bptree_node_middle>();

    // Находим середину
    size_t mid = node->_keys.size() / 2;
    tkey mid_key = node->_keys[mid];

    // Перемещаем ключи и указатели в новый узел
    for (size_t i = mid + 1; i < node->_keys.size(); ++i) {
        new_node->_keys.push_back(node->_keys[i]);
    }

    for (size_t i = mid + 1; i <= node->_keys.size(); ++i) {
        new_node->_pointers.push_back(node->_pointers[i]);
    }

    // Обновляем исходный узел
    node->_keys.resize(mid);
    node->_pointers.resize(mid + 1);

    // Если это корень, создаем новый корень
    if (node == _root) {
        bptree_node_middle *new_root = get_allocator().template new_object<bptree_node_middle>();
        new_root->_keys.push_back(mid_key);
        new_root->_pointers.push_back(node);
        new_root->_pointers.push_back(new_node);
        _root = new_root;
    } else {
        // Находим родителя и обновляем его
        bptree_node_middle *parent = find_parent(_root, node);

        // Находим позицию для нового ключа
        size_t pos = 0;
        while (pos < parent->_keys.size() && compare_keys(parent->_keys[pos], mid_key)) {
            ++pos;
        }

        // Вставляем ключ и указатель
        parent->_keys.insert(parent->_keys.begin() + pos, mid_key);
        parent->_pointers.insert(parent->_pointers.begin() + pos + 1, new_node);

        // Проверяем, нужно ли разделить родительский узел
        if (parent->_keys.size() > maximum_keys_in_node) {
            split_internal_node(parent);
        }
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(
    const tree_data_type &data) {
    // Если дерево пустое, создаем корень
    if (_root == nullptr) {
        _root = get_allocator().template new_object<bptree_node_term>();
        static_cast<bptree_node_term *>(_root)->_data.push_back(data);
        _size = 1;
        return {bptree_iterator(static_cast<bptree_node_term *>(_root), 0), true};
    }

    // Находим лист для вставки
    bptree_node_term *leaf = find_leaf(data.first);

    // Проверяем, существует ли уже ключ
    for (size_t i = 0; i < leaf->_data.size(); ++i) {
        if (!compare_keys(leaf->_data[i].first, data.first) && !compare_keys(data.first, leaf->_data[i].first)) {
            return {bptree_iterator(leaf, i), false};
        }
    }

    // Находим позицию для вставки
    size_t pos = 0;
    while (pos < leaf->_data.size() && compare_keys(leaf->_data[pos].first, data.first)) {
        ++pos;
    }

    // Вставляем пару ключ-значение
    leaf->_data.insert(leaf->_data.begin() + pos, data);
    ++_size;

    // Проверяем, нужно ли разделить узел
    if (leaf->_data.size() > maximum_keys_in_node) {
        split_node(leaf);
    }

    return {bptree_iterator(leaf, pos), true};
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const {
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare,
    t>::BP_tree(const compare &cmp, pp_allocator<value_type> alloc, logger *logger) : compare(cmp), _allocator(alloc),
    _logger(logger), _root(nullptr), _size(0) {
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare,
    t>::BP_tree(pp_allocator<value_type> alloc, const compare &cmp, logger *logger) : compare(cmp), _allocator(alloc),
    _logger(logger), _root(nullptr), _size(0) {
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BP_tree<tkey, tvalue, compare, t>::BP_tree(iterator begin, iterator end, const compare &cmp,
                                           pp_allocator<value_type> alloc, logger *logger)
    : _allocator(alloc), _root(nullptr), _logger(logger) {

    for (auto it = begin; it != end; ++it) {
        insert_or_assign(*it);
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(std::initializer_list<std::pair<tkey, tvalue> > data, const compare &cmp,
                                           pp_allocator<value_type> alloc, logger *logger) : compare(cmp),
    _allocator(alloc), _logger(logger), _root(nullptr), _size(0) {
    for (auto it = data.begin(); it != data.end(); ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const BP_tree &other)
    : _allocator(other._allocator), _root(nullptr) {

    for (const auto& item : other) {
        insert_or_assign(item);
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(BP_tree &&other) noexcept
    : _allocator(std::move(other._allocator)), _root(other._root) {

    other._root = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t> &BP_tree<tkey, tvalue, compare, t>::operator=(const BP_tree &other) {
    if (this == &other) {
        return *this;
    }

    if (_root) {
        clear_recursive(_root);
    }


    _allocator = other._allocator;

    for (const auto& item : other) {
        insert_or_assign(item);
    }

    return *this;
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t> &BP_tree<tkey, tvalue, compare, t>::operator=(BP_tree &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (_root) {
        clear_recursive(_root);
    }


    _allocator = std::move(other._allocator);
    _root = other._root;

    other._root = nullptr;

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::~BP_tree() noexcept {
    clear();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::begin() {
    if (empty()) {
        return end();
    }

    // Находим самый левый лист (минимальный ключ)
    bptree_node_base *current = _root;

    // Спускаемся вниз по левым указателям до листового узла
    while (!current->_is_terminate) {
        bptree_node_middle *middle_node = static_cast<bptree_node_middle *>(current);
        current = middle_node->_pointers[0];
    }

    return bptree_iterator(static_cast<bptree_node_term *>(current), 0);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::end() {
    // end() указывает на позицию за последним элементом (nullptr)
    return bptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::begin() const {
    // Используем ту же логику, что и в неконстантном begin(), но возвращаем константный итератор
    if (empty()) {
        return end();
    }

    bptree_node_base *current = _root;

    while (!current->_is_terminate) {
        bptree_node_middle *middle_node = static_cast<bptree_node_middle *>(current);
        current = middle_node->_pointers[0];
    }

    return bptree_const_iterator(static_cast<bptree_node_term *>(current), 0);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::end() const {
    // Возвращаем константный итератор на позицию за последним элементом
    return bptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cbegin() const {
    // cbegin() просто делегирует вызов константному begin()
    return begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cend() const {
    // cend() просто делегирует вызов константному end()
    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::size() const noexcept {
    return _size;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::empty() const noexcept {
    return _size == 0;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey &key) {
    if (empty()) {
        return end();
    }

    bptree_node_term *leaf = find_leaf(key);

    // Ищем ключ в листе
    for (size_t i = 0; i < leaf->_data.size(); ++i) {
        // Проверяем на эквивалентность (не меньше и не больше)
        if (!compare_keys(leaf->_data[i].first, key) && !compare_keys(key, leaf->_data[i].first)) {
            return bptree_iterator(leaf, i);
        }
    }

    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::find(
    const tkey &key) const {
    if (empty()) {
        return end();
    }

    bptree_node_term *leaf = find_leaf(key);

    // Ищем ключ в листе
    for (size_t i = 0; i < leaf->_data.size(); ++i) {
        if (!compare_keys(leaf->_data[i].first, key) && !compare_keys(key, leaf->_data[i].first)) {
            return bptree_const_iterator(leaf, i);
        }
    }

    return end();
}
template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::contains(const tkey &key) const {
    return find(key) != end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey &key) {
    if (empty()) {
        return end();
    }

    auto it = begin();
    while (it != end() && compare_keys(it->first, key)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator
BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey &key) const {
    if (empty()) {
        return end();
    }

    auto it = begin();
    while (it != end() && compare_keys(it->first, key)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey &key) {
    if (empty()) {
        return end();
    }

    auto it = begin();
    while (it != end() && !compare_keys(key, it->first)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator
BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey &key) const {
    if (empty()) {
        return end();
    }

    auto it = begin();
    while (it != end() && !compare_keys(key, it->first)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear_recursive(bptree_node_base *node) noexcept {
    if (node == nullptr) {
        return;
    }

    // Если это внутренний узел, сначала освобождаем всех его потомков
    if (!node->_is_terminate) {
        bptree_node_middle *middle_node = static_cast<bptree_node_middle *>(node);

        // Рекурсивно освобождаем память для всех потомков
        for (size_t i = 0; i < middle_node->_pointers.size(); ++i) {
            clear_recursive(middle_node->_pointers[i]);
        }

        // Удаляем внутренний узел
        get_allocator().template delete_object<bptree_node_middle>(middle_node);
    } else {
        // Удаляем листовой узел
        get_allocator().template delete_object<bptree_node_term>(static_cast<bptree_node_term *>(node));
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear() noexcept {
    // Вызываем вспомогательный метод для рекурсивного удаления узлов
    if (_root != nullptr) {
        clear_recursive(_root);
        _root = nullptr;
        _size = 0;
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term *
BP_tree<tkey, tvalue, compare, t>::find_leaf(const tkey &key) const {
    if (_root == nullptr) {
        return nullptr;
    }

    bptree_node_base *cur = _root;

    // Спускаемся до листа, как описано в конспекте
    while (!cur->_is_terminate) {
        auto *middle_node = static_cast<bptree_node_middle *>(cur);
        size_t i;

        for (i = 0; i < middle_node->_keys.size(); ++i) {
            if (compare_keys(key, middle_node->_keys[i])) {
                break;
            }
        }

        cur = middle_node->_pointers[i];
    }

    return static_cast<bptree_node_term *>(cur);
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(
    tree_data_type &&data) {
    if (_root == nullptr) {
        _root = get_allocator().template new_object<bptree_node_term>();
        static_cast<bptree_node_term*>(_root)->_data.push_back(std::move(data));
        _size = 1;
        return {bptree_iterator(static_cast<bptree_node_term*>(_root), 0), true};
    }

    bptree_node_term* leaf = find_leaf(data.first);

    for (size_t i = 0; i < leaf->_data.size(); ++i) {
        if (!compare_keys(leaf->_data[i].first, data.first) && !compare_keys(data.first, leaf->_data[i].first)) {
            return {bptree_iterator(leaf, i), false};
        }
    }

    size_t pos = 0;
    while (pos < leaf->_data.size() && compare_keys(leaf->_data[pos].first, data.first)) {
        ++pos;
    }

    leaf->_data.insert(leaf->_data.begin() + pos, std::move(data));
    ++_size;

    if (leaf->_data.size() > maximum_keys_in_node) {
        split_node(leaf);
    }

    return {bptree_iterator(leaf, pos), true};
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare,
    t>::emplace(Args &&... args) {
    tree_data_type new_data(std::forward<Args>(args)...);

    return insert(std::move(new_data));
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type &data) {
    if (!_root) {
        // Создаем корневой узел, если дерево пустое
        _root = get_allocator().template new_object<bptree_node_term>();
        _root->_is_terminate = true;
        static_cast<bptree_node_term*>(_root)->_next = nullptr;
    }

    // Находим листовой узел для вставки/обновления
    bptree_node_term *leaf = find_leaf(data.first);

    // Ищем позицию в листе
    auto &leaf_data = leaf->_data;
    size_t pos = 0;
    while (pos < leaf_data.size() && compare_keys(leaf_data[pos].first, data.first)) {
        ++pos;
    }

    // Если ключ уже существует - обновляем значение
    if (pos < leaf_data.size() && keys_equal(leaf_data[pos].first, data.first)) {
        leaf_data[pos].second = data.second;
        return bptree_iterator(leaf, pos);
    }

    // Вставляем новый элемент
    leaf_data.insert(leaf_data.begin() + pos, data);

    // Проверяем переполнение и разделяем при необходимости
    if (leaf_data.size() > maximum_keys_in_node) {
        split_leaf_node(leaf);
    }

    return bptree_iterator(leaf, pos);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type &&data) {
    if (!_root) {
        _root = get_allocator().template new_object<bptree_node_term>();
        _root->_is_terminate = true;
        static_cast<bptree_node_term*>(_root)->_next = nullptr;
    }

    bptree_node_term *leaf = find_leaf(data.first);
    auto &leaf_data = leaf->_data;
    size_t pos = 0;

    while (pos < leaf_data.size() && compare_keys(leaf_data[pos].first, data.first)) {
        ++pos;
    }

    if (pos < leaf_data.size() && keys_equal(leaf_data[pos].first, data.first)) {
        leaf_data[pos].second = std::move(data.second);
        return bptree_iterator(leaf, pos);
    }

    leaf_data.insert(leaf_data.begin() + pos, std::move(data));

    if (leaf_data.size() > maximum_keys_in_node) {
        split_leaf_node(leaf);
    }

    return bptree_iterator(leaf, pos);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
template<typename... Args>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args &&... args) {
    tree_data_type data(std::forward<Args>(args)...);

    if (!_root) {
        _root = get_allocator().template new_object<bptree_node_term>();
        _root->_is_terminate = true;
        static_cast<bptree_node_term*>(_root)->_next = nullptr;
    }

    bptree_node_term *leaf = find_leaf(data.first);
    auto &leaf_data = leaf->_data;
    size_t pos = 0;

    while (pos < leaf_data.size() && compare_keys(leaf_data[pos].first, data.first)) {
        ++pos;
    }

    if (pos < leaf_data.size() && keys_equal(leaf_data[pos].first, data.first)) {
        leaf_data[pos].second = std::move(data.second);
        return bptree_iterator(leaf, pos);
    }

    leaf_data.emplace(leaf_data.begin() + pos, std::move(data));

    if (leaf_data.size() > maximum_keys_in_node) {
        split_leaf_node(leaf);
    }

    return bptree_iterator(leaf, pos);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator pos) {
    if (pos == end()) {
        return end();
    }

    bptree_node_term *node = pos._node;
    size_t index = pos._index;

    // Удаляем элемент
    node->_data.erase(node->_data.begin() + index);

    // Сохраняем следующую позицию для возврата
    bptree_iterator next_pos = pos;
    ++next_pos;

    // Проверяем нужна ли ребалансировка
    if (node->_data.size() < minimum_keys_in_node && node != _root) {
        rebalance_after_delete(node);
    }

    return next_pos;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator pos) {
    bptree_iterator mutable_pos(const_cast<bptree_node_term*>(pos._node), pos._index);
    return erase(mutable_pos);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator beg, bptree_iterator en) {
    bptree_iterator current = beg;

    while (current != en) {
        current = erase(current);  // erase возвращает следующий итератор
    }

    return current;
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator beg, bptree_const_iterator en) {
    bptree_iterator mutable_beg(const_cast<bptree_node_term*>(beg._node), beg._index);
    bptree_iterator mutable_en(const_cast<bptree_node_term*>(en._node), en._index);

    return erase(mutable_beg, mutable_en);
}


template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::rebalance_internal_after_delete(bptree_node_middle *node) {
    // Находим родителя узла
    bptree_node_middle *parent = find_parent(_root, node);

    // Если это корень, проверяем особый случай
    if (node == _root) {
        // Если у корня остался только один дочерний узел, делаем его новым корнем
        if (node->_keys.size() == 0 && node->_pointers.size() == 1) {
            _root = node->_pointers[0];
            get_allocator().template delete_object<bptree_node_middle>(node);
        }
        return;
    }

    // Проверяем, является ли узел дефицитным (меньше minimum_keys_in_node ключей)
    if (node->_keys.size() >= minimum_keys_in_node) {
        return; // Узел не дефицитный, ничего делать не нужно
    }

    // Находим индекс узла в родителе
    size_t child_idx = 0;
    while (child_idx < parent->_pointers.size() && parent->_pointers[child_idx] != node) {
        ++child_idx;
    }

    // Пробуем заимствовать у левого соседа
    if (child_idx > 0) {
        bptree_node_middle *left_sibling = static_cast<bptree_node_middle *>(parent->_pointers[child_idx - 1]);

        if (left_sibling->_keys.size() > minimum_keys_in_node) {

            node->_keys.insert(node->_keys.begin(), parent->_keys[child_idx - 1]);

            node->_pointers.insert(node->_pointers.begin(), left_sibling->_pointers.back());
            left_sibling->_pointers.pop_back();

            parent->_keys[child_idx - 1] = left_sibling->_keys.back();
            left_sibling->_keys.pop_back();

            return;
        }
    }

    // Пробуем заимствовать у правого соседа
    if (child_idx < parent->_pointers.size() - 1) {
        bptree_node_middle *right_sibling = static_cast<bptree_node_middle *>(parent->_pointers[child_idx + 1]);

        if (right_sibling->_keys.size() > minimum_keys_in_node) {
            // Перемещаем разделитель из родителя в конец текущего узла
            node->_keys.push_back(parent->_keys[child_idx]);

            // Перемещаем первый указатель из правого соседа в конец текущего узла
            node->_pointers.push_back(right_sibling->_pointers.front());
            right_sibling->_pointers.erase(right_sibling->_pointers.begin());

            // Заменяем разделитель в родителе на первый ключ правого соседа
            parent->_keys[child_idx] = right_sibling->_keys.front();
            right_sibling->_keys.erase(right_sibling->_keys.begin());

            return;
        }
    }

    // Если не можем заимствовать, объединяем с одним из соседей

    // Объединяем с левым соседом, если он существует
    if (child_idx > 0) {
        bptree_node_middle *left_sibling = static_cast<bptree_node_middle *>(parent->_pointers[child_idx - 1]);

        // Добавляем разделитель из родителя в конец левого соседа
        left_sibling->_keys.push_back(parent->_keys[child_idx - 1]);

        // Перемещаем все ключи и указатели из текущего узла в левый сосед
        for (size_t i = 0; i < node->_keys.size(); ++i) {
            left_sibling->_keys.push_back(node->_keys[i]);
        }

        for (size_t i = 0; i < node->_pointers.size(); ++i) {
            left_sibling->_pointers.push_back(node->_pointers[i]);
        }

        // Удаляем разделитель и указатель на текущий узел из родителя
        parent->_keys.erase(parent->_keys.begin() + child_idx - 1);
        parent->_pointers.erase(parent->_pointers.begin() + child_idx);

        get_allocator().template delete_object<bptree_node_middle>(node);

        if (parent != _root && parent->_keys.size() < minimum_keys_in_node) {
            rebalance_internal_after_delete(parent);
        }

        if (parent == _root && parent->_keys.empty()) {
            _root = parent->_pointers[0];
            get_allocator().template delete_object<bptree_node_middle>(parent);
        }
    }
    // Иначе объединяем с правым соседом
    else {
        bptree_node_middle *right_sibling = static_cast<bptree_node_middle *>(parent->_pointers[child_idx + 1]);

        // Добавляем разделитель из родителя в конец текущего узла
        node->_keys.push_back(parent->_keys[child_idx]);

        // Перемещаем все ключи и указатели из правого соседа в текущий узел
        for (size_t i = 0; i < right_sibling->_keys.size(); ++i) {
            node->_keys.push_back(right_sibling->_keys[i]);
        }

        for (size_t i = 0; i < right_sibling->_pointers.size(); ++i) {
            node->_pointers.push_back(right_sibling->_pointers[i]);
        }

        // Удаляем разделитель и указатель на правого соседа из родителя
        parent->_keys.erase(parent->_keys.begin() + child_idx);
        parent->_pointers.erase(parent->_pointers.begin() + child_idx + 1);

        get_allocator().template delete_object<bptree_node_middle>(right_sibling);

        if (parent != _root && parent->_keys.size() < minimum_keys_in_node) {
            rebalance_internal_after_delete(parent);
        }

        if (parent == _root && parent->_keys.empty()) {
            _root = parent->_pointers[0];
            get_allocator().template delete_object<bptree_node_middle>(parent);
        }
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::rebalance_after_delete(bptree_node_term *node) {
    bptree_node_middle *parent = find_parent(_root, node);

    // Находим индекс узла в родителе
    size_t child_idx = 0;
    while (child_idx < parent->_pointers.size() && parent->_pointers[child_idx] != node) {
        ++child_idx;
    }

    // Пробуем взять у левого соседа
    if (child_idx > 0) {
        bptree_node_term *left_sibling = static_cast<bptree_node_term *>(parent->_pointers[child_idx - 1]);

        if (left_sibling->_data.size() > minimum_keys_in_node) {
            // Перемещаем последний элемент из левого соседа в текущий узел
            node->_data.insert(node->_data.begin(), left_sibling->_data.back());
            left_sibling->_data.pop_back();

            // Обновляем ключ в родителе
            parent->_keys[child_idx - 1] = node->_data.front().first;

            return;
        }
    }

    // Пробуем взять у правого соседа
    if (child_idx < parent->_pointers.size() - 1) {
        bptree_node_term *right_sibling = static_cast<bptree_node_term *>(parent->_pointers[child_idx + 1]);

        if (right_sibling->_data.size() > minimum_keys_in_node) {
            // Перемещаем первый элемент из правого соседа в текущий узел
            node->_data.push_back(right_sibling->_data.front());
            right_sibling->_data.erase(right_sibling->_data.begin());

            // Обновляем ключ в родителе
            parent->_keys[child_idx] = right_sibling->_data.front().first;

            return;
        }
    }

    // Если не можем взять, объединяем с соседом
    if (child_idx > 0) {
        // Объединяем с левым соседом
        bptree_node_term *left_sibling = static_cast<bptree_node_term *>(parent->_pointers[child_idx - 1]);

        // Перемещаем все данные в левого соседа
        for (const auto &data: node->_data) {
            left_sibling->_data.push_back(data);
        }

        left_sibling->_next = node->_next;

        // Удаляем ключ и указатель из родителя
        parent->_keys.erase(parent->_keys.begin() + child_idx - 1);
        parent->_pointers.erase(parent->_pointers.begin() + child_idx);

        get_allocator().template delete_object<bptree_node_term>(node);

        if (parent != _root && parent->_keys.size() < minimum_keys_in_node) {
            rebalance_internal_after_delete(parent);
        }
    } else {
        // Объединяем с правым соседом
        bptree_node_term *right_sibling = static_cast<bptree_node_term *>(parent->_pointers[child_idx + 1]);

        // Перемещаем все данные из правого соседа в текущий узел
        for (const auto &data: right_sibling->_data) {
            node->_data.push_back(data);
        }

        node->_next = right_sibling->_next;

        // Удаляем ключ и указатель из родителя
        parent->_keys.erase(parent->_keys.begin() + child_idx);
        parent->_pointers.erase(parent->_pointers.begin() + child_idx + 1);

        get_allocator().template delete_object<bptree_node_term>(right_sibling);

        if (parent != _root && parent->_keys.size() < minimum_keys_in_node) {
            rebalance_internal_after_delete(parent);
        }
    }

    // Если корень стал пустым, удаляем его
    if (parent == _root && parent->_keys.empty()) {
        _root = parent->_pointers[0];
        _root = parent->_pointers[0];
        get_allocator().template delete_object<bptree_node_middle>(parent);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(const tkey &key) {
    if (empty()) {
        return end();
    }
    bptree_node_term *leaf = find_leaf(key);

    size_t pos = 0;
    bool found = false;

    for (size_t i = 0; i < leaf->_data.size(); ++i) {
        if (!compare_keys(leaf->_data[i].first, key) && !compare_keys(key, leaf->_data[i].first)) {
            pos = i;
            found = true;
            break;
        }
    }

    // Если ключ не найден, возвращаем end()
    if (!found) {
        return end();
    }

    // Получаем следующую позицию итератора
    bptree_iterator next_it;
    if (pos + 1 < leaf->_data.size()) {
        next_it = bptree_iterator(leaf, pos + 1);
    } else if (leaf->_next != nullptr) {
        next_it = bptree_iterator(leaf->_next, 0);
    } else {
        next_it = end();
    }

    // Удаляем ключ
    leaf->_data.erase(leaf->_data.begin() + pos);
    --_size;

    // Проверяем, нужно ли ребалансировать дерево
    if (leaf != _root && leaf->_data.size() < minimum_keys_in_node) {
        rebalance_after_delete(leaf);
    }

    return next_it;
}

#endif
