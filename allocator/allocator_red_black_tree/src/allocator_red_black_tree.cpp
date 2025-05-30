#include "allocator_red_black_tree.h"
#include <cstring>
#include <algorithm>


allocator_red_black_tree::~allocator_red_black_tree() {
    auto* logger_ptr = get_logger();
    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::~allocator_red_black_tree() called", logger::severity::trace);
    }

    if (_trusted_memory != nullptr) {
        auto* parent_allocator = get_parent_allocator();
        if (parent_allocator) {
            parent_allocator->deallocate(_trusted_memory, get_total_size());
        }
        else {
            ::free(_trusted_memory);
        }
    }

    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::~allocator_red_black_tree() finished", logger::severity::trace);
    }
}


allocator_red_black_tree::allocator_red_black_tree(
    size_t space_size,
    std::pmr::memory_resource* parent_allocator,
    logger* logger_ptr,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) {
    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::allocator_red_black_tree() called", logger::severity::trace);
    }

    size_t total_size = space_size + allocator_metadata_size + free_block_metadata_size;

    if (parent_allocator) {
        _trusted_memory = parent_allocator->allocate(total_size);
    }
    else {
        _trusted_memory = ::malloc(total_size);
    }

    if (!_trusted_memory) {
        if (logger_ptr) {
            logger_ptr->log("Failed to allocate trusted memory", logger::severity::error);
        }
        throw std::bad_alloc();
    }

    // Инициализация метаданных аллокатора
    char* metadata_ptr = static_cast<char*>(_trusted_memory);

    *reinterpret_cast<logger**>(metadata_ptr) = logger_ptr;
    metadata_ptr += sizeof(logger*);

    *reinterpret_cast<std::pmr::memory_resource**>(metadata_ptr) = parent_allocator;
    metadata_ptr += sizeof(std::pmr::memory_resource*);

    *reinterpret_cast<fit_mode*>(metadata_ptr) = allocate_fit_mode;
    metadata_ptr += sizeof(fit_mode);

    *reinterpret_cast<size_t*>(metadata_ptr) = total_size;
    metadata_ptr += sizeof(size_t);

    new(metadata_ptr) std::mutex();
    metadata_ptr += sizeof(std::mutex);

    *reinterpret_cast<void**>(metadata_ptr) = nullptr; // root указатель

    // Инициализация первого свободного блока
    void* first_block = static_cast<char*>(_trusted_memory) + allocator_metadata_size;

    initialize_free_block(first_block, space_size + free_block_metadata_size, nullptr, nullptr, nullptr);
    set_color(first_block, block_color::BLACK);
    set_tree_root(first_block);

    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::allocator_red_black_tree() finished", logger::severity::trace);
        logger_ptr->log("Available memory: " + std::to_string(space_size) + " bytes", logger::severity::information);
        logger_ptr->log(get_blocks_state(), logger::severity::debug);
    }
}


void allocator_red_black_tree::insert_into_tree(void* block) {
    void* root = get_tree_root();

    if (!root) {
        set_color(block, block_color::BLACK);
        set_tree_root(block);
        return;
    }

    void* current = root;
    void* parent = nullptr;
    size_t block_size = get_block_size(block);

    while (current) {
        parent = current;
        if (block_size <= get_block_size(current)) {
            current = get_left_child(current);
        }
        else {
            current = get_right_child(current);
        }
    }

    initialize_free_block(block, block_size, parent, nullptr, nullptr);

    // Присоединяем к родителю
    if (block_size <= get_block_size(parent)) {
        set_left_child(parent, block);
    }
    else {
        set_right_child(parent, block);
    }

    post_insert(block);
}


