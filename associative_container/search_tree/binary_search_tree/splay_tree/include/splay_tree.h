#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SPLAY_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SPLAY_TREE_H

#include <binary_search_tree.h>

namespace __detail
{
    class SPL_TAG;

    template<typename tkey, typename tvalue, typename compare>
    class bst_impl<tkey, tvalue, compare, SPL_TAG>
    {
        friend class binary_search_tree<tkey, tvalue, compare, SPL_TAG>;
        template<class ...Args>
        static binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node* create_node(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, Args&& ...args);

        static void delete_node(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node**);

        //Does not invalidate node*, needed for splay tree
        static void post_search(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
                                binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node**);

        //Does not invalidate node*
        static void post_insert(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node**);

        static void erase(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node**);

        static void swap(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& lhs, binary_search_tree<tkey, tvalue, compare, SPL_TAG>& rhs) noexcept;
        static void splay(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
                binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node**);
        static void replace_node_in_parent(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** , binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node* );
    };
}

template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>>
class splay_tree final: public binary_search_tree<tkey, tvalue, compare, __detail::SPL_TAG>
{

    using parent = binary_search_tree<tkey, tvalue, compare, __detail::SPL_TAG>;
    friend class __detail::bst_impl<tkey, tvalue, compare, __detail::SPL_TAG>;
public:

    using value_type = parent::value_type;

    explicit splay_tree(
            const compare& comp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger *logger = nullptr);

    explicit splay_tree(
            pp_allocator<value_type> alloc,
            const compare& comp = compare(),
            logger *logger = nullptr);

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit splay_tree(iterator begin, iterator end, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);

    template<std::ranges::input_range Range>
    explicit splay_tree(Range&& range, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);


    splay_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);

public:

    ~splay_tree() noexcept final = default;

    splay_tree(splay_tree const &other);

    splay_tree &operator=(splay_tree const &other);

    splay_tree(splay_tree &&other) noexcept = default;

    splay_tree &operator=(splay_tree &&other) noexcept = default;

};

