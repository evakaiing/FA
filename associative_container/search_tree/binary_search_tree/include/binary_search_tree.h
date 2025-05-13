#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H

#include <list>
#include <stack>
#include <vector>
#include <memory>
#include <logger.h>
#include <logger_guardant.h>
#include <not_implemented.h>
#include <search_tree.h>
#include <stack>
#include <ranges>
#include <pp_allocator.h>
#include <concepts>

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare, typename tag>
    class bst_impl;

    class BST_TAG;
}


template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>, typename tag = __detail::BST_TAG>
class binary_search_tree : private compare
{
public:

    using value_type = std::pair<const tkey, tvalue>;

    friend class __detail::bst_impl<tkey, tvalue, compare, tag>;


protected:
    
    struct node
    {
    
    public:
        
        value_type data;

        node* parent;
        node* left_subtree;
        node* right_subtree;

        template<class ...Args>
        explicit node(node* parent, Args&& ...args);

        virtual ~node() = default;
    };

    friend class __detail::bst_impl<tkey, tvalue, compare, tag>;
    friend class binary_search_tree<tkey, tvalue, compare, tag>::node;

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const value_type & lhs, const value_type & rhs) const;

public:

    // region iterators definition

    class prefix_iterator;
    class prefix_const_iterator;
    class prefix_reverse_iterator;
    class prefix_const_reverse_iterator;

    class infix_iterator;
    class infix_const_iterator;
    class infix_reverse_iterator;
    class infix_const_reverse_iterator;

    class postfix_iterator;
    class postfix_const_iterator;
    class postfix_reverse_iterator;
    class postfix_const_reverse_iterator;

    /** @brief Watch about behavior of reverse iterators.
     *
     * @example Arrow is common iterator
     *  1 2 3 -> 4 5 6 7
     *  *it == 4.
     *
     *  @example But reverse:
     *  1 2 3 \<- 4 5 6 7
     *  *rit == 3
     */

    class prefix_iterator
    {
        friend class prefix_reverse_iterator;
        friend class binary_search_tree<tkey, tvalue, compare, tag>;
    protected:

        explicit prefix_iterator(node* data, node* backup);


        node* _data;

        /** If iterator == end or before_begin _data points to nullptr, _backup to last node
         *
         */
        node* _backup;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        
        explicit prefix_iterator(node* data = nullptr);

        virtual ~prefix_iterator() =default;

        bool operator==(
            prefix_iterator const &other) const noexcept;
        
        bool operator!=(
            prefix_iterator const &other) const noexcept;
        
        prefix_iterator &operator++() & noexcept;
        
        prefix_iterator operator++(int not_used) noexcept;

        prefix_iterator &operator--() & noexcept;

        prefix_iterator operator--(int not_used) noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;
        
    };
    
    class prefix_const_iterator
    {
        friend class binary_search_tree<tkey, tvalue, compare, tag>;
    protected:
        prefix_iterator _base;
        explicit prefix_const_iterator(const node* data, const node* backup);

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_const_iterator(const node* data = nullptr);

        prefix_const_iterator(const prefix_iterator&) noexcept;

        virtual ~prefix_const_iterator() = default;

        bool operator==(
                prefix_iterator const &other) const noexcept;

        bool operator!=(
                prefix_iterator const &other) const noexcept;

        prefix_const_iterator &operator++() & noexcept;

        prefix_const_iterator operator++(int not_used) noexcept;

        prefix_const_iterator &operator--() & noexcept;

        prefix_const_iterator operator--(int not_used) noexcept;

        /** Throws exception if end
         */
        reference operator*() const;

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;
        
    };
    
    class prefix_reverse_iterator
    {
    protected:

        prefix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_reverse_iterator(node* data = nullptr);

        explicit prefix_reverse_iterator(node* data, node* backup);

        prefix_reverse_iterator(const prefix_iterator&) noexcept;

        operator prefix_iterator() const noexcept; // неявное преобразование к prefix_iterator
        /*
         * prefix_reverse_iterator rit = ...;
         * prefix_iterator it = rit;
         * */

        prefix_iterator base() const noexcept;

        /*

           индекс:       0   1   2   3
           массив:      [A] [B] [C] [D]
                         ↑   ↑   ↑   ↑
                        it0 it1 it2 it3

        rev = reverse_iterator(it2) указывает на B
        rev.base() - указывает на С, туда же куда и it2 (изначальный итератор)
        *rev = *(rev.base - 1)

         */


        virtual ~prefix_reverse_iterator() =default;

        bool operator==(prefix_reverse_iterator const &other) const noexcept;

        bool operator!=(prefix_reverse_iterator const &other) const noexcept;

        prefix_reverse_iterator &operator++() & noexcept;

        prefix_reverse_iterator operator++(int not_used) noexcept;

        prefix_reverse_iterator &operator--() & noexcept;

        prefix_reverse_iterator operator--(int not_used) noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;
        
    };
    
    class prefix_const_reverse_iterator
    {
    protected:

        prefix_const_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_const_reverse_iterator(const node* data = nullptr);

        prefix_const_reverse_iterator(const prefix_const_reverse_iterator&) noexcept;

        operator prefix_const_iterator() const noexcept;
        prefix_const_iterator base() const noexcept;

        virtual ~prefix_const_reverse_iterator() =default;

        bool operator==(prefix_const_reverse_iterator const &other) const noexcept;

        bool operator!=(prefix_const_reverse_iterator const &other) const noexcept;

        prefix_const_reverse_iterator &operator++() & noexcept;

        prefix_const_reverse_iterator operator++(int not_used) noexcept;

        prefix_const_reverse_iterator &operator--()& noexcept;

        prefix_const_reverse_iterator operator--(int not_used) noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;
        
    };


    class infix_iterator
    {
        friend class infix_const_iterator;
        friend class binary_search_tree<tkey, tvalue, compare, tag>;
    protected:

        explicit infix_iterator(node* data, node* backup);

        node* _data;

        /** If iterator == end or before_begin _data points to nullptr, _backup to last node
         *
         */
        node* _backup;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_iterator(node* data = nullptr);

        virtual ~infix_iterator() =default;

        bool operator==(
                infix_iterator const &other) const noexcept;

        bool operator!=(
                infix_iterator const &other) const noexcept;

        infix_iterator &operator++() & noexcept;

        infix_iterator operator++(int not_used) noexcept;

        infix_iterator &operator--() & noexcept;

        infix_iterator operator--(int not_used) noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class infix_const_iterator
    {
    protected:

        infix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_const_iterator(const node* data = nullptr);

        explicit infix_const_iterator(const node* data, const node* backup);

        infix_const_iterator(const infix_iterator&) noexcept;

        virtual ~infix_const_iterator() =default;

        bool operator==(
                infix_const_iterator const &other) const noexcept;

        bool operator!=(
                infix_const_iterator const &other) const noexcept;

        infix_const_iterator &operator++() & noexcept;

        infix_const_iterator operator++(int not_used) noexcept;

        infix_const_iterator &operator--() & noexcept;

        infix_const_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class infix_reverse_iterator
    {
    protected:
        explicit infix_reverse_iterator(node* data, node* backup);
        infix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_reverse_iterator(node* data = nullptr);

        infix_reverse_iterator(const infix_iterator&) noexcept;
        operator infix_iterator() const noexcept;

        infix_iterator base() const noexcept;

        virtual ~infix_reverse_iterator() =default;

        bool operator==(infix_reverse_iterator const &other) const noexcept;

        bool operator!=(infix_reverse_iterator const &other) const noexcept;

        infix_reverse_iterator &operator++() & noexcept;

        infix_reverse_iterator operator++(int not_used) noexcept;

        infix_reverse_iterator &operator--() & noexcept;

        infix_reverse_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class infix_const_reverse_iterator
    {
    protected:

        infix_const_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_const_reverse_iterator(const node* data = nullptr);

        infix_const_reverse_iterator(const infix_const_iterator&) noexcept;

        operator infix_const_iterator() const noexcept;
        infix_const_iterator base() const noexcept;

        virtual ~infix_const_reverse_iterator() =default;

        bool operator==(infix_const_reverse_iterator const &other) const noexcept;

        bool operator!=(infix_const_reverse_iterator const &other) const noexcept;

        infix_const_reverse_iterator &operator++() & noexcept;

        infix_const_reverse_iterator operator++(int not_used) noexcept;

        infix_const_reverse_iterator &operator--() & noexcept;

        infix_const_reverse_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };


    class postfix_iterator
    {
        friend class binary_search_tree<tkey, tvalue, compare, tag>;
    protected:

        node* _data;

        /** If iterator == end or before_begin _data points to nullptr, _backup to last node
         *
         */
        node* _backup;

        explicit postfix_iterator(node* data, node* backup);

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_iterator(node* data = nullptr);

        virtual ~postfix_iterator() =default;

        bool operator==(
                postfix_iterator const &other) const noexcept;

        bool operator!=(
                postfix_iterator const &other) const noexcept;

        postfix_iterator &operator++() & noexcept;

        postfix_iterator operator++(int not_used) noexcept;

        postfix_iterator &operator--() & noexcept;

        postfix_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class postfix_const_iterator
    {
        friend class binary_search_tree<tkey, tvalue, compare, tag>;
    protected:

        postfix_iterator _base;

        explicit postfix_const_iterator(const node* data, const node* backup);

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_const_iterator(const node* data = nullptr);

        postfix_const_iterator(const postfix_iterator&) noexcept;

        virtual ~postfix_const_iterator() =default;

        bool operator==(
                postfix_iterator const &other) const noexcept;

        bool operator!=(
                postfix_iterator const &other) const noexcept;

        postfix_const_iterator &operator++() & noexcept;

        postfix_const_iterator operator++(int not_used) noexcept;

        postfix_const_iterator &operator--() & noexcept;

        postfix_const_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class postfix_reverse_iterator
    {
    protected:

        postfix_iterator _base;
        explicit postfix_reverse_iterator(node* data, node* backup);


    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_reverse_iterator(node* data = nullptr);

        postfix_reverse_iterator(const postfix_iterator&) noexcept;
        operator postfix_iterator() const noexcept;

        postfix_iterator base() const noexcept;

        virtual ~postfix_reverse_iterator() =default;

        bool operator==(postfix_reverse_iterator const &other) const noexcept;

        bool operator!=(postfix_reverse_iterator const &other) const noexcept;

        postfix_reverse_iterator &operator++() & noexcept;

        postfix_reverse_iterator operator++(int not_used) noexcept;

        postfix_reverse_iterator &operator--() & noexcept;

        postfix_reverse_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class postfix_const_reverse_iterator
    {
    protected:

        postfix_const_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_const_reverse_iterator(const node* data = nullptr);

        postfix_const_reverse_iterator(const postfix_const_iterator&) noexcept;

        operator postfix_const_iterator() const noexcept;
        postfix_const_iterator base() const noexcept;

        virtual ~postfix_const_reverse_iterator() =default;

        bool operator==(postfix_const_reverse_iterator const &other) const noexcept;

        bool operator!=(postfix_const_reverse_iterator const &other) const noexcept;

        postfix_const_reverse_iterator &operator++() & noexcept;

        postfix_const_reverse_iterator operator++(int not_used) noexcept;

        postfix_const_reverse_iterator &operator--() & noexcept;

        postfix_const_reverse_iterator const operator--(int not_used) const noexcept;

        /** Throws exception if end
         */
        reference operator*();

        /** UB if iterator points to end
         *
         */

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };



    friend class prefix_iterator;
    friend class prefix_const_iterator;
    friend class prefix_reverse_iterator;
    friend class prefix_const_reverse_iterator;

    friend class postfix_iterator;
    friend class postfix_const_iterator;
    friend class postfix_reverse_iterator;
    friend class postfix_const_reverse_iterator;

    friend class infix_iterator;
    friend class infix_const_iterator;
    friend class infix_reverse_iterator;
    friend class infix_const_reverse_iterator;

    // endregion iterators definition

public:

    struct not_found : public std::exception {
        const char* what() const noexcept override {
            return "element wasn't founded";
        }
    };

    struct duplicate_key : public std::exception {
        const char* what() const noexcept override {
            return "key already exist";
        }
    };

    struct allocation_error : public std::exception {
        const char* what() const noexcept override {
            return "memory allocation error";
        }
    };

    struct null_pointer_dereference_attempt : public std::exception {
        const char* what() const noexcept override {
            return "attempt to gain access to nullptr";
        }
    };

protected:
    
    node *_root;
    node* _fake_node;
    logger* _logger;
    size_t _size;
    
    /** You should use coercion ctor or template methods of allocator
     */
    pp_allocator<value_type> _allocator;
    compare _compare;

public:
    explicit binary_search_tree(
            const compare& comp = compare(),
            pp_allocator<value_type> walloc = pp_allocator<value_type>(),
            logger *logger = nullptr);

    explicit binary_search_tree(
            pp_allocator<value_type> alloc,
            const compare& comp = compare(),
            logger *logger = nullptr);


    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit binary_search_tree(iterator begin, iterator end, const compare& cmp = compare(), 
                                pp_allocator<value_type> alloc = pp_allocator<value_type>(), 
                                logger* logger = nullptr);

    // since c++23
    // struct from_range_t { explicit from_range_t() = default; };
    // constexpr inline std::from_range_t from_range {};
    // std::from_range_t - tag, std::from_range - object type of std::from_range_t
    // example: std::vector{1, 2, 3} - is std::ranges::input_range

    template<std::ranges::input_range Range>
    explicit binary_search_tree(Range&& range, const compare& cmp = compare(),
                                pp_allocator<value_type> alloc = pp_allocator<value_type>(),
                                logger* logger = nullptr);


    binary_search_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
                       pp_allocator<value_type> alloc = pp_allocator<value_type>(), 
                       logger* logger = nullptr);

public:
    
    binary_search_tree(const binary_search_tree &other);
    
    binary_search_tree(binary_search_tree &&other) noexcept;
    
    binary_search_tree &operator=(const binary_search_tree &other);
    
    binary_search_tree &operator=(binary_search_tree &&other) noexcept;
    
    virtual ~binary_search_tree();

public:

    tvalue& at(const tkey& key);
    const tvalue& at(const tkey& key) const;

    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    bool empty() const noexcept;

    size_t size() const noexcept;

    void clear() noexcept;

    std::pair<infix_iterator, bool> insert(const value_type&);
    std::pair<infix_iterator, bool> insert(value_type&&);

    template<std::input_iterator InputIt>
    void insert(InputIt first, InputIt last);

    template<std::ranges::input_range R>
    void insert_range( R&& rg );

    template<class ...Args>
    std::pair<infix_iterator, bool> emplace(Args&&...args);

    infix_iterator insert_or_assign(const value_type&);
    infix_iterator insert_or_assign(value_type&&);

    template<std::input_iterator InputIt >
    void insert_or_assign(InputIt first, InputIt last);


    template<class ...Args>
    infix_iterator emplace_or_assign(Args&&...args);

    virtual void swap(binary_search_tree& other) noexcept;

    bool contains(const tkey& key) const;

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

    size_t erase(const tkey& key);

public:
    
    // region iterators requests definition

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
    
    // endregion iterators requests definition

protected:



    // region subtree rotations definition
    
    static void small_left_rotation(node *&subtree_root) noexcept;

    static void small_right_rotation(node *&subtree_root) noexcept;

    static void big_left_rotation(node *&subtree_root) noexcept;

    static void big_right_rotation(node *&subtree_root) noexcept;

    static void double_left_rotation(node *&subtree_root) noexcept;

    static void double_right_rotation(node *&subtree_root) noexcept;
    
    // endregion subtree rotations definition
    
};

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare = std::less<tkey>, typename tag = BST_TAG>
    class bst_impl
    {
        friend class binary_search_tree<tkey, tvalue, compare, tag>;

        using node_t = binary_search_tree<tkey, tvalue, compare, tag>::node;
        template<class ...Args>
        static binary_search_tree<tkey, tvalue, compare, tag>::node* create_node(binary_search_tree<tkey, tvalue, compare, tag>& cont,
                                                                                 Args&& ...args);

        // Only calls destructor and frees memory
        static void delete_node(binary_search_tree<tkey, tvalue, compare, tag>& cont,
                                binary_search_tree<tkey, tvalue, compare, tag>::node*);

        static void clear_subtree(binary_search_tree<tkey, tvalue, compare, tag>& cont,
                                  binary_search_tree<tkey, tvalue, compare, tag>::node*);

        static void clear(binary_search_tree<tkey, tvalue, compare, tag>& cont);

        //Does not invalidate node*, needed for splay tree
        static void post_search(binary_search_tree<tkey, tvalue, compare, tag>::node**){}

        //Does not invalidate node*
        static void post_insert(binary_search_tree<tkey, tvalue, compare, tag>& cont,
                                binary_search_tree<tkey, tvalue, compare, tag>::node**){}

        // Removes this node from tree and deletes it
        static void erase(binary_search_tree<tkey, tvalue, compare, tag>& cont,
                          binary_search_tree<tkey, tvalue, compare, tag>::node**);

        static void swap(binary_search_tree<tkey, tvalue, compare, tag>& lhs,
                         binary_search_tree<tkey, tvalue, compare, tag>& rhs) noexcept;

    };


}










template<typename tkey, typename tvalue, typename compare, typename tag>
void __detail::bst_impl<tkey, tvalue, compare, tag>::swap(binary_search_tree<tkey, tvalue, compare, tag> &lhs,
                                                binary_search_tree<tkey, tvalue, compare, tag> &rhs) noexcept
{
    using std::swap;
    swap(lhs._root, rhs._root);
    swap(lhs._size, rhs._size);
    swap(lhs._allocator, rhs._allocator);
    swap(lhs._logger, rhs._logger);
    if constexpr (std::allocator_traits<decltype(lhs._allocator)>::propagate_on_container_swap::value) {
        swap(lhs._allocator, rhs._allocator);
    }
}



template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<input_iterator_for_pair<tkey, tvalue> iterator>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(iterator begin, iterator end, const compare &cmp,
                                                                   pp_allocator<typename binary_search_tree<tkey, tvalue, compare, tag>::value_type> alloc, logger *logger)
{
    throw not_implemented("template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>\n"
                          "template<input_iterator_for_pair<tkey, tvalue> iterator>\n"
                          "binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(iterator , iterator , const compare &,\n"
                          "pp_allocator<typename binary_search_tree<tkey, tvalue, compare, tag>::value_type> , logger *)", "your code should be here...");
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::compare_pairs(const binary_search_tree::value_type &lhs,
                                                              const binary_search_tree::value_type &rhs) const
{
    return (_compare(lhs.first, rhs.first) && _compare(lhs.second, rhs.second));
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return (compare{}(lhs, rhs));
}

template<typename compare, typename U, typename iterator>
explicit binary_search_tree(iterator begin, iterator end, const compare& cmp = compare(),
                            pp_allocator<U> alloc = pp_allocator<U>(),
                            logger* logger = nullptr) -> binary_search_tree<const typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare>;

template<typename compare, typename U, std::ranges::forward_range Range>
explicit binary_search_tree(Range&& range, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> binary_search_tree<const typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::first_type, typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::second_type, compare> ;

template<typename tkey, typename tvalue, typename compare, typename U>
binary_search_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> binary_search_tree<tkey, tvalue, compare>;


// region node implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<class ...Args>
binary_search_tree<tkey, tvalue, compare, tag>::node::node(node* parent, Args&& ...args)
        : parent(parent), data(std::forward<Args>(args)...) {}


// endregion node implementation

// region prefix_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::prefix_iterator(node* data)
{
    _data = data;
    _backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::prefix_iterator(node* data, node* backup) {
    _data = data;
    _backup = backup;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator==(
        prefix_iterator const &other) const noexcept
{
    return (_data == other._data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator!=(
        prefix_iterator const &other) const noexcept
{
    return !(*this == other);
}
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator++() & noexcept {
    if (_data->left_subtree) {
        _data = _data->left_subtree;
    } else if (_data->right_subtree) {
        _data = _data->right_subtree;
    } else {
        while (_data->parent) {
            node* parent = _data->parent;
            if (parent->left_subtree == _data && parent->right_subtree) {
                _data = parent->right_subtree;
                return *this;
            }
            _data = parent;

            if (_data == _backup) {
                return *this;
            }
        }

        // segfault
        _data = _backup;
    }
    return *this;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator++(int not_used) noexcept
{
    prefix_iterator copy = *this;
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator&
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator--() & noexcept
{
    if (_data == nullptr || _data == _backup) {
        // Переход к последнему элементу в префиксном порядке
        _data = _backup->right_subtree; // корень
        if (!_data) return *this;

        while (_data->right_subtree || _data->left_subtree) {
            if (_data->right_subtree) {
                _data = _data->right_subtree;
            } else {
                _data = _data->left_subtree;
            }
        }
        return *this;
    }

    // Если есть левый родитель, переходим к нему
    if (_data->parent && _data == _data->parent->right_subtree && _data->parent->left_subtree) {
        _data = _data->parent->left_subtree;
        while (_data->right_subtree || _data->left_subtree) {
            if (_data->right_subtree) {
                _data = _data->right_subtree;
            }
            else {
                _data = _data->left_subtree;
            }
        }
        return *this;
    }

    // Иначе просто поднимаемся к родителю
    if (_data->parent) {
        _data = _data->parent;
        return *this;
    }

    // Если родителя нет — достигли начала
    _data = _backup;
    return *this;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator--(int not_used) noexcept
{
    prefix_iterator copy = *this;
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator*()
{
    if (_data == _backup) {
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::null_pointer_dereference_attempt{};
    }
    return (_data->data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator->() noexcept
{
    return &(_data->data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::depth() const noexcept
{
    size_t depth = 0;
    node* cur = _data;
    while(cur && cur->parent) {
        ++depth;
        cur = cur->parent;
    }
    return depth - 1;
}

// endregion prefix_iterator implementation

// region prefix_const_iterator implementation

// const_iterator оборачивает обычный iterator, внутри хранит _base

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::prefix_const_iterator(const node* data)
{
    _base._data = const_cast<node*>(data);
    _base._backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::prefix_const_iterator(const node* data, const node* backup)
{
    _base._data = const_cast<node*>(data);
    _base._backup = const_cast<node*>(backup);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator==(
        prefix_iterator const &other) const noexcept
{
    return (_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator!=(
        prefix_iterator const &other) const noexcept
{
    return !(_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator++() & noexcept
{
    ++_base;
    return (*this);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator++(int not_used) noexcept
{
    prefix_const_iterator copy = *this;
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator--() & noexcept
{
    --_base;
    return (*this);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator--(int not_used) noexcept
{
    prefix_const_iterator copy = *this;
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator*() const
{
    if (_base._data == _base._backup) { // если end
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::null_pointer_dereference_attempt{};
    }
    return (*_base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion prefix_const_iterator implementation

// region prefix_reverse_iterator implementation

/* @example Arrow is common iterator
*  1 2 3 -> 4 5 6 7
*  *it == 4.
*
*  @example But reverse:
*  1 2 3 \<- 4 5 6 7
*  *rit == 3
*/


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::prefix_reverse_iterator(node* data)
    : _base(prefix_iterator(data)) {}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::prefix_reverse_iterator(node* data, node* backup)
    : _base(prefix_iterator(data, backup)) {}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::prefix_reverse_iterator(const prefix_iterator& it) noexcept :
    _base(it) {}


/* operator prefix_iterator -
 * prefix_reverse_iterator rit = ...;
 * prefix_iterator it = rit;
 * */

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::
operator typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator() const noexcept {
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::base() const noexcept {
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator==(prefix_reverse_iterator const &other) const noexcept
{
    return (_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator!=(prefix_reverse_iterator const &other) const noexcept
{
    return !(_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator++() & noexcept
{
    --_base;
    return (*this);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator++(int not_used) noexcept
{
    prefix_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return(*this);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator--(int not_used) noexcept
{
    prefix_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

// при разыменовании указывает на следующую по порядку обхода!
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator*()
{
    prefix_iterator tmp = _base;
    --tmp;
    return *tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator->() noexcept {
    prefix_iterator tmp = _base;
    --tmp;
    return tmp.operator->();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion prefix_reverse_iterator implementation

// region prefix_const_reverse_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(const node* data)
{
    _base._data = const_cast<node*>(data);
    _base._backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(const prefix_const_reverse_iterator& it) noexcept
{
    _base._base._data = it._base._base._data;
    _base._base._backup = it._base._base._backup;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::
operator typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator() const noexcept {
    return _base;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator==(prefix_const_reverse_iterator const &other) const noexcept
{
    return (_base == other.base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator!=(prefix_const_reverse_iterator const &other) const noexcept
{
    return !(this == other);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator++() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator++(int not_used) noexcept
{
    prefix_const_reverse_iterator tmp = _base;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator--() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator--(int not_used) noexcept
{
    prefix_const_reverse_iterator tmp = _base;
    --_base;
    return *this;
}

// при разыменовании указывает на следующую по порядку обхода!
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator*()
{
    prefix_const_iterator tmp = _base;
    --tmp;
    return *tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator->() noexcept
{
    prefix_const_iterator tmp = _base;
    --tmp;
    return tmp.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion prefix_const_reverse_iterator implementation

// region infix_iterator implementation
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::infix_iterator(node* data)
{
    _data = data;
    _backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::infix_iterator(node* data, node* backup)
{
    _data = data;
    _backup = backup;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator==(infix_iterator const &other) const noexcept
{
    return (_data == other._data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator!=(infix_iterator const &other) const noexcept
{
    return !(_data == other._data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator++() & noexcept
{
    if (_data == _backup) {
        return *this; // мы уже в end(), ничего не делаем
    }

    if (_data -> right_subtree) {
        _data = _data->right_subtree;
        while(_data && _data->left_subtree) {
            _data = _data->left_subtree;
        }
    } else {
        using node_t = binary_search_tree<tkey, tvalue, compare, tag>::node;
        node_t* parent = _data->parent;
        // Eсли текущий узел — правый сын, то его родитель уже был посещён ранее в инфиксном обходе
        while(parent && (_data == parent->right_subtree)) {
            _data = parent;
            parent = parent->parent;
        }
        _data = parent ? parent : _backup; // Eсли последний элемент, то в фиктивную ноду
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator++(int not_used) noexcept
{
    infix_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator--() & noexcept
{
    if (_data == _backup) { // end()
        _data = _backup;
        if (!_data) return *this;

        while (_data->right_subtree) {
            _data = _data->right_subtree;
        }
        return *this;
    }

    // Eсть левое поддерево — идём в него, потом вправо максимально
    if (_data->left_subtree) {
        _data = _data->left_subtree;
        while (_data->right_subtree) {
            _data = _data->right_subtree;
        }
        return *this;
    }

    // Иначе поднимаемся по дереву, пока не найдём узел, из правого которого пришли
    node* cur = _data;
    while (cur->parent && cur == cur->parent->left_subtree) {
        cur = cur->parent;
    }

    _data = cur->parent ? cur->parent : _backup;
    return *this;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator--(int not_used) noexcept
{
    infix_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator*()
{
    if (_data == _backup) {
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::null_pointer_dereference_attempt{};
    }
    return _data->data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator->() noexcept
{
    // возвращаемое значение - указатель, поэтому берем адрес
    return &(_data->data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::depth() const noexcept
{
    size_t depth = 0;
    node* cur = _data;
    while (cur && cur->parent) {
        ++depth;
        cur = cur->parent;
    }
    return depth - 1;

}

// endregion infix_iterator implementation

// region infix_const_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::infix_const_iterator(const node* data)
{
    _base._data = const_cast<node*>(data);
    _base._backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::infix_const_iterator(const node* data, const node* backup)
{
    _base._data = const_cast<node*>(data);
    _base._backup = const_cast<node*>(backup);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::infix_const_iterator(const infix_iterator& it) noexcept
{
    _base._data = it._data;
    _base._backup = it._backup;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator==(infix_const_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator!=(infix_const_iterator const &other) const noexcept
{
    return !(_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator++() & noexcept
{
    ++_base;
    return (*this);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator++(int not_used) noexcept
{
    infix_const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator--() & noexcept
{
    --_base;
    return (*this);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator--(int not_used) const noexcept
{
    infix_const_iterator tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator*()
{
    if (_base._data == _base._backup) { // если end
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::null_pointer_dereference_attempt{};
    }
    return *(_base._data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion infix_const_iterator implementation

// region infix_reverse_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::infix_reverse_iterator(node* data)
    : _base(infix_iterator(data)) {}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::infix_reverse_iterator(node* data, node* backup)
    : _base(infix_iterator(data, backup)) {}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::infix_reverse_iterator(const infix_iterator& it) noexcept
    : _base(it) {}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator==(infix_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator!=(infix_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator++(int not_used) noexcept
{
    infix_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator--(int not_used) const noexcept
{
    infix_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator*()
{
    infix_iterator tmp = _base;
    --tmp;
    return *tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator->() noexcept
{
    infix_iterator tmp = _base;
    --tmp;
    return tmp.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion infix_reverse_iterator implementation

// region infix_const_reverse_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::infix_const_reverse_iterator(const node* data) {
    _base._data = const_cast<node*>(data);
    _base._backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::infix_const_reverse_iterator(const infix_const_iterator& it) noexcept
{
    _base._base._data = it._base._base._data;
    _base._base._backup = it._base._base._backup;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator==(infix_const_reverse_iterator const &other) const noexcept
{
    return _base == other.base();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator!=(infix_const_reverse_iterator const &other) const noexcept
{
    return !_base != other.base();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator++(int not_used) noexcept
{
    infix_const_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator--(int not_used) const noexcept
{
    infix_const_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

// при разыменовании указывает на следующую по порядку обхода!
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator*()
{
    infix_const_iterator tmp = _base;
    --tmp;
    return *tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator->() noexcept
{
    infix_const_iterator tmp = _base;
    --tmp;
    return tmp.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion infix_const_reverse_iterator implementation

// region postfix_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::postfix_iterator(node* data)
    : _data(data), _backup(nullptr) {}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::postfix_iterator(node* data, node* backup)
        : _data(data), _backup(backup) {}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator==(postfix_iterator const &other) const noexcept
{
    return _data == other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator!=(postfix_iterator const &other) const noexcept
{
    return !(_data == other._data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator++() & noexcept
{
    /* 1. если _data == _backup -> итератор на end -> ничего не делаем

    2. если _data — это правый ребёнок, или у него нет родителя:
    -> поднимаемся вверх к родителю

    3. если _data — это левый ребёнок:
    -> если у родителя есть правое поддерево, идём в него
    и ищем первый по постфиксному (самый глубокий)
    -> если правого поддерева нет -> родитель — следующий
    */

    if (_data == _backup) {
        return *this; // мы уже в end(), ничего не делаем
    }

    node* parent = _data->parent;

    // если текущий узел — правый сын, или у родителя нет правого
    if (!parent || parent->right_subtree == _data || !parent->right_subtree) {
        _data = parent ? parent : _backup;
    } else if (parent->left_subtree == _data && parent->right_subtree) {
        // если мы пришли из левого и есть правое поддерево — идём в него
        _data = parent->right_subtree;

        // идём вниз: постфикс -> в самый левый/правый глубоко
        while (_data->left_subtree || _data->right_subtree) {
            if (_data->left_subtree) {
                _data = _data->left_subtree;
            } else {
                _data = _data->right_subtree;
            }
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator++(int not_used) noexcept
{
    postfix_iterator tmp = *this;
    ++(*this);
    return tmp;
}

// TODO: fix this!
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator--() & noexcept
{
    if (_data == _backup) {
        // мы в end()
        _data = _backup->right_subtree; // корень
        return *this;
    }

    node* parent = _data->parent;

    if (!parent) {
        _data = _backup;
        return *this;
    }

    if (parent->left_subtree == _data && !parent->right_subtree) {
        _data = _data->left_subtree;
    } else if (parent->left_subtree == _data && parent->right_subtree) {
        while(_data && _data->right_subtree) {
            _data = _data->right_subtree;
        }
    }
    /*if (parent->right_subtree == _data) {
        // если текущий узел — правый ребёнок
        if (parent->left_subtree) {
            // переходим в левое поддерево и идём до самого глубокого правого/левого
            _data = parent->left_subtree;
            while (_data->right_subtree || _data->left_subtree) {
                if (_data->right_subtree)
                    _data = _data->right_subtree;
                else
                    _data = _data->left_subtree;
            }
        } else {
            // eсли левого поддерева нет — предыдущий элемент — родитель
            _data = parent;
        }
    } else {
        // eсли текущий узел — левый ребёнок, предыдущий элемент — родитель
        _data = parent;
    }*/

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator--(int not_used) const noexcept
{
    postfix_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator*()
{
    if (_data == _backup) {
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::null_pointer_dereference_attempt{};
    }
    return _data->data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator->() noexcept
{
    // возвращаемое значение - указатель, поэтому берем адрес
    return &(_data->data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::depth() const noexcept
{
    size_t depth = 0;
    node* cur = _data;
    while (cur && cur->parent) {
        ++depth;
        cur = cur->parent;
    }
    return depth - 1;
}

// endregion postfix_iterator implementation

// region postfix_const_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::postfix_const_iterator(const node* data)
{
    _base._data = const_cast<node*>(data);
    _base._backup = nullptr;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::postfix_const_iterator(const postfix_iterator& it) noexcept
{
    _base._data = const_cast<node*>(it._data);
    _base._backup = const_cast<node*>(it._backup);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::postfix_const_iterator(const node* data, const node* backup) {
    _base._data = const_cast<node*>(data);
    _base._backup = const_cast<node*>(backup);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator==(postfix_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator!=(postfix_iterator const &other) const noexcept
{
    return !(_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator++() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator++(int not_used) noexcept
{
    postfix_const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator--() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator--(int not_used) const noexcept
{
    postfix_const_iterator tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator*()
{
    return (_base._data);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion postfix_const_iterator implementation

// region postfix_reverse_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::postfix_reverse_iterator(node* data)
    : _base(postfix_iterator(data)) {}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::postfix_reverse_iterator(node* data, node* backup)
        : _base(postfix_iterator(data, backup)) {}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::postfix_reverse_iterator(const postfix_iterator& it) noexcept
    : _base(it) {}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator==(postfix_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator!=(postfix_reverse_iterator const &other) const noexcept
{
    return !(_base == other._base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator++(int not_used) noexcept
{
    postfix_reverse_iterator tmp = *this;
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator--(int not_used) const noexcept
{
    postfix_reverse_iterator tmp = *this;
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator*()
{
    postfix_iterator tmp = _base;
    --tmp;
    return *tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator->() noexcept
{
    postfix_iterator tmp = _base;
    --tmp;
    return tmp.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion postfix_reverse_iterator implementation

// region postfix_const_reverse_iterator implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(const node* data)
{
    _base._data = const_cast<node*>(data);
    _base._backup = nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(const postfix_const_iterator& it) noexcept
{
    _base._base._data = it._base._base._data;
    _base._base._backup = it._base._base._backup;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator==(postfix_const_reverse_iterator const &other) const noexcept
{
    return (_base == other.base);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator!=(postfix_const_reverse_iterator const &other) const noexcept
{
    return !(this == other);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator++() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator++(int not_used) noexcept
{
    postfix_const_reverse_iterator tmp = _base;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator--() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator--(int not_used) const noexcept
{
    postfix_const_reverse_iterator tmp = _base;
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator*()
{
    postfix_const_iterator tmp = _base;
    --tmp;
    return *tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator->() noexcept
{
    postfix_const_iterator tmp = _base;
    --tmp;
    return tmp.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

// endregion postfix_const_reverse_iterator implementation

// region binary_search_tree implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
        const compare& comp,
        pp_allocator<value_type> alloc,
        logger *logger)
        : _root(nullptr),
          _fake_node(nullptr),
          _logger(logger),
          _size(0),
          _allocator(std::move(alloc))
{
    this->_compare = comp;
    _fake_node = _allocator.template new_object<node>(nullptr, value_type{});
}



template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
        pp_allocator<value_type> alloc,
        const compare& comp,
        logger *logger)
        : _root(nullptr),
          _fake_node(nullptr),
          _logger(logger),
          _size(0),
          _allocator(std::move(alloc))
{
    this->_compare = comp;  // compare is a class, not a field
    _fake_node = _allocator.template new_object<node>(nullptr, value_type{});
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::ranges::input_range Range>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
        Range&& range,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger) :
        _root(nullptr),
        _fake_node(nullptr),
        _logger(logger),
        _size(0),
        _allocator(std::move(alloc))
{
    this->_compare = cmp;

    _fake_node = _allocator.template new_object<node>(nullptr, value_type{});

    for (auto&& val : range) {
        insert(std::forward<decltype(val)>(val));
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger)
        : _root(nullptr),
          _fake_node(nullptr),
          _logger(logger),
          _size(0),
          _allocator(std::move(alloc))
{
    this->_compare = cmp;

    _fake_node = _allocator.template new_object<node>(nullptr, value_type{});

    for (const auto& val : data) {
        insert(val);
    }
}


// endregion binary_search_tree implementation

// region binary_search_tree 5_rules implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(const binary_search_tree& other)
        : _root(nullptr),
          _fake_node(nullptr),
          _logger(other._logger),
          _size(0),
          _allocator(other._allocator)
{
    this->_compare = other._compare;

    _fake_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, nullptr, value_type{});

    try {
        if (other._root) {
            _root = __detail::bst_impl<tkey, tvalue, compare, tag>::copy_subtree(*this, other._root, nullptr);

            _fake_node->right_subtree = _root;
            _root->parent = _fake_node;
        }
    } catch (...) {
        // очищаем всё, если что-то пошло не так
        this->clear();
        if (_fake_node) {
            _allocator.template delete_object(_fake_node);
            _fake_node = nullptr;
        }
        throw;
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(binary_search_tree&& other) noexcept
        : _root(other._root),
          _fake_node(other._fake_node),
          _logger(other._logger),
          _size(other._size)
{
    if constexpr (pp_allocator<value_type>::propagate_on_container_move_assignment::value) {
        _allocator = std::move(other._allocator);
    } else {
        _allocator = pp_allocator<value_type>{}; // или оставить прежний
    }

    other._root = nullptr;
    other._fake_node = nullptr;
    other._logger = nullptr;
    other._size = 0;
}



template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>&
binary_search_tree<tkey, tvalue, compare, tag>::operator=(const binary_search_tree& other)
{
    if (this == &other) return *this;

    binary_search_tree temp(other);

    this->swap(temp);

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>&
binary_search_tree<tkey, tvalue, compare, tag>::operator=(binary_search_tree&& other) noexcept
{
    if (this == &other) return *this;

    this->clear();

    if constexpr (pp_allocator<value_type>::propagate_on_container_move_assignment::value) {
        _allocator = std::move(other._allocator);
    }

    _root = other._root;
    _fake_node = other._fake_node;
    _logger = other._logger;
    _size = other._size;

    other._root = nullptr;
    other._fake_node = nullptr;
    other._size = 0;

    return *this;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::~binary_search_tree()
{
    clear();
}

// endregion binary_search_tree 5_rules implementation

// region binary_search_tree methods_access implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
tvalue& binary_search_tree<tkey, tvalue, compare, tag>::at(const tkey& key)
{
    infix_iterator it = find(key);
    if (it == end()) {
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::not_found{};
    }
    return it->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
const tvalue& binary_search_tree<tkey, tvalue, compare, tag>::at(const tkey& key) const
{
    infix_const_iterator it = static_cast<const binary_search_tree*>(this)->find(key);
    if (it == end()) {
        throw typename binary_search_tree<tkey, tvalue, compare, tag>::not_found{};
    }
    return it->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
tvalue& binary_search_tree<tkey, tvalue, compare, tag>::operator[](const tkey& key)
{
    node* cur = _root;
    while(cur) {
        if (key > cur->data.first && cur->right_subtree) {
            cur = cur->right_subtree;
        } else if (key < cur->data.first && cur->left_subtree) {
            cur = cur->left_subtree;
        } else if (key == cur->data.first) {
            return cur->data.second;
        } else {
            break;
        }
    }
    // tvalue по умолчанию
    auto res = insert({key, tvalue{}});
    return res.first->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
tvalue& binary_search_tree<tkey, tvalue, compare, tag>::operator[](tkey&& key)
{
    node* cur = _root;
    while(cur) {
        if (key > cur->data.first && cur->right_subtree) {
            cur = cur->right_subtree;
        } else if (key < cur->data.first && cur->left_subtree) {
            cur = cur->left_subtree;
        } else if (key == cur->data.first) {
            return cur->data.second;
        } else {
            break;
        }
    }
    // tvalue по умолчанию
    auto res = insert(std::pair<tkey, tvalue>{std::move(key), tvalue{}});
    return res.first->second;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::empty() const noexcept
{
    return (_size == 0);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::clear() noexcept
{
    __detail::bst_impl<tkey, tvalue, compare, tag>::clear(*this);
}

// endregion binary_search_tree methods_access implementation

// region binary_search_tree methods_insert and methods_emplace implementation

// TODO: add post
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool>
binary_search_tree<tkey, tvalue, compare, tag>::insert(const value_type& value_to_insert)
{
    // return-type
    //    Пара, состоящая из итератора к вставленному элементу
    //   (или к элементу, который помешал вставке)
    //   и значения bool, равного true, если вставка произошла.

    tkey key = value_to_insert.first;
    tvalue value = value_to_insert.second;

    node* parent = _fake_node;
    node* current = _root;

    while(current != nullptr) {
        parent = current;
        if (_compare(current->data.first, key)) {
            current = current->right_subtree;
        } else if (_compare(key, current->data.first)) {
            current = current->left_subtree;
        } else {
            return {infix_iterator(current), false}; // такой элемент уже существует
        }
    }

    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, value_to_insert);
    // parent - к кому надо прикрепить новый
    if (parent == _fake_node) {
        _root = new_node;
        _fake_node->right_subtree = _root;
        _root->parent = _fake_node;
    } else if (_compare(new_node->data.first, parent->data.first)) {
        parent->left_subtree = new_node;
        new_node->parent = parent;
    } else {
        parent->right_subtree = new_node;
        new_node->parent = parent;
    }

    ++_size;

    __detail::bst_impl<tkey, tvalue, compare, tag>::post_insert(*this, &new_node);
    return {infiix_iterator(new_node), true};
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool>
binary_search_tree<tkey, tvalue, compare, tag>::insert(value_type&& value_to_insert)
{
// return-type
    //    Пара, состоящая из итератора к вставленному элементу
    //   (или к элементу, который помешал вставке)
    //   и значения bool, равного true, если вставка произошла.

    tkey key = value_to_insert.first;
    tvalue value = value_to_insert.second;

    node* parent = _fake_node;
    node* current = _root;

    while(current) {
        parent = current;
        if (_compare(current->data.first, key)) {
            current = current->right_subtree;
        } else if (_compare(key, current->data.first)) {
            current = current->left_subtree;
        } else {
            return {infix_iterator(current, _fake_node), false}; // такой элемент уже существует
        }
    }

    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, std::move(value_to_insert));
    // parent - к кому надо прикрепить новый
    if (parent == _fake_node) {
        _root = new_node;
        _fake_node->right_subtree = _root;
        _root->parent = _fake_node;
        _root->right_subtree = nullptr;
        _root->left_subtree = nullptr;
    } else if (_compare(new_node->data.first, parent->data.first)) {
        parent->left_subtree = new_node;
        new_node->parent = parent;
        new_node->right_subtree = nullptr;
        new_node->left_subtree = nullptr;
    } else {
        parent->right_subtree = new_node;
        new_node->parent = parent;
        new_node->right_subtree = nullptr;
        new_node->left_subtree = nullptr;
    }

    ++_size;

    return {infix_iterator(new_node, _fake_node), true};
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::input_iterator InputIt>
void binary_search_tree<tkey, tvalue, compare, tag>::insert(InputIt first, InputIt last)
{
    for (; first != last; ++first) {
        insert(*first);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::ranges::input_range R>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_range(R&& rg)
{
    insert(std::ranges::begin(rg), std::ranges::end(rg));
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<class ...Args>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool>
binary_search_tree<tkey, tvalue, compare, tag>::emplace(Args&&... args)
{
    // forward args into a value_type (std::pair<tkey, tvalue>)
    value_type value(std::forward<Args>(args)...);

    std::pair<infix_iterator, bool> it = insert(std::move(value));
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(const value_type& value)
{
    // Если ключ, эквивалентный k, уже существует в контейнере, замена значения
    // Если ключа не существует, вставляет новое значение

    // return value
    // Компонент итератора указывает на элемент, который был вставлен или обновлен.

    node* cur = _root;
    while (cur) {
        if (_compare(value.first, cur->data.first) && cur->left_subtree) {
            cur = cur->left_subtree;
        } else if (_compare(cur->data.first, value.first) && cur->right_subtree) {
            cur = cur->right_subtree;
        } else if (!_compare(cur->data.first, value.first) && !_compare(value.first,cur->data.first)) {
            cur->data.second = value.second;
            return infix_iterator(cur, _fake_node);
        } else {
            break;
        }
    }

    auto it = insert(value);
    return it.first;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(value_type&& value)
{
    // Если ключ, эквивалентный k, уже существует в контейнере, замена значения
    // Если ключа не существует, вставляет новое значение

    // return value
    // Компонент итератора указывает на элемент, который был вставлен или обновлен.

    node* cur = _root;
    while (cur) {
        if (_compare(value.first, cur->data.first) && cur->left_subtree) {
            cur = cur->left_subtree;
        } else if (_compare(cur->data.first, value.first) && cur->right_subtree) {
            cur = cur->right_subtree;
        } else if (!_compare(cur->data.first, value.first) && !_compare(value.first,cur->data.first)) {
            cur->data.second = std::move(value.second);
            return infix_iterator(cur, _fake_node);
        } else {
            break;
        }
    }

    auto it = insert(std::move(value));
    return it.first;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::input_iterator InputIt>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(InputIt first, InputIt last)
{
    for (; first != last; ++first) {
        insert(*first);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<class ...Args>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::emplace_or_assign(Args&&... args) {
    // forward args into a value_type (std::pair<tkey, tvalue>)
    value_type value(std::forward<Args>(args)...);

    infix_iterator it = insert_or_assign(std::move(value));
    return it;
}

// endregion binary_search_tree methods_insert and methods_emplace implementation

// region binary_search_tree swap_method implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::swap(binary_search_tree& other) noexcept
{
    using std::swap;

    swap(this->_root, other._root);
    swap(this->_size, other._size);
    swap(this->_fake_node, other._fake_node);
    swap(this->_logger, other._logger);

    if constexpr (std::allocator_traits<decltype(_allocator)>::propagate_on_container_swap::value) {
        swap(this->_allocator, other._allocator);
    }
}

// endregion binary_search_tree swap_method implementation

// region binary_search_tree methods_search and methods_erase implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::contains(const tkey& key) const
{
    infix_const_iterator it = find(key);
    if (it == end()) {
        return false;
    }
    return true;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::find(const tkey& key)
//  Return value
// An iterator to the requested element.
// If no such element is found, past-the-end (see end()) iterator is returned.
{
    node* cur = _root;
    while (cur != _fake_node) {
        if (_compare(cur->data.first, key)) {
            // key > cur->data.first
            if (cur->right_subtree != _fake_node) {
                cur = cur->right_subtree;
            } else {
                return infix_iterator(_fake_node);
            }
        } else if (_compare(key, cur->data.first)) {
            // key < cur->data.first
            if (cur->left_subtree != _fake_node)
                cur = cur->left_subtree;
            else
                break;
        } else {
            // key == cur->data.first
            return infix_iterator(cur, _fake_node);
        }
    }

    __detail::bst_impl<tkey, tvalue, compare, tag>::post_search(&cur);
    return infix_iterator(_fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::find(const tkey& key) const
{
    node* cur = _root;
    while (cur != _fake_node) {
        if (_compare(cur->data.first, key)) {
            // key > cur->data.first
            if (cur->right_subtree != _fake_node) {
                cur = cur->right_subtree;
            }
            else {
                return infix_const_iterator(_fake_node);
            }
        } else if (_compare(key, cur->data.first)) {
            // key < cur->data.first
            if (cur->left_subtree != _fake_node)
                cur = cur->left_subtree;
            else
                break;
        } else {
            // key == cur->data.first
            return infix_const_iterator(cur, _fake_node);
        }
    }

    __detail::bst_impl<tkey, tvalue, compare, tag>::post_search(&cur);

    return infix_const_iterator(_fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::lower_bound(const tkey& key)
{
    node* cur = _root;
    node* res = nullptr;

    while (cur) {
        if (_compare(cur->data.first, key)) {
            cur = cur->right_subtree;
        } else {
            res = cur;
            cur = cur->left_subtree;
        }
    }

    return res ? infix_iterator(res, _fake_node) : end();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::lower_bound(const tkey& key) const
{
    node* cur = _root;
    node* res = nullptr;

    while (cur) {
        if (_compare(cur->data.first, key)) {
            cur = cur->right_subtree;
        } else {
            res = cur;
            cur = cur->left_subtree;
        }
    }

    return res ? infix_const_iterator(res, _fake_node) : end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::upper_bound(const tkey& key)
{
    node* cur = _root;
    node* res = nullptr;

    while (cur) {
        if (_compare(key, cur->data.first)) {
            // key < cur.key -> потенциальный ответ, идём влево
            res = cur;
            cur = cur->left_subtree;
        } else {
            // key >= cur.key -> точно не подходит, идём вправо
            cur = cur->right_subtree;
        }
    }

    return res ? infix_iterator(res, _fake_node) : end();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::upper_bound(const tkey& key) const
{
    node* cur = _root;
    node* res = nullptr;

    while (cur) {
        if (_compare(key, cur->data.first)) {
            // key < cur.key -> потенциальный ответ, идём влево
            res = cur;
            cur = cur->left_subtree;
        } else {
            // key >= cur.key -> точно не подходит, идём вправо
            cur = cur->right_subtree;
        }
    }

    return res ? infix_const_iterator(res, _fake_node) : end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::erase(infix_iterator pos)
{
    __detail::bst_impl<tkey, tvalue, compare, tag> erase(*this, &pos._data->data);
    ++pos;
    return pos;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::erase(infix_const_iterator pos)
{
    __detail::bst_impl<tkey, tvalue, compare, tag> erase(*this, &pos._data->data);
    ++pos;
    return pos;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::erase(infix_iterator first, infix_iterator last)
{
    for (; first != last; ++first) {
        __detail::bst_impl<tkey, tvalue, compare, tag> erase(*this, &first._data->data);
    }
    ++first;
    return first;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::erase(infix_const_iterator first, infix_const_iterator last)
{
    for (; first != last; ++first) {
        __detail::bst_impl<tkey, tvalue, compare, tag> erase(*this, &first._data->data);
    }
    ++first;
    return first;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::erase(const tkey& key)
{
    auto it = find(key);
    if (it == end()) return 0;
    __detail::bst_impl<tkey, tvalue, compare, tag>::erase(*this, &it._data);
    return 1;
}

// endregion binary_search_tree methods_search and methods_erase implementation

// region infix_iterators requests implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin() noexcept
{
    node* cur = _root;
    while (cur && cur->left_subtree) {
        cur = cur->left_subtree;
    }
    return infix_iterator(cur, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end() noexcept
{
    return infix_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin() const noexcept
{
    node* cur = _root;
    while (cur && cur->left_subtree) {
        cur = cur->left_subtree;
    }
    return infix_const_iterator(cur, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end() const noexcept
{
    return infix_const_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin() const noexcept
{
    return begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend() const noexcept
{
    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin() noexcept
{
    return infix_reverse_iterator(end());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend() noexcept
{
    return infix_reverse_iterator(begin());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin() const noexcept
{
    return infix_const_reverse_iterator(cend());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend() const noexcept
{
    return infix_const_reverse_iterator(cbegin());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin() const noexcept
{
    return infix_const_reverse_iterator(cend());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend() const noexcept
{
    return infix_const_reverse_iterator(cbegin());
}

// endregion infix_iterators requests implementation

// region prefix_iterators requests implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_prefix() noexcept
{
    return prefix_iterator(_root, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_prefix() noexcept
{
    return prefix_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_prefix() const noexcept
{
    return prefix_const_iterator(_root, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_prefix() const noexcept
{
    return prefix_const_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin_prefix() const noexcept
{
    return prefix_const_iterator(_root, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend_prefix() const noexcept
{
    return prefix_const_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_prefix() noexcept {
    return prefix_reverse_iterator(end_prefix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_prefix() noexcept {
    return prefix_reverse_iterator(begin_prefix());
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_prefix() const noexcept
{
    return prefix_const_reverse_iterator(end_prefix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_prefix() const noexcept
{
    return prefix_const_reverse_iterator(begin_prefix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin_prefix() const noexcept
{
    return prefix_const_reverse_iterator(end_prefix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend_prefix() const noexcept
{
    return prefix_const_reverse_iterator(begin_prefix());
}

// endregion prefix_iterators requests implementation

// region infix_iterators methods implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_infix() noexcept
{
    return begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_infix() noexcept
{
    return end();
}

// TODO: check this
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_infix() const noexcept
{
    return cbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_infix() const noexcept
{
    return cend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin_infix() const noexcept
{
    return cbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend_infix() const noexcept
{
    return cend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_infix() noexcept
{
    return rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_infix() noexcept
{
    return rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_infix() const noexcept
{
    return crbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_infix() const noexcept
{
    return crend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin_infix() const noexcept
{
    return crbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend_infix() const noexcept
{
    return crend();
}

// endregion infix_iterators methods implementation

// region postfix_iterators requests implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_postfix() noexcept
{
    node* cur = _root;
    while (cur->left_subtree || cur->right_subtree) {
        if (cur->left_subtree) {
            cur = cur->left_subtree;
        } else if (cur->right_subtree) {
            cur = cur->right_subtree;
        }
    }
    return postfix_iterator(cur, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_postfix() noexcept
{
    return postfix_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_postfix() const noexcept
{
    return postfix_const_iterator(_root, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_postfix() const noexcept
{
    return postfix_const_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin_postfix() const noexcept
{
    node* cur = _root;
    while (cur->left_subtree || cur->right_subtree) {
        if (cur->left_subtree) {
            cur = cur->left_subtree;
        } else if (cur->right_subtree) {
            cur = cur->right_subtree;
        }
    }
    return postfix_const_iterator(cur, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend_postfix() const noexcept
{
    return postfix_const_iterator(_fake_node, _fake_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_postfix() noexcept
{
    return postfix_reverse_iterator(end_postfix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_postfix() noexcept
{
    return postfix_reverse_iterator(begin_postfix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_postfix() const noexcept
{
    return postfix_const_reverse_iterator(end_postfix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_postfix() const noexcept
{
    return postfix_const_reverse_iterator(begin_postfix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin_postfix() const noexcept
{
    return postfix_const_reverse_iterator(end_postfix());
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend_postfix() const noexcept
{
    return postfix_const_reverse_iterator(begin_postfix());
}

// endregion postfix_iterators requests implementation

// endregion iterators requests implementation

//region subtree rotations implementation

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::small_left_rotation(node *&subtree_root) noexcept
{
    /*
     *     x                 y
     *  t1    y ----->   x       t3
     *     t2   t3     t1  t2
     *
     */
    node* y = subtree_root->right_subtree;
    node* x = subtree_root;
    x->right_subtree = y->left_subtree;
    if (y->left_subtree)
        y->left_subtree->parent = x;

    y->parent = x->parent;

    if (x->parent) {
        if (x->parent->left_subtree == x)
            x->parent->left_subtree = y;
        else
            x->parent->right_subtree = y;
    }

    y->left_subtree = x;
    x->parent = y;

    x = y;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::small_right_rotation(node *&subtree_root) noexcept
{
    /*
     *        x                          y
     *     y      t3      ---->     t1        x
     *  t1   t2                            t2    t3
     *
     */

    node* y = subtree_root->left_subtree;
    node* x = subtree_root;
    x->left_subtree = y->right_subtree;
    if (y->right_subtree)
        y->right_subtree->parent = x;

    y->parent = x->parent;

    if (x->parent) {
        if (x->parent->left_subtree == x)
            x->parent->left_subtree = y;
        else
            x->parent->right_subtree = y;
    }

    y->right_subtree = x;
    x->parent = y;

    x = y;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::big_left_rotation(node *&subtree_root) noexcept
{
    /*
     *            x                                x                                       z
     *        t1       y       small right y   t4         z          small left x     x          y
     *             z       t1   ---------->          t3        y      --------->   t4    t3   t2   t1
     *         t3      t2                                 t2       t1
     */

    small_right_rotation(subtree_root->right_subtree);
    small_left_rotation(subtree_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::big_right_rotation(node *&subtree_root) noexcept
{
    /*
     *         x          small left y            x              small right x            z
     *     y       t4    ------------->        z        t4       ------------>       y          x
     * t1     z                             y     t3                              t1    t2   t3   t4
     *     t2  t3                        t1   t2
     */
    small_left_rotation(subtree_root->left_subtree);
    small_right_rotation(subtree_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::double_left_rotation(node *&subtree_root) noexcept
{
    small_left_rotation(subtree_root);
    small_left_rotation(subtree_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::double_right_rotation(node *&subtree_root) noexcept
{
    small_right_rotation(subtree_root);
    small_right_rotation(subtree_root);
}

//endregion subtree rotations implementation

namespace __detail {
    template<typename tkey, typename tvalue, typename compare, typename tag>
    template<typename ...Args>
    typename binary_search_tree<tkey, tvalue, compare, tag>::node*
    bst_impl<tkey, tvalue, compare, tag>::create_node(binary_search_tree<tkey, tvalue, compare, tag>& cont, Args&& ...args)
    {
        using tree_t = binary_search_tree<tkey, tvalue, compare, tag>;
        using node_t = typename tree_t::node;

        try {
            //  `template` требуется, так как new_object шаблонный метод,
            // компилятору нужно явно сказать, что это шаблон
            node_t* new_node = cont._allocator.template new_object<node_t>(std::forward<Args>(args)...);
            return new_node;
        } catch (...) {
            throw typename tree_t::allocation_error{};
        }

    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::delete_node(binary_search_tree<tkey, tvalue, compare, tag>& cont, binary_search_tree<tkey, tvalue, compare, tag>::node* to_delete)
    {
        using tree_t = binary_search_tree<tkey, tvalue, compare, tag>;
        using node_t = typename tree_t::node;

        if (!to_delete) {
            return;
        }

        cont._allocator.template destroy<node_t>(to_delete);

    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::erase(binary_search_tree<tkey, tvalue, compare, tag>& cont, typename binary_search_tree<tkey, tvalue, compare, tag>::node** node_ptr)
    {
        using tree_t = binary_search_tree<tkey, tvalue, compare, tag>;
        using node_t = typename tree_t::node;

        if (!node_ptr) {
            throw typename tree_t::null_pointer_dereference_attempt{};
        }

        node_t* to_delete = *node_ptr;

        if (!to_delete) {
            throw typename tree_t::not_found{};
        }

        if (!to_delete->left_subtree && !to_delete->right_subtree) {
            if (to_delete->parent == cont._fake_node) { // это корень
                cont._root = cont._fake_node;
            } else if (to_delete->parent->left_subtree == to_delete) {
                to_delete->parent->left_subtree = nullptr;
            } else {
                to_delete->parent->right_subtree = nullptr;
            }
            delete_node(cont, to_delete);
            cont._size--;
            return;
        }

        node_t* parent = to_delete->parent;

        if (!to_delete->right_subtree && to_delete->left_subtree) { // только левый ребенок
            if (parent == cont._fake_node) {
                cont._root = to_delete->left_subtree;
                to_delete->left_subtree->parent = cont._fake_node;
            } else if (parent->left_subtree == to_delete) {
                parent->left_subtree = to_delete->left_subtree;
                to_delete->left_subtree->parent = parent;
            } else if (parent->right_subtree == to_delete) {
                parent->right_subtree = to_delete->left_subtree;
                to_delete->left_subtree->parent = parent;
            }
            delete_node(cont, to_delete);
            return;

        } else if (to_delete->right_subtree && !to_delete->left_subtree) { // только правый ребенок
            if (parent == cont._fake_node) {
                cont._root = to_delete->right_subtree;
                to_delete->right_subtree->parent = cont._fake_node;
            } else if (parent->left_subtree == to_delete) {
                parent->left_subtree = to_delete->right_subtree;
                to_delete->right_subtree->parent = parent;
            } else if (parent->right_subtree == to_delete) {
                parent->right_subtree = to_delete->right_subtree;
                to_delete->right_subtree->parent = parent;
            }
            delete_node(cont, to_delete);
            return;
        } else if (to_delete->right_subtree && to_delete->left_subtree) {
            node_t* min_node = to_delete->left_subtree;

            while(min_node->right_subtree) {
                min_node = min_node->right_subtree;
            }

            node_t* node_to_replace = create_node(cont, min_node->parent, min_node->data);

            if (to_delete->left_subtree == min_node) {
                node_to_replace->left_subtree = min_node->left_subtree;
            } else {
                node_to_replace->left_subtree = to_delete->left_subtree;
            }

            node_to_replace->right_subtree = to_delete->right_subtree;

            if (node_to_replace->left_subtree) {
                node_to_replace->left_subtree->parent = node_to_replace;
            }

            if (node_to_replace->right_subtree) {
                node_to_replace->right_subtree->parent = node_to_replace;
            }

            // если у min_node есть ребенок (а он может быть только левым), нужно зацепить в правое поддерево
            if (min_node->left_subtree) {
                node_to_replace->left_subtree->right_subtree = min_node->left_subtree;
                min_node->left_subtree->parent = node_to_replace->left_subtree->right_subtree;
            }
            erase(cont, &min_node);

            if (to_delete == cont._root) {
                cont._root = node_to_replace;
            } else if (parent->left_subtree == to_delete) {
                parent->left_subtree = node_to_replace;
            } else {
                parent->right_subtree = node_to_replace;
            }

            node_to_replace->parent = parent;


            delete_node(cont, to_delete);

            return;
        }

    }



    template <typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::clear_subtree(binary_search_tree<tkey, tvalue, compare, tag> &cont,
                                                             typename binary_search_tree<tkey, tvalue, compare, tag>::node* node) {
        if (node == nullptr) {
            return;
        }

        clear_subtree(cont, node->left_subtree);
        clear_subtree(cont, node->right_subtree);

        delete_node(cont, node);
    }

    template <typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::clear(binary_search_tree<tkey, tvalue, compare, tag>& cont) {
        clear_subtree(cont,cont._root);
        delete_node(cont, cont._fake_node);
        cont._size = 0;
        cont._root = nullptr;
    }

    template <typename tkey, typename tvalue, typename compare, typename tag>
    binary_search_tree<tkey, tvalue, compare, tag>::node& find(binary_search_tree<tkey, tvalue, compare, tag>& cont,
                                                               const tkey& key) {
        using node_t = binary_search_tree<tkey, tvalue, compare, tag>::node;
        compare comp{};

        node_t* current = cont._root;
        while (current != nullptr) {
            if (!comp(key, current->data.first) && !comp(current->data.first, key)) {
                return *current;
            }

            if (comp(key, current->data.first)) {
                current = current->left_subtree;
            } else {
                current = current->right_subtree;
            }
        }

        throw typename binary_search_tree<tkey, tvalue, compare, tag>::not_found {};

    }


}

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H