void allocator_red_black_tree::post_insert(void* node) {
    if (!node) return;

    void* t = node;

    while (t && get_parent(t) && get_color(get_parent(t)) == block_color::RED) {
        void* parent = get_parent(t);
        void* grandparent = get_parent(parent);

        if (!grandparent) break;

        if (parent == get_left_child(grandparent)) {
            void* uncle = get_right_child(grandparent);

            if (uncle && get_color(uncle) == block_color::RED) {
                // Дядя красный - перекрашиваем
                set_color(parent, block_color::BLACK);
                set_color(uncle, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                t = grandparent;
            }
            else {
                // Дядя черный
                if (t == get_right_child(parent)) {
                    // t - правый ребенок parent
                    t = parent;
                    void* g = grandparent;
                    left_rotate(parent);
                    parent = get_parent(t);
                    grandparent = g;
                }
                // t - левый ребенок parent (или стал им после case 2)
                set_color(parent, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                right_rotate(grandparent);
                break;
            }
        }
        else {
            void* uncle = get_left_child(grandparent);

            if (uncle && get_color(uncle) == block_color::RED) {
                // Дядя красный - перекрашиваем
                set_color(parent, block_color::BLACK);
                set_color(uncle, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                t = grandparent;
            }
            else {
                // Дядя черный
                if (t == get_left_child(parent)) {
                    // t - левый ребенок parent
                    t = parent;
                    void* g = grandparent;
                    right_rotate(parent);
                    parent = get_parent(t);
                    grandparent = g; // Восстанавливаем дедушку
                }
                // t - правый ребенок parent (или стал им после case 2)
                set_color(parent, block_color::BLACK);
                set_color(grandparent, block_color::RED);
                left_rotate(grandparent);
                break;
            }
        }
    }

    void* root = get_tree_root();
    if (root) {
        set_color(root, block_color::BLACK);
    }
}

void* allocator_red_black_tree::do_allocate_sm(size_t size) {
    auto* logger_ptr = get_logger();
    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::do_allocate_sm(" + std::to_string(size) + ") called", logger::severity::debug);
    }

    std::lock_guard<std::mutex> lock(get_mutex());

    if (size == 0) {
        if (logger_ptr) {
            logger_ptr->log("Allocation request for 0 bytes redirected to 1 byte", logger::severity::warning);
        }
        size = 1;
    }

    void* suitable_block = find_suitable_block(size);
    if (!suitable_block) {
        if (logger_ptr) {
            logger_ptr->log("No suitable block found for allocation", logger::severity::error);
        }
        throw std::bad_alloc();
    }

    size_t block_size = get_block_size(suitable_block);

    remove_from_tree(suitable_block);
    void* result = split_blocks(suitable_block, size, block_size);

    if (logger_ptr) {
        logger_ptr->log("Available memory: " + std::to_string(calculate_available_memory()) + " bytes", logger::severity::information);
        logger_ptr->log(get_blocks_state(), logger::severity::debug);
        logger_ptr->log("allocator_red_black_tree::do_allocate_sm() finished", logger::severity::debug);
    }

    return static_cast<char*>(result) + occupied_block_metadata_size;
}

void allocator_red_black_tree::do_deallocate_sm(void* at) {
    auto* logger_ptr = get_logger();
    if (logger_ptr) logger_ptr->log("do_deallocate_sm() called", logger::severity::debug);

    std::lock_guard<std::mutex> lock(get_mutex());

    if (!at) return;

    void* block_start = static_cast<char*>(at) - occupied_block_metadata_size;

    if (!is_block_belongs_to_allocator(block_start)) {
        throw std::logic_error("Block doesn't belong to this allocator");
    }

    void* merged_block = merge_blocks(block_start);

    mark_block_as_free(merged_block);

    insert_into_tree(merged_block);

    if (logger_ptr) {
        logger_ptr->log("Available: " + std::to_string(calculate_available_memory()) + " bytes", logger::severity::information);
        logger_ptr->log(get_blocks_state(), logger::severity::debug);
        logger_ptr->log("do_deallocate_sm() finished", logger::severity::debug);
    }
}

void allocator_red_black_tree::remove_from_tree(void* node_to_delete) {
    if (!node_to_delete) return;

    void* replacement_node = node_to_delete; // Узел, который фактически удаляется
    block_color replacement_original_color = get_color(replacement_node);

    void* child_node = nullptr; // Ребенок replacement_node, который займет его место
    void* child_parent = nullptr; // Родитель child_node

    if (get_left_child(node_to_delete) == nullptr) {
        child_node = get_right_child(node_to_delete);
        child_parent = get_parent(node_to_delete);
        transplant(node_to_delete, child_node);
    }
    else if (get_right_child(node_to_delete) == nullptr) {
        child_node = get_left_child(node_to_delete);
        child_parent = get_parent(node_to_delete);
        transplant(node_to_delete, child_node);
    }
    else {
        replacement_node = get_left_child(node_to_delete);

        while (get_right_child(replacement_node) != nullptr) {
            replacement_node = get_right_child(replacement_node);
        }

        replacement_original_color = get_color(replacement_node);
        child_node = get_left_child(replacement_node);

        if (get_parent(replacement_node) == node_to_delete) {
            child_parent = replacement_node;
        }
        else {
            child_parent = get_parent(replacement_node);
            transplant(replacement_node, child_node);
            set_left_child(replacement_node, get_left_child(node_to_delete));
        }

        transplant(node_to_delete, replacement_node);
        set_right_child(replacement_node, get_right_child(node_to_delete));
        set_color(replacement_node, get_color(node_to_delete));
    }


    if (replacement_original_color == block_color::BLACK) {
        if (child_node != nullptr) {
            fix_double_black(child_node);
        }
        else if (child_parent != nullptr) {
            void* temp_nil_node = create_temp_nil_node(child_parent);

            attach_temp_nil_node(temp_nil_node, child_parent, replacement_node, child_node);

            fix_double_black(temp_nil_node);

            remove_temp_nil_node(temp_nil_node);
        }
    }

    // Корень всегда должен быть черным
    void* root = get_tree_root();
    if (root != nullptr) {
        set_color(root, block_color::BLACK);
    }
}


void allocator_red_black_tree::transplant(void* u, void* v) {
    void* u_parent = get_parent(u);

    if (!u_parent) {
        set_tree_root(v);
    }
    else if (u == get_left_child(u_parent)) {
        set_left_child(u_parent, v);
    }
    else {
        set_right_child(u_parent, v);
    }

    if (v) {
        set_parent(v, u_parent);
    }
}


void* allocator_red_black_tree::minimum(void* node) const {
    if (!node) return nullptr;

    while (get_left_child(node)) {
        node = get_left_child(node);
    }

    return node;
}


void allocator_red_black_tree::fix_double_black(void* double_black_node) {
    void* current_node = double_black_node;

    while (current_node && current_node != get_tree_root() && get_color(current_node) == block_color::BLACK) {
        void* parent_node = get_parent(current_node);

        bool is_left_child = (current_node == get_left_child(parent_node));

        void* sibling_node = is_left_child
            ? get_right_child(parent_node)
            : get_left_child(parent_node);

        if (sibling_node == nullptr) {
            current_node = parent_node;
            continue;
        }

        if (is_left_child) { // current_node - левый ребенок, sibling - правый
            // Случай 1: Брат красный
            if (get_color(sibling_node) == block_color::RED) {
                set_color(sibling_node, block_color::BLACK);
                set_color(parent_node, block_color::RED);
                left_rotate(parent_node);
                sibling_node = get_right_child(parent_node);
            }

            void* sibling_left_child = get_left_child(sibling_node);
            void* sibling_right_child = get_right_child(sibling_node);

            bool sibling_left_is_black = (!sibling_left_child || get_color(sibling_left_child) == block_color::BLACK);
            bool sibling_right_is_black = (!sibling_right_child || get_color(sibling_right_child) == block_color::BLACK);

            // Случай 2: Брат черный, и оба его ребенка черные
            if (sibling_left_is_black && sibling_right_is_black) {
                set_color(sibling_node, block_color::RED);
                current_node = parent_node;
            }
            else {
                // Случай 3: Правый ребенок брата черный (левый красный)
                if (sibling_right_is_black) {
                    if (sibling_left_child)
                        set_color(sibling_left_child, block_color::BLACK);
                    set_color(sibling_node, block_color::RED);
                    right_rotate(sibling_node);
                    sibling_node = get_right_child(parent_node);
                    sibling_right_child = get_right_child(sibling_node);
                }

                // Случай 4: Правый ребенок брата красный
                set_color(sibling_node, get_color(parent_node));
                set_color(parent_node, block_color::BLACK);
                if (sibling_right_child)
                    set_color(sibling_right_child, block_color::BLACK);
                left_rotate(parent_node);
                current_node = get_tree_root(); // Проблема решена
            }
        }
        else { // current_node - правый ребенок, sibling - левый (симметрично)
            // Случай 1: Брат красный
            if (get_color(sibling_node) == block_color::RED) {
                set_color(sibling_node, block_color::BLACK);
                set_color(parent_node, block_color::RED);
                right_rotate(parent_node);
                sibling_node = get_left_child(parent_node);
            }

            void* sibling_left_child = get_left_child(sibling_node);
            void* sibling_right_child = get_right_child(sibling_node);

            bool sibling_left_is_black = (!sibling_left_child || get_color(sibling_left_child) == block_color::BLACK);
            bool sibling_right_is_black = (!sibling_right_child || get_color(sibling_right_child) == block_color::BLACK);

            // Случай 2: Брат черный, и оба его ребенка черные
            if (sibling_left_is_black && sibling_right_is_black) {
                set_color(sibling_node, block_color::RED);
                current_node = parent_node;
            }
            else {
                // Случай 3: Левый ребенок брата черный (правый красный)
                if (sibling_left_is_black) {
                    if (sibling_right_child)
                        set_color(sibling_right_child, block_color::BLACK);
                    set_color(sibling_node, block_color::RED);
                    left_rotate(sibling_node);
                    sibling_node = get_left_child(parent_node);
                    sibling_left_child = get_left_child(sibling_node);
                }

                // Случай 4: Левый ребенок брата красный
                set_color(sibling_node, get_color(parent_node));
                set_color(parent_node, block_color::BLACK);
                if (sibling_left_child)
                    set_color(sibling_left_child, block_color::BLACK);
                right_rotate(parent_node);
                current_node = get_tree_root(); // Проблема решена
            }
        }
    }

    if (current_node) {
        set_color(current_node, block_color::BLACK);
    }
}

void* allocator_red_black_tree::split_blocks(void* block, size_t requested_size, size_t block_size) {
    size_t needed_size = requested_size + occupied_block_metadata_size;

    if (block_size >= needed_size + free_block_metadata_size) {
        void* new_free_block = static_cast<char*>(block) + needed_size;
        size_t new_free_size = block_size - needed_size;

        initialize_free_block(new_free_block, new_free_size, nullptr, nullptr, nullptr);

        insert_into_tree(new_free_block);

        set_block_size(block, needed_size);
    }

    mark_block_as_occupied(block);

    return block;
}


void* allocator_red_black_tree::merge_blocks(void* block) {
    size_t user_size = get_block_size(block);
    void* next_block = static_cast<char*>(block) + user_size;
    char* memory_end = static_cast<char*>(_trusted_memory) + get_total_size();

    if (next_block < memory_end && !is_block_occupied(next_block)) {
        remove_from_tree(next_block);
        size_t next_user_size = get_block_size(next_block);
        size_t total_user_size = user_size + next_user_size;
        set_block_size(block, total_user_size);
    }

    return block;
}

void allocator_red_black_tree::left_rotate(void* x) {
    if (!x) return;
    void* y = get_right_child(x);
    if (!y) return;

    // Обновляем правого ребенка x
    set_right_child(x, get_left_child(y));

    // Обновляем родителя y
    void* x_parent = get_parent(x);
    set_parent(y, x_parent);

    // Если x был корнем, обновляем корень
    if (!x_parent) {
        set_tree_root(y);
    }
    else if (x == get_left_child(x_parent)) {
        set_left_child(x_parent, y);
    }
    else {
        set_right_child(x_parent, y);
    }

    set_left_child(y, x);
}

void allocator_red_black_tree::right_rotate(void* y) {
    if (!y) return;
    void* x = get_left_child(y);
    if (!x) return;

    // Обновляем левого ребенка y
    set_left_child(y, get_right_child(x));

    // Обновляем родителя x
    void* y_parent = get_parent(y);
    set_parent(x, y_parent);

    // Если y был корнем, обновляем корень
    if (!y_parent) {
        set_tree_root(x);
    }
    else if (y == get_left_child(y_parent)) {
        set_left_child(y_parent, x);
    }
    else {
        set_right_child(y_parent, x);
    }

    set_right_child(x, y);
}

allocator_red_black_tree::block_color allocator_red_black_tree::get_color(void* block) const {
    if (!block) return block_color::BLACK;
    block_data* data = reinterpret_cast<block_data*>(block);
    return data->color;
}


void allocator_red_black_tree::set_color(void* block, block_color color) {
    if (!block) return;
    block_data* data = reinterpret_cast<block_data*>(block);
    data->color = color;
}


void* allocator_red_black_tree::get_parent(void* block) const {
    if (!block) return nullptr;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data);
    return *reinterpret_cast<void**>(block_ptr);
}

void allocator_red_black_tree::set_parent(void* block, void* parent) {
    if (!block) return;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data);
    *reinterpret_cast<void**>(block_ptr) = parent;
}