template<typename compare, typename U, typename iterator>
explicit splay_tree(iterator begin, iterator end, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> splay_tree<const typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare>;

template<typename compare, typename U, std::ranges::forward_range Range>
explicit splay_tree(Range&& range, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> splay_tree<const typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::first_type, typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::second_type, compare> ;

template<typename tkey, typename tvalue, typename compare, typename U>
splay_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> splay_tree<tkey, tvalue, compare>;

// region implementation

template<typename tkey, typename tvalue, compator<tkey> compare>
splay_tree<tkey, tvalue, compare>::splay_tree(
        const compare& comp,
        pp_allocator<value_type> alloc,
        logger *logger) : parent(comp, alloc, logger)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
splay_tree<tkey, tvalue, compare>::splay_tree(
        pp_allocator<value_type> alloc,
        const compare& comp,
        logger *logger) : parent(alloc, comp, logger)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<input_iterator_for_pair<tkey, tvalue> iterator>
splay_tree<tkey, tvalue, compare>::splay_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger) : parent(cmp, alloc, logger)
{
    for (auto it = begin; it != end; ++it)
    {
        this->emplace(*it);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<std::ranges::input_range Range>
splay_tree<tkey, tvalue, compare>::splay_tree(
        Range&& range,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger) : parent(cmp, alloc, logger)
{
    for (auto &&element : range)
    {
        emplace(std::forward<decltype(element)>(element));
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
splay_tree<tkey, tvalue, compare>::splay_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger) : parent(cmp, alloc, logger)
{
    for (const auto &element : data)
    {
        emplace(element.first, element.second);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
splay_tree<tkey, tvalue, compare>::splay_tree(splay_tree const &other) : parent(other)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
splay_tree<tkey, tvalue, compare> &splay_tree<tkey, tvalue, compare>::operator=(splay_tree const &other)
{
    if (this != &other) {
        parent::operator=(other);
    }
    return *this;
}

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare>
    template<class ...Args>
    binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node* bst_impl<tkey, tvalue, compare, SPL_TAG>::create_node(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, Args&& ...args){
        using node_type = typename splay_tree<tkey, tvalue, compare>::node;
        auto *new_node = cont._allocator.template new_object<node_type>(std::forward<Args>(args)...);
        return new_node;
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::delete_node(
            binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** node){
        using node_type = typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node;
        if (node && *node)
        {
            cont._allocator.template delete_object<node_type>(*node);
            *node = nullptr;
        }
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::post_search(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
                     typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** node)
    {
        // После поиска надо поднять узел в корень
        // Если не нашли, то последний посещённый поднимаем
        if (node && *node)
        {
            splay(cont, node);
        }
    }

        //Does not invalidate node*
    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::post_insert(
            binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** node){
        //После вставки так же поднимаем узел в корень
        if (node && *node)
        {
            splay(cont, node);
        }
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::erase(
            binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** node_ptr){
        using node_type = typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node;
        if (node_ptr == nullptr || *node_ptr == nullptr)
            return;

        // 1. Поднимаем удаляемый узел в корень
        splay(cont, node_ptr);
        node_type* node_to_delete = *node_ptr;
        // Случай 1: Нет левого поддерева
        if (node_to_delete->left_subtree == nullptr)
        {
            replace_node_in_parent(cont, node_ptr, node_to_delete->right_subtree);
        }
            // Случай 2: Нет правого поддерева
        else if (node_to_delete->right_subtree == nullptr)
        {
            replace_node_in_parent(cont, node_ptr, node_to_delete->left_subtree);
        }
            // Случай 3: Есть оба поддерева
        else
        {

            // Находим максимальный узел в левом поддереве
            auto* max_left = node_to_delete->left_subtree;
            while (max_left->right_subtree != nullptr)
            {
                max_left = max_left->right_subtree;
            }

            // Поднимаем этот узел в корень левого поддерева
            auto* left_child = node_to_delete->left_subtree;
            if (left_child != max_left)
            {
                // Временное отсоединение левого поддерева
                node_to_delete->left_subtree = nullptr;
                max_left->parent = nullptr;

                // Поднимаем max_left в корень
                auto** max_ptr = &max_left;
                splay(cont, max_ptr);

                // Присоединяем правое поддерево удаляемого узла
                max_left->right_subtree = node_to_delete->right_subtree;
                if (node_to_delete->right_subtree)
                    node_to_delete->right_subtree->parent = max_left;

                // Заменяем удаляемый узел
                replace_node_in_parent(cont, node_ptr, max_left);
            }
            else
            {
                // Левый потомок уже является максимальным
                max_left->right_subtree = node_to_delete->right_subtree;
                node_to_delete->right_subtree->parent = max_left;
                replace_node_in_parent(cont, node_ptr, max_left);
            }
        }
        delete_node(cont, &node_to_delete);
        cont._size--;
    }


    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::swap(binary_search_tree<tkey, tvalue, compare, SPL_TAG>& lhs, binary_search_tree<tkey, tvalue, compare, SPL_TAG>& rhs) noexcept{
        using std::swap;
        swap(lhs.root, rhs.root);
        swap(lhs._logger, rhs._logger);
        swap(lhs._size, rhs._size);
        swap(lhs._allocator, rhs._allocator);
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::splay(
            binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** node_ptr)
    {
        if (node_ptr == nullptr || *node_ptr == nullptr || (*node_ptr)->parent == nullptr)
            return;

        auto node = *node_ptr;

        while (node->parent != nullptr)
        {
            // Zig
            if (node->parent->parent == nullptr)
            {
                if (node == node->parent->left_subtree)
                {
                    auto parent = node->parent;
                    cont.small_right_rotation(parent);
                }
                else
                {
                    auto parent = node->parent;
                    cont.small_left_rotation(parent);
                }
            }
            else
            {
                auto parent = node->parent;
                auto grandparent = parent->parent;

                // Zig-Zig (left-left or right-right)
                if (node == parent->left_subtree && parent == grandparent->left_subtree)
                {
                    cont.small_right_rotation(grandparent);
                    cont.small_right_rotation(parent);
                }
                else if (node == parent->right_subtree && parent == grandparent->right_subtree)
                {
                    cont.small_left_rotation(grandparent);
                    cont.small_left_rotation(parent);
                }
                    // Zig-Zag (left-right or right-left)
                else if (node == parent->right_subtree && parent == grandparent->left_subtree)
                {
                    cont.small_left_rotation(parent);
                    cont.small_right_rotation(grandparent);
                }
                else
                {
                    cont.small_right_rotation(parent);
                    cont.small_left_rotation(grandparent);
                }
            }
        }
        cont._root = node;
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, SPL_TAG>::replace_node_in_parent(
            binary_search_tree<tkey, tvalue, compare, SPL_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node** node_ptr,
            typename binary_search_tree<tkey, tvalue, compare, SPL_TAG>::node* new_node)
                                {
        if (node_ptr == nullptr || *node_ptr == nullptr)
            return;

        auto *old_node = *node_ptr;
        auto *parent = old_node->parent;

        if (parent == nullptr) {
            cont._root = new_node;
            if (new_node != nullptr)
                new_node->parent = nullptr;
        } else {
            if (parent->left_subtree == old_node)
                parent->left_subtree = new_node;
            else
                parent->right_subtree = new_node;

            if (new_node != nullptr)
                new_node->parent = parent;
        }

        *node_ptr = new_node;
    }
}


// endregion implementation

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SPLAY_TREE_H