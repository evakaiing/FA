#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H

#include <pp_allocator.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>

class allocator_red_black_tree final:
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:

    enum class block_color : unsigned char
    { RED, BLACK };

    struct block_data
    {
        bool occupied : 4;
        block_color color : 4;
    };

    void *_trusted_memory;

    static constexpr const size_t allocator_metadata_size = sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);
    static constexpr const size_t occupied_block_metadata_size = sizeof(block_data) + 3 * sizeof(void*); // parent, left, right
    static constexpr const size_t free_block_metadata_size = sizeof(block_data) + 5 * sizeof(void*); // parent, left, right, next_free, prev_free

public:
    
    ~allocator_red_black_tree() override;
    
    allocator_red_black_tree(
        allocator_red_black_tree const &other);
    
    allocator_red_black_tree &operator=(
        allocator_red_black_tree const &other);
    
    allocator_red_black_tree(
        allocator_red_black_tree &&other) noexcept;
    
    allocator_red_black_tree &operator=(
        allocator_red_black_tree &&other) noexcept;

public:
    
    explicit allocator_red_black_tree(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            logger *logger = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t size) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource&) const noexcept override;

    size_t calculate_available_memory() const;

    std::string get_blocks_state() const;
    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;

private:
    void fix_double_black(void* x);
    void* minimum(void* node) const;
    void transplant(void* u, void* v);
    void remove_from_tree(void* z);
    void* merge_blocks(void* block);
    void* split_blocks(void* block, size_t requested_size, size_t block_size);

    bool is_block_belongs_to_allocator(void* block) const;

    void right_rotate(void* y);
    void left_rotate(void* x);

    void mark_block_as_free(void* block);
    void mark_block_as_occupied(void* block);
    bool is_block_occupied(void* block) const;

    void set_block_size(void* block, size_t size);
    size_t get_block_size(void* block) const;
    void set_prev_free(void* block, void* prev_free);
    void* get_prev_free(void* block) const;
    void set_next_free(void* block, void* next_free);
    void* get_next_free(void* block) const;
    void set_right_child(void* block, void* right_child);
    void* get_right_child(void* block) const;
    void set_color(void* block, block_color color);
    block_color get_color(void* block) const;
    void* get_parent(void* block) const;
    void set_parent(void* block, void* parent);
    void* get_left_child(void* block) const;
    void set_left_child(void* block, void* left_child);

    inline void set_fit_mode(allocator_with_fit_mode::fit_mode mode) override;
    inline logger *get_logger() const override;
    void set_tree_root(void* root);
    void* get_tree_root() const;
    std::mutex& get_mutex() const;
    size_t get_total_size() const;
    std::pmr::memory_resource* get_parent_allocator() const;
    allocator_with_fit_mode::fit_mode get_fit_mode() const;

    void* find_suitable_block(size_t size);
    void* find_first_fit(void* node, size_t size);
    void* find_best_fit(void* node, size_t size);
    void find_best_fit_recursive(void* node, size_t size, void*& best_node, size_t& best_size);
    void* find_worst_fit(void* node, size_t size);
    void find_worst_fit_recursive(void* node, size_t size, void*& worst_node, size_t& worst_size);
    void initialize_free_block(void* block, size_t size, void* parent, void* left, void* right);

    void insert_into_tree(void* block);
    void post_insert(void* node);
    void remove_temp_nil_node(void* temp_nil);
    void attach_temp_nil_node(void* temp_nil, void* parent_node,
                                                    void* replacement_node, void* child_node);
    void* create_temp_nil_node(void* parent_node);



private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;


    inline std::string get_typename() const noexcept override;

    class rb_iterator
    {
        void* _block_ptr;
        void* _trusted;

    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const rb_iterator&) const noexcept;

        bool operator!=(const rb_iterator&) const noexcept;

        rb_iterator& operator++() & noexcept;

        rb_iterator operator++(int n);

        size_t size() const noexcept;

        void* operator*() const noexcept;

        bool occupied()const noexcept;

        rb_iterator();

        rb_iterator(void* trusted);
    };

    friend class rb_iterator;

    rb_iterator begin() const noexcept;
    rb_iterator end() const noexcept;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H