void* allocator_red_black_tree::get_left_child(void* block) const {
    if (!block) return nullptr;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + sizeof(void*);
    return *reinterpret_cast<void**>(block_ptr);
}

void allocator_red_black_tree::set_left_child(void* block, void* left_child) {
    if (!block) return;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + sizeof(void*);
    *reinterpret_cast<void**>(block_ptr) = left_child;

    if (left_child) {
        set_parent(left_child, block);
    }
}

void* allocator_red_black_tree::get_right_child(void* block) const {
    if (!block) return nullptr;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 2 * sizeof(void*);
    return *reinterpret_cast<void**>(block_ptr);
}

void allocator_red_black_tree::set_right_child(void* block, void* right_child) {
    if (!block) return;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 2 * sizeof(void*);
    *reinterpret_cast<void**>(block_ptr) = right_child;

    if (right_child) {
        set_parent(right_child, block);
    }
}


void* allocator_red_black_tree::get_next_free(void* block) const {
    if (!block) return nullptr;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 3 * sizeof(void*) + sizeof(size_t);
    return *reinterpret_cast<void**>(block_ptr);
}

void allocator_red_black_tree::set_next_free(void* block, void* next_free) {
    if (!block) return;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 3 * sizeof(void*) + sizeof(size_t);
    *reinterpret_cast<void**>(block_ptr) = next_free;
}

