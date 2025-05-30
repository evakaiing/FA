#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_RED_BLACK_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_RED_BLACK_TREE_H

#include <binary_search_tree.h>

namespace __detail
{
    class RB_TAG;

    template<typename tkey, typename tvalue, typename compare>
    class bst_impl<tkey, tvalue, compare, RB_TAG>

    {
        friend class binary_search_tree<tkey, tvalue, compare, RB_TAG>;
        template<class ...Args>
        static binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* create_node(binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont, Args&& ...args);

        static void delete_node(binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont, binary_search_tree<tkey, tvalue, compare, RB_TAG>::node**);

        //Does not invalidate node*, needed for splay tree
        static void post_search(binary_search_tree<tkey, tvalue, compare, RB_TAG>::node**){}

        //Does not invalidate node*
        static void post_insert(binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont, binary_search_tree<tkey, tvalue, compare, RB_TAG>::node**);

        static void erase(binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont, binary_search_tree<tkey, tvalue, compare, RB_TAG>::node**);

        static void fix_double_black(
                binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont,
                binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* node);

        static void replace_node(
                binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont,
                binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* old_node,
                binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* new_node);

        static void swap(binary_search_tree<tkey, tvalue, compare, RB_TAG>& lhs, binary_search_tree<tkey, tvalue, compare, RB_TAG>& rhs) noexcept;


    };
}

template<typename tkey,typename tvalue, compator<tkey> compare = std::less<tkey>>
class red_black_tree final: public binary_search_tree<tkey, tvalue, compare, __detail::RB_TAG>
{

public:

    enum class node_color : unsigned char
    {
        RED,
        BLACK
    };

private:

    using parent = binary_search_tree<tkey, tvalue, compare, __detail::RB_TAG>;
    friend class __detail::bst_impl<tkey, tvalue, compare, __detail::RB_TAG>;
    struct node final:
        parent::node
    {
        node_color color;

        template<class ...Args>
        node(parent::node* par, Args&&... args);
        explicit node(parent::node* par)
                : parent::node(par), color(node_color::BLACK)
        {
            this->left_subtree = this->right_subtree = nullptr;
        }

        ~node() noexcept override =default;
    };

public:

    void print_tree() const
    {
        if (this->_root == nullptr)
        {
            std::cout << "Tree is empty" << std::endl;
            return;
        }

        print_subtree(static_cast<node*>(this->_root), 0);
    }

private:

    void print_subtree(node* current, int depth) const
    {
        if (current == nullptr)
        {
            return;
        }
        print_subtree(static_cast<node*>(current->right_subtree), depth + 1);

        for (int i = 0; i < depth; ++i)
        {
            std::cout << "    ";
        }
        std::cout << current->data.first << "("
                  << (current->color == node_color::RED ? "R" : "B") << ")";

        if (current->parent != nullptr)
        {
            std::cout << " [parent: " << static_cast<node*>(current->parent)->data.first << "]";
        }
        std::cout << std::endl;
        print_subtree(static_cast<node*>(current->left_subtree), depth + 1);
    }

public:

    using value_type = parent::value_type;

    explicit red_black_tree(
            const compare& comp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger *logger = nullptr);

    explicit red_black_tree(
            pp_allocator<value_type> alloc,
            const compare& comp = compare(),
            logger *logger = nullptr);

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit red_black_tree(iterator begin, iterator end, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);

    template<std::ranges::input_range Range>
    explicit red_black_tree(Range&& range, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);