void* allocator_red_black_tree::get_prev_free(void* block) const {
    if (!block) return nullptr;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 3 * sizeof(void*) + sizeof(size_t) + sizeof(void*);
    return *reinterpret_cast<void**>(block_ptr);
}

void allocator_red_black_tree::set_prev_free(void* block, void* prev_free) {
    if (!block) return;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 3 * sizeof(void*) + sizeof(size_t) + sizeof(void*);
    *reinterpret_cast<void**>(block_ptr) = prev_free;
}

size_t allocator_red_black_tree::get_block_size(void* block) const {
    if (!block) return 0;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 3 * sizeof(void*);
    return *reinterpret_cast<size_t*>(block_ptr);
}

void allocator_red_black_tree::set_block_size(void* block, size_t size) {
    if (!block) return;
    char* block_ptr = static_cast<char*>(block) + sizeof(block_data) + 3 * sizeof(void*);
    *reinterpret_cast<size_t*>(block_ptr) = size;
}

bool allocator_red_black_tree::is_block_occupied(void* block) const {
    if (!block) return false;
    block_data* data = reinterpret_cast<block_data*>(block);
    return data->occupied;
}

void allocator_red_black_tree::mark_block_as_occupied(void* block) {
    if (!block) return;
    block_data* data = reinterpret_cast<block_data*>(block);
    data->occupied = true;
}

void allocator_red_black_tree::mark_block_as_free(void* block) {
    if (!block) return;
    block_data* data = reinterpret_cast<block_data*>(block);
    data->occupied = false;
}


bool allocator_red_black_tree::is_block_belongs_to_allocator(void* block) const {
    char* block_ptr = static_cast<char*>(block);
    char* memory_start = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    char* memory_end = static_cast<char*>(_trusted_memory) + get_total_size();

    return block_ptr >= memory_start && block_ptr < memory_end;
}

void* allocator_red_black_tree::create_temp_nil_node(void* parent_node) {
    static char temp_nil_buffer[free_block_metadata_size];
    void* temp_nil = temp_nil_buffer;

    initialize_free_block(temp_nil, 0, parent_node, nullptr, nullptr);
    set_color(temp_nil, block_color::BLACK);

    return temp_nil;
}

void allocator_red_black_tree::attach_temp_nil_node(void* temp_nil, void* parent_node,
                                                    void* replacement_node, void* child_node) {
    if (parent_node == replacement_node) {
        if (get_left_child(replacement_node) == child_node) {
            set_left_child(replacement_node, temp_nil);
        }
        else {
            set_right_child(replacement_node, temp_nil);
        }
    }
    else if (get_left_child(parent_node) == child_node) {
        set_left_child(parent_node, temp_nil);
    }
    else if (get_right_child(parent_node) == child_node) {
        set_right_child(parent_node, temp_nil);
    }
}