    red_black_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);


    // region iterator definition


    class prefix_iterator : public parent::prefix_iterator
    {
        friend binary_search_tree<tkey, tvalue, compare, __detail::RB_TAG>;
        friend class __detail::bst_impl<tkey, tvalue, compare, __detail::RB_TAG>;
    public:

        using value_type = parent::prefix_iterator::value_type;
        using difference_type = parent::prefix_iterator::difference_type;
        using pointer = parent::prefix_iterator::pointer;
        using reference = parent::prefix_iterator::reference;
        using iterator_category = parent::prefix_iterator::iterator_category;

        explicit prefix_iterator(parent::node* n = nullptr) noexcept;
        prefix_iterator(parent::prefix_iterator) noexcept;

        node_color get_color() const noexcept;

        using parent::prefix_iterator::depth;
        using parent::prefix_iterator::operator*;
        using parent::prefix_iterator::operator==;
        using parent::prefix_iterator::operator!=;
        using parent::prefix_iterator::operator++;
        using parent::prefix_iterator::operator--;
        using parent::prefix_iterator::operator->;
    };

    class prefix_const_iterator : public parent::prefix_const_iterator
    {
    public:

        using value_type = parent::prefix_const_iterator::value_type;
        using difference_type = parent::prefix_const_iterator::difference_type;
        using pointer = parent::prefix_const_iterator::pointer;
        using reference = parent::prefix_const_iterator::reference;
        using iterator_category = parent::prefix_const_iterator::iterator_category;

        explicit prefix_const_iterator(parent::node* n = nullptr) noexcept;
        prefix_const_iterator(parent::prefix_const_iterator) noexcept;

        node_color get_color() const noexcept;

        prefix_const_iterator(prefix_iterator) noexcept;

        using parent::prefix_const_iterator::depth;
        using parent::prefix_const_iterator::operator*;
        using parent::prefix_const_iterator::operator==;
        using parent::prefix_const_iterator::operator!=;
        using parent::prefix_const_iterator::operator++;
        using parent::prefix_const_iterator::operator--;
        using parent::prefix_const_iterator::operator->;
    };

    class prefix_reverse_iterator : public parent::prefix_reverse_iterator
    {
    public:

        using value_type = parent::prefix_reverse_iterator::value_type;
        using difference_type = parent::prefix_reverse_iterator::difference_type;
        using pointer = parent::prefix_reverse_iterator::pointer;
        using reference = parent::prefix_reverse_iterator::reference;
        using iterator_category = parent::prefix_reverse_iterator::iterator_category;

        explicit prefix_reverse_iterator(parent::node* n = nullptr) noexcept;
        prefix_reverse_iterator(parent::prefix_reverse_iterator) noexcept;

        node_color get_color() const noexcept;

        prefix_reverse_iterator(prefix_iterator) noexcept;
        operator prefix_iterator() const noexcept;
        prefix_iterator base() const noexcept;

        using parent::prefix_reverse_iterator::depth;
        using parent::prefix_reverse_iterator::operator*;
        using parent::prefix_reverse_iterator::operator==;
        using parent::prefix_reverse_iterator::operator!=;
        using parent::prefix_reverse_iterator::operator++;
        using parent::prefix_reverse_iterator::operator--;
        using parent::prefix_reverse_iterator::operator->;
    };

    class prefix_const_reverse_iterator : public parent::prefix_const_reverse_iterator
    {
    public:

        using value_type = parent::prefix_const_reverse_iterator::value_type;
        using difference_type = parent::prefix_const_reverse_iterator::difference_type;
        using pointer = parent::prefix_const_reverse_iterator::pointer;
        using reference = parent::prefix_const_reverse_iterator::reference;
        using iterator_category = parent::prefix_const_reverse_iterator::iterator_category;

        explicit prefix_const_reverse_iterator(parent::node* n = nullptr) noexcept;
        prefix_const_reverse_iterator(parent::prefix_const_reverse_iterator) noexcept;

        node_color get_color() const noexcept;

        prefix_const_reverse_iterator(prefix_const_iterator) noexcept;
        operator prefix_const_iterator() const noexcept;
        prefix_const_iterator base() const noexcept;

        using parent::prefix_const_reverse_iterator::depth;
        using parent::prefix_const_reverse_iterator::operator*;
        using parent::prefix_const_reverse_iterator::operator==;
        using parent::prefix_const_reverse_iterator::operator!=;
        using parent::prefix_const_reverse_iterator::operator++;
        using parent::prefix_const_reverse_iterator::operator--;
        using parent::prefix_const_reverse_iterator::operator->;
    };

    class infix_iterator : public parent::infix_iterator
    {
        friend binary_search_tree<tkey, tvalue, compare, __detail::RB_TAG>;
        friend class __detail::bst_impl<tkey, tvalue, compare, __detail::RB_TAG>;
    public:

        using value_type = parent::infix_iterator::value_type;
        using difference_type = parent::infix_iterator::difference_type;
        using pointer = parent::infix_iterator::pointer;
        using reference = parent::infix_iterator::reference;
        using iterator_category = parent::infix_iterator::iterator_category;

        explicit infix_iterator(parent::node* n = nullptr) noexcept;
        infix_iterator(parent::infix_iterator) noexcept;

        node_color get_color() const noexcept;

        using parent::infix_iterator::depth;
        using parent::infix_iterator::operator*;
        using parent::infix_iterator::operator==;
        using parent::infix_iterator::operator!=;
        using parent::infix_iterator::operator++;
        using parent::infix_iterator::operator--;
        using parent::infix_iterator::operator->;
    };

    class infix_const_iterator : parent::infix_const_iterator
    {
    public:

        using value_type = parent::infix_const_iterator::value_type;
        using difference_type = parent::infix_const_iterator::difference_type;
        using pointer = parent::infix_const_iterator::pointer;
        using reference = parent::infix_const_iterator::reference;
        using iterator_category = parent::infix_const_iterator::iterator_category;

        explicit infix_const_iterator(parent::node* n = nullptr) noexcept;
        infix_const_iterator(parent::infix_const_iterator) noexcept;

        node_color get_color() const noexcept;

        infix_const_iterator(infix_iterator) noexcept;

        using parent::infix_const_iterator::depth;
        using parent::infix_const_iterator::operator*;
        using parent::infix_const_iterator::operator==;
        using parent::infix_const_iterator::operator!=;
        using parent::infix_const_iterator::operator++;
        using parent::infix_const_iterator::operator--;
        using parent::infix_const_iterator::operator->;
    };

    class infix_reverse_iterator : public parent::infix_reverse_iterator
    {
    public:

        using value_type = parent::infix_reverse_iterator::value_type;
        using difference_type = parent::infix_reverse_iterator::difference_type;
        using pointer = parent::infix_reverse_iterator::pointer;
        using reference = parent::infix_reverse_iterator::reference;
        using iterator_category = parent::infix_reverse_iterator::iterator_category;

        explicit infix_reverse_iterator(parent::node* n = nullptr) noexcept;
        infix_reverse_iterator(parent::infix_reverse_iterator) noexcept;

        node_color get_color() const noexcept;

        infix_reverse_iterator(infix_iterator) noexcept;
        operator infix_iterator() const noexcept;
        infix_iterator base() const noexcept;

        using parent::infix_reverse_iterator::depth;
        using parent::infix_reverse_iterator::operator*;
        using parent::infix_reverse_iterator::operator==;
        using parent::infix_reverse_iterator::operator!=;
        using parent::infix_reverse_iterator::operator++;
        using parent::infix_reverse_iterator::operator--;
        using parent::infix_reverse_iterator::operator->;
    };

    class infix_const_reverse_iterator : public parent::infix_const_reverse_iterator
    {
    public:

        using value_type = parent::infix_const_reverse_iterator::value_type;
        using difference_type = parent::infix_const_reverse_iterator::difference_type;
        using pointer = parent::infix_const_reverse_iterator::pointer;
        using reference = parent::infix_const_reverse_iterator::reference;
        using iterator_category = parent::infix_const_reverse_iterator::iterator_category;

        explicit infix_const_reverse_iterator(parent::node* n = nullptr) noexcept;
        infix_const_reverse_iterator(parent::infix_const_reverse_iterator) noexcept;

        node_color get_color() const noexcept;

        infix_const_reverse_iterator(infix_const_iterator) noexcept;
        operator infix_const_iterator() const noexcept;
        infix_const_iterator base() const noexcept;

        using parent::infix_const_reverse_iterator::depth;
        using parent::infix_const_reverse_iterator::operator*;
        using parent::infix_const_reverse_iterator::operator==;
        using parent::infix_const_reverse_iterator::operator!=;
        using parent::infix_const_reverse_iterator::operator++;
        using parent::infix_const_reverse_iterator::operator--;
        using parent::infix_const_reverse_iterator::operator->;
    };

    class postfix_iterator : public parent::postfix_iterator
    {
        friend binary_search_tree<tkey, tvalue, compare, __detail::RB_TAG>;
        friend class __detail::bst_impl<tkey, tvalue, compare, __detail::RB_TAG>;
    public:

        using value_type = parent::postfix_iterator::value_type;
        using difference_type = parent::postfix_iterator::difference_type;
        using pointer = parent::postfix_iterator::pointer;
        using reference = parent::postfix_iterator::reference;
        using iterator_category = parent::postfix_iterator::iterator_category;

        explicit postfix_iterator(parent::node* n = nullptr) noexcept;
        postfix_iterator(parent::postfix_iterator) noexcept;

        node_color get_color() const noexcept;

        using parent::postfix_iterator::depth;
        using parent::postfix_iterator::operator*;
        using parent::postfix_iterator::operator==;
        using parent::postfix_iterator::operator!=;
        using parent::postfix_iterator::operator++;
        using parent::postfix_iterator::operator--;
        using parent::postfix_iterator::operator->;
    };

    class postfix_const_iterator : public parent::postfix_const_iterator
    {
    public:

        using value_type = parent::postfix_const_iterator::value_type;
        using difference_type = parent::postfix_const_iterator::difference_type;
        using pointer = parent::postfix_const_iterator::pointer;
        using reference = parent::postfix_const_iterator::reference;
        using iterator_category = parent::postfix_const_iterator::iterator_category;

        explicit postfix_const_iterator(parent::node* n = nullptr) noexcept;
        postfix_const_iterator(parent::postfix_const_iterator) noexcept;

        node_color get_color() const noexcept;

        postfix_const_iterator(postfix_iterator) noexcept;

        using parent::postfix_const_iterator::depth;
        using parent::postfix_const_iterator::operator*;
        using parent::postfix_const_iterator::operator==;
        using parent::postfix_const_iterator::operator!=;
        using parent::postfix_const_iterator::operator++;
        using parent::postfix_const_iterator::operator--;
        using parent::postfix_const_iterator::operator->;
    };

    class postfix_reverse_iterator : public parent::postfix_reverse_iterator
    {
    public:

        using value_type = parent::postfix_reverse_iterator::value_type;
        using difference_type = parent::postfix_reverse_iterator::difference_type;
        using pointer = parent::postfix_reverse_iterator::pointer;
        using reference = parent::postfix_reverse_iterator::reference;
        using iterator_category = parent::postfix_reverse_iterator::iterator_category;

        explicit postfix_reverse_iterator(parent::node* n = nullptr) noexcept;
        postfix_reverse_iterator(parent::postfix_reverse_iterator) noexcept;

        node_color get_color() const noexcept;

        postfix_reverse_iterator(postfix_iterator) noexcept;
        operator postfix_iterator() const noexcept;
        postfix_iterator base() const noexcept;

        using parent::postfix_reverse_iterator::depth;
        using parent::postfix_reverse_iterator::operator*;
        using parent::postfix_reverse_iterator::operator==;
        using parent::postfix_reverse_iterator::operator!=;
        using parent::postfix_reverse_iterator::operator++;
        using parent::postfix_reverse_iterator::operator--;
        using parent::postfix_reverse_iterator::operator->;
    };

    class postfix_const_reverse_iterator : public parent::postfix_const_reverse_iterator
    {
    public:

        using value_type = parent::postfix_const_reverse_iterator::value_type;
        using difference_type = parent::postfix_const_reverse_iterator::difference_type;
        using pointer = parent::postfix_const_reverse_iterator::pointer;
        using reference = parent::postfix_const_reverse_iterator::reference;
        using iterator_category = parent::postfix_const_reverse_iterator::iterator_category;

        explicit postfix_const_reverse_iterator(parent::node* n = nullptr) noexcept;
        postfix_const_reverse_iterator(parent::postfix_const_reverse_iterator) noexcept;

        node_color get_color() const noexcept;

        postfix_const_reverse_iterator(postfix_const_iterator) noexcept;
        operator postfix_const_iterator() const noexcept;
        postfix_const_iterator base() const noexcept;

        using parent::postfix_const_reverse_iterator::depth;
        using parent::postfix_const_reverse_iterator::operator*;
        using parent::postfix_const_reverse_iterator::operator==;
        using parent::postfix_const_reverse_iterator::operator!=;
        using parent::postfix_const_reverse_iterator::operator++;
        using parent::postfix_const_reverse_iterator::operator--;
        using parent::postfix_const_reverse_iterator::operator->;

    };



    // endregion iterator definition

    // region iterator requests declaration

    infix_iterator begin() noexcept;

    infix_iterator end() noexcept;

    infix_const_iterator begin() const noexcept;

    infix_const_iterator end() const noexcept;

    infix_const_iterator cbegin() const noexcept;

    infix_const_iterator cend() const noexcept;

    infix_reverse_iterator rbegin() noexcept;

    infix_reverse_iterator rend() noexcept;

    infix_const_reverse_iterator rbegin() const noexcept;

    infix_const_reverse_iterator rend() const noexcept;

    infix_const_reverse_iterator crbegin() const noexcept;

    infix_const_reverse_iterator crend() const noexcept;


    prefix_iterator begin_prefix() noexcept;

    prefix_iterator end_prefix() noexcept;

    prefix_const_iterator begin_prefix() const noexcept;

    prefix_const_iterator end_prefix() const noexcept;

    prefix_const_iterator cbegin_prefix() const noexcept;

    prefix_const_iterator cend_prefix() const noexcept;

    prefix_reverse_iterator rbegin_prefix() noexcept;

    prefix_reverse_iterator rend_prefix() noexcept;

    prefix_const_reverse_iterator rbegin_prefix() const noexcept;

    prefix_const_reverse_iterator rend_prefix() const noexcept;

    prefix_const_reverse_iterator crbegin_prefix() const noexcept;

    prefix_const_reverse_iterator crend_prefix() const noexcept;


    infix_iterator begin_infix() noexcept;

    infix_iterator end_infix() noexcept;

    infix_const_iterator begin_infix() const noexcept;

    infix_const_iterator end_infix() const noexcept;

    infix_const_iterator cbegin_infix() const noexcept;

    infix_const_iterator cend_infix() const noexcept;

    infix_reverse_iterator rbegin_infix() noexcept;

    infix_reverse_iterator rend_infix() noexcept;

    infix_const_reverse_iterator rbegin_infix() const noexcept;

    infix_const_reverse_iterator rend_infix() const noexcept;

    infix_const_reverse_iterator crbegin_infix() const noexcept;

    infix_const_reverse_iterator crend_infix() const noexcept;


    postfix_iterator begin_postfix() noexcept;

    postfix_iterator end_postfix() noexcept;

    postfix_const_iterator begin_postfix() const noexcept;

    postfix_const_iterator end_postfix() const noexcept;

    postfix_const_iterator cbegin_postfix() const noexcept;

    postfix_const_iterator cend_postfix() const noexcept;

    postfix_reverse_iterator rbegin_postfix() noexcept;

    postfix_reverse_iterator rend_postfix() noexcept;

    postfix_const_reverse_iterator rbegin_postfix() const noexcept;

    postfix_const_reverse_iterator rend_postfix() const noexcept;

    postfix_const_reverse_iterator crbegin_postfix() const noexcept;

    postfix_const_reverse_iterator crend_postfix() const noexcept;

    // endregion iterator requests declaration

public:

    ~red_black_tree() noexcept final = default;

    red_black_tree(red_black_tree const &other);

    red_black_tree &operator=(red_black_tree const &other);

    red_black_tree(red_black_tree &&other) noexcept;

    red_black_tree &operator=(red_black_tree &&other) noexcept;


    void swap(parent& other) noexcept override;


    /** Only rebinds iterators
     */
    std::pair<infix_iterator, bool> insert(const value_type&);
    std::pair<infix_iterator, bool> insert(value_type&&);

    template<class ...Args>
    std::pair<infix_iterator, bool> emplace(Args&&...args);

    infix_iterator insert_or_assign(const value_type&);
    infix_iterator insert_or_assign(value_type&&);

    template<class ...Args>
    infix_iterator emplace_or_assign(Args&&...args);

    infix_iterator find(const tkey&);
    infix_const_iterator find(const tkey&) const;

    infix_iterator lower_bound(const tkey&);
    infix_const_iterator lower_bound(const tkey&) const;

    infix_iterator upper_bound(const tkey&);
    infix_const_iterator upper_bound(const tkey&) const;

    infix_iterator erase(infix_iterator pos);
    infix_iterator erase(infix_const_iterator pos);

    infix_iterator erase(infix_iterator first, infix_iterator last);
    infix_iterator erase(infix_const_iterator first, infix_const_iterator last);

    using parent::erase;
    using parent::insert;
    using parent::insert_or_assign;
};