void allocator_red_black_tree::remove_temp_nil_node(void* temp_nil) {
    void* parent = get_parent(temp_nil);

    if (parent) {
        if (get_left_child(parent) == temp_nil) {
            set_left_child(parent, nullptr);
        }
        else if (get_right_child(parent) == temp_nil) {
            set_right_child(parent, nullptr);
        }
    }
    else if (get_tree_root() == temp_nil) {
        set_tree_root(nullptr);
    }
}


size_t allocator_red_black_tree::calculate_available_memory() const {
    size_t available = 0;
    for (auto block : get_blocks_info_inner()) {
        if (!block.is_block_occupied) {
            available += block.block_size - free_block_metadata_size;
        }
    }
    return available;
}

void* allocator_red_black_tree::find_suitable_block(size_t size) {
    auto fit_mode = get_fit_mode();
    void* root = get_tree_root();

    switch (fit_mode) {
        case fit_mode::first_fit:
            return find_first_fit(root, size);
        case fit_mode::the_best_fit:
            return find_best_fit(root, size);
        case fit_mode::the_worst_fit:
            return find_worst_fit(root, size);
        default:
            return find_first_fit(root, size);
    }
}

void* allocator_red_black_tree::find_first_fit(void* node, size_t size) {
    if (!node) return nullptr;

    size_t block_size = get_block_size(node);
    if (block_size >= size + occupied_block_metadata_size) {
        return node;
    }

    void* left_result = find_first_fit(get_left_child(node), size);
    if (left_result) return left_result;

    return find_first_fit(get_right_child(node), size);
}

void* allocator_red_black_tree::find_best_fit(void* node, size_t size) {
    void* best_node = nullptr;
    size_t best_size = SIZE_MAX;
    find_best_fit_recursive(node, size, best_node, best_size);
    return best_node;
}

void allocator_red_black_tree::find_best_fit_recursive(void* node, size_t size, void*& best_node, size_t& best_size) {
    if (!node) return;

    size_t block_size = get_block_size(node);
    if (block_size >= size + free_block_metadata_size && block_size < best_size) {
        best_node = node;
        best_size = block_size;
    }

    find_best_fit_recursive(get_left_child(node), size, best_node, best_size);
    find_best_fit_recursive(get_right_child(node), size, best_node, best_size);
}

void* allocator_red_black_tree::find_worst_fit(void* node, size_t size) {
    void* worst_node = nullptr;
    size_t worst_size = 0;
    find_worst_fit_recursive(node, size, worst_node, worst_size);
    return worst_node;
}

void allocator_red_black_tree::find_worst_fit_recursive(void* node, size_t size, void*& worst_node, size_t& worst_size) {
    if (!node) return;

    size_t block_size = get_block_size(node);
    if (block_size >= size + free_block_metadata_size && block_size > worst_size) {
        worst_node = node;
        worst_size = block_size;
    }

    find_worst_fit_recursive(get_left_child(node), size, worst_node, worst_size);
    find_worst_fit_recursive(get_right_child(node), size, worst_node, worst_size);
}