template<typename compare, typename U, typename iterator>
explicit red_black_tree(iterator begin, iterator end, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> red_black_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare>;

template<typename compare, typename U, std::ranges::forward_range Range>
explicit red_black_tree(Range&& range, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> red_black_tree<typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::first_type, typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::second_type, compare> ;

template<typename tkey, typename tvalue, typename compare, typename U>
red_black_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> red_black_tree<tkey, tvalue, compare>;

namespace __detail {

    class RB_TAG {};

    template<typename tkey, typename tvalue, typename compare>
    template<class ...Args>
    binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* bst_impl<tkey, tvalue, compare, RB_TAG>::create_node(
            binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont, Args&& ...args)
    {
        using node_type = typename red_black_tree<tkey, tvalue, compare>::node;
        auto *new_node = cont._allocator.template new_object<node_type>(std::forward<Args>(args)...);
        return new_node;
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, RB_TAG>::delete_node(
            binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node** node)
    {
        using node_type = typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node;
        if (node && *node)
        {
            cont._allocator.template delete_object<node_type>(*node);
            *node = nullptr;
        }
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, RB_TAG>::post_insert(
            binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node** node_ptr) {

        using rb_node = typename red_black_tree<tkey, tvalue, compare>::node;
        using node_type = typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node;
        using color_type = typename red_black_tree<tkey, tvalue, compare>::node_color;

        if (node_ptr == nullptr || *node_ptr == nullptr) return;

        auto* t = static_cast<rb_node*>(*node_ptr);

        while (t != nullptr && t->parent != nullptr && static_cast<rb_node*>(t->parent)->color == color_type::RED) {
            auto* parent = static_cast<rb_node*>(t->parent);
            auto* grandparent = static_cast<rb_node*>(parent->parent);

            if (parent == grandparent->left_subtree) {
                auto* uncle = static_cast<rb_node*>(grandparent->right_subtree);

                if (uncle && uncle->color == color_type::RED) {
                    parent->color = color_type::BLACK;
                    uncle->color = color_type::BLACK;
                    grandparent->color = color_type::RED;
                    t = grandparent;
                } else {
                    if (t == parent->right_subtree) {
                        t = parent;
                        auto* g = grandparent; // Сохраняем до поворота
                        cont.small_left_rotation(reinterpret_cast<node_type*&>(parent));
                        parent = static_cast<rb_node*>(t->parent);
                        grandparent = g; // Восстанавливаем
                    }
                    parent->color = color_type::BLACK;
                    grandparent->color = color_type::RED;
                    cont.small_right_rotation(reinterpret_cast<node_type*&>(grandparent));
                }

            } else {
                auto* uncle = static_cast<rb_node*>(grandparent->left_subtree);

                if (uncle && uncle->color == color_type::RED) {
                    parent->color = color_type::BLACK;
                    uncle->color = color_type::BLACK;
                    grandparent->color = color_type::RED;
                    t = grandparent;
                } else {
                    if (t == parent->left_subtree) {
                        t = parent;
                        auto* g = grandparent;
                        cont.small_right_rotation(reinterpret_cast<node_type*&>(parent));
                        parent = static_cast<rb_node*>(t->parent);
                        grandparent = g;
                    }
                    parent->color = color_type::BLACK;
                    grandparent->color = color_type::RED;
                    cont.small_left_rotation(reinterpret_cast<node_type*&>(grandparent));
                }
            }
        }

        static_cast<rb_node*>(cont._root)->color = color_type::BLACK;
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, RB_TAG>::erase(
            binary_search_tree<tkey, tvalue, compare, RB_TAG>& tree,
            typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node** node_to_delete_ptr)
    {
        using rb_node = typename red_black_tree<tkey, tvalue, compare>::node;
        using node_type = typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node;
        using color_type = typename red_black_tree<tkey, tvalue, compare>::node_color;

        if (!(*node_to_delete_ptr)) return;
        auto* node_to_delete = static_cast<rb_node*>(*node_to_delete_ptr);

        rb_node* replacement_node = node_to_delete; // Узел, который фактически удаляется
        color_type replacement_original_color = replacement_node->color;

        rb_node* child_node = nullptr; // Ребенок replacement_node, который займет его место
        rb_node* child_parent = nullptr; // Родитель child_node

        if (node_to_delete->left_subtree == nullptr) {
            child_node = static_cast<rb_node*>(node_to_delete->right_subtree);
            child_parent = static_cast<rb_node*>(node_to_delete->parent);
            replace_node(tree, node_to_delete, child_node);
        }
        else if (node_to_delete->right_subtree == nullptr) {
            child_node = static_cast<rb_node*>(node_to_delete->left_subtree);
            child_parent = static_cast<rb_node*>(node_to_delete->parent);
            replace_node(tree, node_to_delete, child_node);
        }
        else {
            replacement_node = static_cast<rb_node*>(node_to_delete->left_subtree);
            while (replacement_node->right_subtree != nullptr) {
                replacement_node = static_cast<rb_node*>(replacement_node->right_subtree);
            }
            replacement_original_color = replacement_node->color;
            child_node = static_cast<rb_node*>(replacement_node->left_subtree);

            if (replacement_node->parent == node_to_delete) {
                child_parent = replacement_node;
            }
            else {
                child_parent = static_cast<rb_node*>(replacement_node->parent);
                replace_node(tree, replacement_node, child_node);
                replacement_node->left_subtree = node_to_delete->left_subtree;
                if (replacement_node->left_subtree) {
                    static_cast<rb_node*>(replacement_node->left_subtree)->parent = replacement_node;
                }
            }
            replace_node(tree, node_to_delete, replacement_node);
            replacement_node->right_subtree = node_to_delete->right_subtree;
            if (replacement_node->right_subtree) {
                static_cast<rb_node*>(replacement_node->right_subtree)->parent = replacement_node;
            }
            replacement_node->color = node_to_delete->color;
        }
        delete_node(tree, node_to_delete_ptr);

        // Если удаленный узел был черным, нужно восстановить свойства красно-черного дерева
        if (replacement_original_color == color_type::BLACK) {
            if (child_node != nullptr) {
                fix_double_black(tree, child_node);
            }
            else if (child_parent != nullptr) {
                auto* temp_nil = new rb_node(static_cast<node_type*>(child_parent));
                temp_nil->color = color_type::BLACK;

                // Прикрепляем временный узел в нужное место
                if (child_parent == replacement_node) {
                    if (replacement_node->left_subtree == child_node) {
                        replacement_node->left_subtree = temp_nil;
                    }
                }
                else if (child_parent->left_subtree == child_node) {
                    child_parent->left_subtree = temp_nil;
                }
                else if (child_parent->right_subtree == child_node) {
                    child_parent->right_subtree = temp_nil;
                }

                fix_double_black(tree, temp_nil);

                // Удаляем временный NIL-узел
                if (temp_nil->parent) {
                    if (static_cast<rb_node*>(temp_nil->parent)->left_subtree == temp_nil) {
                        static_cast<rb_node*>(temp_nil->parent)->left_subtree = nullptr;
                    }
                    else if (static_cast<rb_node*>(temp_nil->parent)->right_subtree == temp_nil) {
                        static_cast<rb_node*>(temp_nil->parent)->right_subtree = nullptr;
                    }
                }
                else if (tree._root == temp_nil) {
                    tree._root = nullptr;
                }
                delete temp_nil;
            }
        }

        // Корень всегда должен быть черным
        if (tree._root != nullptr) {
            static_cast<rb_node*>(tree._root)->color = color_type::BLACK;
        }
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, RB_TAG>::replace_node(
            binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* old_node, // u
            typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* new_node) // v
    {
        if (old_node->parent == nullptr) {
            cont._root = new_node;
        } else {
            if (old_node == old_node->parent->left_subtree)
                old_node->parent->left_subtree = new_node;
            else
                old_node->parent->right_subtree = new_node;
        }

        if (new_node) {
            new_node->parent = old_node->parent;
        }
    }


    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, RB_TAG>::fix_double_black(
            binary_search_tree<tkey, tvalue, compare, RB_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node* x_node)
    {
        using node_type = typename binary_search_tree<tkey, tvalue, compare, RB_TAG>::node;
        using rb_node = typename red_black_tree<tkey, tvalue, compare>::node;
        using color_type = typename red_black_tree<tkey, tvalue, compare>::node_color;

        auto* current_x = static_cast<rb_node*>(x_node);

        while (current_x != cont._root && current_x->color == color_type::BLACK) {
            auto* parent = static_cast<rb_node*>(current_x->parent);

            bool is_left_child = (current_x == parent->left_subtree);
            rb_node* sibling = is_left_child
                               ? static_cast<rb_node*>(parent->right_subtree)
                               : static_cast<rb_node*>(parent->left_subtree);

            if (sibling == nullptr) {
                current_x = parent;
                continue;
            }

            node_type** ptr_to_rotate_target_in_structure;

            // Случай 1: Брат красный
            if (sibling->color == color_type::RED) {
                parent->color = color_type::RED;
                sibling->color = color_type::BLACK;

                if (parent->parent == nullptr) {
                    ptr_to_rotate_target_in_structure = &cont._root;
                } else {
                    if (parent == parent->parent->left_subtree) {
                        ptr_to_rotate_target_in_structure = &(parent->parent->left_subtree);
                    } else {
                        ptr_to_rotate_target_in_structure = &(parent->parent->right_subtree);
                    }
                }

                if (is_left_child) {
                    cont.small_left_rotation(*ptr_to_rotate_target_in_structure);
                } else {
                    cont.small_right_rotation(*ptr_to_rotate_target_in_structure);
                }

                sibling = is_left_child
                          ? static_cast<rb_node*>(parent->right_subtree)
                          : static_cast<rb_node*>(parent->left_subtree);

                if (sibling == nullptr) {
                    current_x = parent;
                    continue;
                }
            }

            auto* sibling_left = static_cast<rb_node*>(sibling->left_subtree);
            auto* sibling_right = static_cast<rb_node*>(sibling->right_subtree);

            bool sibling_left_is_black = (sibling_left == nullptr || sibling_left->color == color_type::BLACK);
            bool sibling_right_is_black = (sibling_right == nullptr || sibling_right->color == color_type::BLACK);

            // Случай 2: Брат черный, и оба его ребенка черные.
            if (sibling_left_is_black && sibling_right_is_black) {
                sibling->color = color_type::RED;
                current_x = parent;
                continue;
            }
            // Случаи 3 и 4: Брат черный, и хотя бы один его ребенок красный.
            if (is_left_child) { // current_x - левый ребенок, sibling - правый.
                // Случай 3: Правый ребенок sibling черный (значит, левый ребенок sibling красный).
                if (sibling_right_is_black) {
                    if (sibling_left) sibling_left->color = color_type::BLACK;
                    sibling->color = color_type::RED;
                    cont.small_right_rotation(parent->right_subtree);
                    sibling = static_cast<rb_node*>(parent->right_subtree);
                    sibling_right = (sibling) ? static_cast<rb_node*>(sibling->right_subtree) : nullptr;
                }
                // Случай 4: Правый ребенок sibling красный.
                sibling->color = parent->color;
                parent->color = color_type::BLACK;
                if (sibling_right) { // Этот ребенок был красным (или стал после случая 3).
                    sibling_right->color = color_type::BLACK;
                }
                // Левый поворот вокруг parent.
                if (parent->parent == nullptr) { ptr_to_rotate_target_in_structure = &cont._root; }
                else {
                    if (parent == parent->parent->left_subtree) ptr_to_rotate_target_in_structure = &(parent->parent->left_subtree);
                    else ptr_to_rotate_target_in_structure = &(parent->parent->right_subtree);
                }
                cont.small_left_rotation(*ptr_to_rotate_target_in_structure);
                current_x = static_cast<rb_node*>(cont._root);
            } else { // current_x - правый ребенок, sibling - левый (симметрично).
                // Случай 3: Левый ребенок sibling черный (значит, правый ребенок sibling красный).
                if (sibling_left_is_black) {
                    if (sibling_right) sibling_right->color = color_type::BLACK;
                    sibling->color = color_type::RED;
                    cont.small_left_rotation(parent->left_subtree);
                    sibling = static_cast<rb_node*>(parent->left_subtree);
                    sibling_left = (sibling) ? static_cast<rb_node*>(sibling->left_subtree) : nullptr;
                }
                // Случай 4: Левый ребенок sibling красный.
                sibling->color = parent->color;
                parent->color = color_type::BLACK;
                if (sibling_left) {
                    sibling_left->color = color_type::BLACK;
                }
                // Правый поворот вокруг parent.
                if (parent->parent == nullptr) { ptr_to_rotate_target_in_structure = &cont._root; }
                else {
                    if (parent == parent->parent->left_subtree) ptr_to_rotate_target_in_structure = &(parent->parent->left_subtree);
                    else ptr_to_rotate_target_in_structure = &(parent->parent->right_subtree);
                }
                cont.small_right_rotation(*ptr_to_rotate_target_in_structure);
                current_x = static_cast<rb_node*>(cont._root);
            }
        }

        if (current_x != nullptr) {
            current_x->color = color_type::BLACK;
        }
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, RB_TAG>::swap(binary_search_tree<tkey, tvalue, compare, RB_TAG> &lhs,
                                                                            binary_search_tree<tkey, tvalue, compare, RB_TAG> &rhs) noexcept
    {
        using std::swap;
        swap(lhs.root, rhs.root);
        swap(lhs._logger, rhs._logger);
        swap(lhs._size, rhs._size);
        swap(lhs._allocator, rhs._allocator);
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare>
template<class ...Args>
red_black_tree<tkey, tvalue, compare>::node::node(parent::node* par, Args&&... args)
        : parent::node(par, std::forward<Args>(args)...), color(node_color::RED)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::red_black_tree(
        const compare& comp,
        pp_allocator<value_type> alloc,
        logger *logger) : parent(comp, alloc, logger)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::red_black_tree(
        pp_allocator<value_type> alloc,
        const compare& comp,
        logger *logger) : parent(alloc, comp, logger)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<input_iterator_for_pair<tkey, tvalue> iterator>
red_black_tree<tkey, tvalue, compare>::red_black_tree(
        iterator begin, iterator end,
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
red_black_tree<tkey, tvalue, compare>::red_black_tree(
        Range&& range,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger): parent(range, cmp, alloc, logger)
{
    for (auto &&element : range)
    {
        emplace(std::forward<decltype(element)>(element));
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::red_black_tree(
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

// region iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_iterator::prefix_iterator(parent::node* n) noexcept : parent::prefix_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_iterator::prefix_iterator(parent::prefix_iterator it) noexcept : parent::prefix_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::prefix_iterator::get_color() const noexcept
{
    return static_cast<node*>(this->_data)->color;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_iterator::prefix_const_iterator(parent::node* n) noexcept : parent::prefix_const_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_iterator::prefix_const_iterator(parent::prefix_const_iterator it) noexcept : parent::prefix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::prefix_const_iterator::get_color() const noexcept
{
    return prefix_iterator(this->_base).get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_iterator::prefix_const_iterator(prefix_iterator it) noexcept : parent::prefix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator::prefix_reverse_iterator(parent::node* n) noexcept : parent::prefix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator::prefix_reverse_iterator(parent::prefix_reverse_iterator it) noexcept : parent::prefix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator::get_color() const noexcept
{
    return prefix_iterator(this->_base)->color;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator::prefix_reverse_iterator(prefix_iterator it) noexcept : parent::prefix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator::operator red_black_tree<tkey, tvalue, compare>::prefix_iterator() const noexcept
{
    return parent::prefix_reverse_iterator::operator prefix_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_iterator
red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator::base() const noexcept
{
    return parent::prefix_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(parent::node* n) noexcept : parent::prefix_const_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(parent::prefix_const_reverse_iterator it) noexcept : parent::prefix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::get_color() const noexcept
{
    return prefix_iterator(this->_base)->color;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(prefix_const_iterator it) noexcept : parent::prefix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::operator red_black_tree<tkey, tvalue, compare>::prefix_const_iterator() const noexcept
{
    return parent::prefix_const_reverse_iterator::operator prefix_const_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_iterator
red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::base() const noexcept
{
    return parent::prefix_const_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_iterator::infix_iterator(parent::node* n) noexcept : parent::infix_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_iterator::infix_iterator(parent::infix_iterator it) noexcept : parent::infix_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::infix_iterator::get_color() const noexcept
{
    return static_cast<node*>(this->_data)->color;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_iterator::infix_const_iterator(parent::node* n) noexcept : parent::infix_const_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_iterator::infix_const_iterator(parent::infix_const_iterator it) noexcept : parent::infix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::infix_const_iterator::get_color() const noexcept
{
    return infix_iterator(this->_base).get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_iterator::infix_const_iterator(infix_iterator it) noexcept : parent::infix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator::infix_reverse_iterator(parent::node* n) noexcept : parent::infix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator::infix_reverse_iterator(parent::infix_reverse_iterator it) noexcept : parent::infix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator::get_color() const noexcept
{
    return infix_iterator(this->_base)->get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator::infix_reverse_iterator(infix_iterator it) noexcept : parent::infix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator::operator red_black_tree<tkey, tvalue, compare>::infix_iterator() const noexcept
{
    return parent::infix_reverse_iterator::operator infix_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator::base() const noexcept
{
    return parent::infix_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::infix_const_reverse_iterator(parent::node* n) noexcept : parent::infix_const_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::infix_const_reverse_iterator(parent::infix_const_reverse_iterator it) noexcept : parent::infix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::get_color() const noexcept
{
    return infix_iterator(this->_base)->get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::infix_const_reverse_iterator(infix_const_iterator it) noexcept : parent::infix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::operator red_black_tree<tkey, tvalue, compare>::infix_const_iterator() const noexcept
{
    return parent::infix_const_reverse_iterator::operator infix_const_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::base() const noexcept
{
    return parent::infix_const_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_iterator::postfix_iterator(parent::node* n) noexcept : parent::postfix_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_iterator::postfix_iterator(parent::postfix_iterator it) noexcept : parent::postfix_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::postfix_iterator::get_color() const noexcept
{
    return static_cast<node*>(this->_data)->color;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_iterator::postfix_const_iterator(parent::node* n) noexcept : parent::postfix_const_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_iterator::postfix_const_iterator(parent::postfix_const_iterator it) noexcept : parent::postfix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::postfix_const_iterator::get_color() const noexcept
{
    return postfix_iterator(this->_base).get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_iterator::postfix_const_iterator(postfix_iterator it) noexcept : parent::postfix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator::postfix_reverse_iterator(parent::node* n) noexcept : parent::postfix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator::postfix_reverse_iterator(parent::postfix_reverse_iterator it) noexcept : parent::postfix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator::get_color() const noexcept
{
    return postfix_iterator(this->_base).get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator::postfix_reverse_iterator(postfix_iterator it) noexcept : parent::postfix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator::operator red_black_tree<tkey, tvalue, compare>::postfix_iterator() const noexcept
{
    return parent::postfix_reverse_iterator::operator postfix_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_iterator
red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator::base() const noexcept
{
    return parent::postfix_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(parent::node* n) noexcept : parent::postfix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(parent::postfix_const_reverse_iterator it) noexcept : parent::postfix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::node_color
red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::get_color() const noexcept
{
    return postfix_iterator(this->_base).get_color();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(postfix_const_iterator it) noexcept : parent::postfix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::operator red_black_tree<tkey, tvalue, compare>::postfix_const_iterator() const noexcept
{
    return parent::postfix_const_reverse_iterator::operator postfix_const_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_iterator
red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::base() const noexcept
{
    return parent::postfix_const_reverse_iterator::base();
}

// endregion iterator implementation

// region iterator requests implementation

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::begin() noexcept
{
    return parent::begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::end() noexcept
{
    return parent::end();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::begin() const noexcept
{
    return parent::begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::end() const noexcept
{
    return parent::end();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::cbegin() const noexcept
{
    return parent::cbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::cend() const noexcept
{
    return parent::cend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin() noexcept
{
    return parent::rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend() noexcept
{
    return parent::rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin() const noexcept
{
    return parent::rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend() const noexcept
{
    return parent::rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crbegin() const noexcept
{
    return parent::crbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crend() const noexcept
{
    return parent::crend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_iterator
red_black_tree<tkey, tvalue, compare>::begin_prefix() noexcept
{
    return parent::begin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_iterator
red_black_tree<tkey, tvalue, compare>::end_prefix() noexcept
{
    return parent::end_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_iterator
red_black_tree<tkey, tvalue, compare>::begin_prefix() const noexcept
{
    return parent::begin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_iterator
red_black_tree<tkey, tvalue, compare>::end_prefix() const noexcept
{
    return parent::end_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_iterator
red_black_tree<tkey, tvalue, compare>::cbegin_prefix() const noexcept
{
    return parent::cbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_iterator
red_black_tree<tkey, tvalue, compare>::cend_prefix() const noexcept
{
    return parent::cend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin_prefix() noexcept
{
    return parent::rbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend_prefix() noexcept
{
    return parent::rend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin_prefix() const noexcept
{
    return parent::rbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend_prefix() const noexcept
{
    return parent::rend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crbegin_prefix() const noexcept
{
    return parent::crbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crend_prefix() const noexcept
{
    return parent::crend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::begin_infix() noexcept
{
    return parent::begin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::end_infix() noexcept
{
    return parent::end_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::begin_infix() const noexcept
{
    return parent::begin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::end_infix() const noexcept
{
    return parent::end_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::cbegin_infix() const noexcept
{
    return parent::cbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::cend_infix() const noexcept
{
    return parent::cend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin_infix() noexcept
{
    return parent::rbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend_infix() noexcept
{
    return parent::rend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin_infix() const noexcept
{
    return parent::rbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend_infix() const noexcept
{
    return parent::rend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crbegin_infix() const noexcept
{
    return parent::crbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crend_infix() const noexcept
{
    return parent::crend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_iterator
red_black_tree<tkey, tvalue, compare>::begin_postfix() noexcept
{
    return parent::begin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_iterator
red_black_tree<tkey, tvalue, compare>::end_postfix() noexcept
{
    return parent::end_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_iterator
red_black_tree<tkey, tvalue, compare>::begin_postfix() const noexcept
{
    return parent::begin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_iterator
red_black_tree<tkey, tvalue, compare>::end_postfix() const noexcept
{
    return parent::end_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_iterator
red_black_tree<tkey, tvalue, compare>::cbegin_postfix() const noexcept
{
    return parent::cbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_iterator
red_black_tree<tkey, tvalue, compare>::cend_postfix() const noexcept
{
    return parent::cend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin_postfix() noexcept
{
    return parent::rbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend_postfix() noexcept
{
    return parent::rend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rbegin_postfix() const noexcept
{
    return parent::rbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::rend_postfix() const noexcept
{
    return parent::rend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crbegin_postfix() const noexcept
{
    return parent::crbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator
red_black_tree<tkey, tvalue, compare>::crend_postfix() const noexcept
{
    return parent::crend_postfix();
}

// endregion iterator requests implementation

// region rb_tree implementation

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::red_black_tree(red_black_tree const &other) : parent(other)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare> &
red_black_tree<tkey, tvalue, compare>::operator=(red_black_tree const &other)
{
    if (this != &other) {
        parent::operator=(other);
    }
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare>::red_black_tree(red_black_tree &&other) noexcept : parent(std::move(other))
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
red_black_tree<tkey, tvalue, compare> &
red_black_tree<tkey, tvalue, compare>::operator=(red_black_tree &&other) noexcept
{
    if(this != other){
        parent::operator=(std::move(other));
    }
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
void red_black_tree<tkey, tvalue, compare>::swap(parent& other) noexcept
{
    if (this != &other)
    {
        parent::swap(other);
    }
}

// endregion rb_tree implementation

template<typename tkey, typename tvalue, compator<tkey> compare>
std::pair<typename red_black_tree<tkey, tvalue, compare>::infix_iterator, bool>
red_black_tree<tkey, tvalue, compare>::insert(const value_type& value)
{
    auto result = parent::insert(value);
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
std::pair<typename red_black_tree<tkey, tvalue, compare>::infix_iterator, bool>
red_black_tree<tkey, tvalue, compare>::insert(value_type&& value)
{
    auto result = parent::insert(std::move(value));
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<class ...Args>
std::pair<typename red_black_tree<tkey, tvalue, compare>::infix_iterator, bool>
red_black_tree<tkey, tvalue, compare>::emplace(Args&&... args)
{
    return parent::emplace(std::forward<Args>(args)...);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::insert_or_assign(const value_type& value)
{
    auto result = parent::insert_or_assign(value);
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::insert_or_assign(value_type&& value)
{
    auto result = parent::insert_or_assign(std::move(value));
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<class ...Args>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::emplace_or_assign(Args&&... args)
{
    auto it = binary_search_tree<tkey, tvalue, compare, __detail::RB_TAG>::emplace_or_assign(std::forward<Args>(args)...);
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::find(const tkey& key)
{
    return parent::find(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::find(const tkey& key) const
{
    return parent::find(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::lower_bound(const tkey& key)
{
    return parent::lower_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::lower_bound(const tkey& key) const
{
    return parent::lower_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::upper_bound(const tkey& key)
{
    return parent::upper_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_const_iterator
red_black_tree<tkey, tvalue, compare>::upper_bound(const tkey& key) const
{
    return parent::upper_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::erase(infix_iterator pos)
{
    return parent::erase(pos);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::erase(infix_const_iterator pos)
{
    return parent::erase(pos);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::erase(infix_iterator first, infix_iterator last)
{
    return parent::erase(first, last);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename red_black_tree<tkey, tvalue, compare>::infix_iterator
red_black_tree<tkey, tvalue, compare>::erase(infix_const_iterator first, infix_const_iterator last)
{
    return parent::erase(first, last);
}

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_RED_BLACK_TREE_H