void allocator_red_black_tree::initialize_free_block(void* block, size_t size, void* parent, void* left, void* right) {
    char* block_ptr = static_cast<char*>(block);

    block_data* data = reinterpret_cast<block_data*>(block_ptr);
    data->occupied = false;
    data->color = block_color::RED;

    block_ptr += sizeof(block_data);

    *reinterpret_cast<void**>(block_ptr) = parent;
    block_ptr += sizeof(void*);
    *reinterpret_cast<void**>(block_ptr) = left;
    block_ptr += sizeof(void*);
    *reinterpret_cast<void**>(block_ptr) = right;
    block_ptr += sizeof(void*);

    *reinterpret_cast<size_t*>(block_ptr) = size;
    block_ptr += sizeof(size_t);

    *reinterpret_cast<void**>(block_ptr) = nullptr;
    block_ptr += sizeof(void*);
    *reinterpret_cast<void**>(block_ptr) = nullptr;
}

logger* allocator_red_black_tree::get_logger() const {
    return *reinterpret_cast<logger**>(_trusted_memory);
}

std::pmr::memory_resource* allocator_red_black_tree::get_parent_allocator() const {
    return *reinterpret_cast<std::pmr::memory_resource**>(static_cast<char*>(_trusted_memory) + sizeof(logger*));
}

allocator_with_fit_mode::fit_mode allocator_red_black_tree::get_fit_mode() const {
    return *reinterpret_cast<fit_mode*>(static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*));
}

void allocator_red_black_tree::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
    auto* logger_ptr = get_logger();
    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::set_fit_mode() called", logger::severity::debug);
    }

    std::lock_guard<std::mutex> lock(get_mutex());
    *reinterpret_cast<fit_mode*>(static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*)) = mode;

    if (logger_ptr) {
        logger_ptr->log("allocator_red_black_tree::set_fit_mode() finished", logger::severity::debug);
    }
}

size_t allocator_red_black_tree::get_total_size() const {
    return *reinterpret_cast<size_t*>(static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
}

std::mutex& allocator_red_black_tree::get_mutex() const {
    return *reinterpret_cast<std::mutex*>(static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t));
}

void* allocator_red_black_tree::get_tree_root() const {
    return *reinterpret_cast<void**>(static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex));
}

void allocator_red_black_tree::set_tree_root(void* root) {
    *reinterpret_cast<void**>(static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex)) = root;
}


bool allocator_red_black_tree::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

std::string allocator_red_black_tree::get_typename() const noexcept {
    return "allocator_red_black_tree";
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info() const {
    std::lock_guard<std::mutex> lock(get_mutex());
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info_inner() const {
    logger* logger_ptr = get_logger();
    if (logger_ptr) {
        logger_ptr->trace(get_typename() + "::get_blocks_info_inner(): called");
    }

    std::vector<allocator_test_utils::block_info> blocks;

    if (_trusted_memory == nullptr) {
        if (logger_ptr) {
            logger_ptr->error(get_typename() + "::get_blocks_info_inner(): not init memory");
        }
        return blocks;
    }

    try {
        char* heap_start = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
        size_t total_size = *reinterpret_cast<size_t*>(
            static_cast<char*>(_trusted_memory) +
            sizeof(logger*) +
            sizeof(std::pmr::memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));
        char* heap_end = heap_start + total_size - allocator_metadata_size;

        char* current = heap_start;

        while (current < heap_end) {
            allocator_test_utils::block_info info;
            info.block_size = get_block_size(current);
            info.is_block_occupied = is_block_occupied(current);

            blocks.push_back(info);

            current += info.block_size;
        }
    }
    catch (...) {
        if (logger_ptr) {
            logger_ptr->error(get_typename() + "::get_blocks_info_inner() : iteration failed.");
        }
        throw;
    }

    return blocks;
}

std::string allocator_red_black_tree::get_blocks_state() const {
    auto blocks = get_blocks_info_inner();
    std::string result;

    for (size_t i = 0; i < blocks.size(); ++i) {
        if (i > 0) result += "|";
        result += blocks[i].is_block_occupied ? "occup " : "avail ";
        result += std::to_string(blocks[i].block_size);
    }

    return result;
}
