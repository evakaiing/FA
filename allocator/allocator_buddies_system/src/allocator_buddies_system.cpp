#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"
#include "../../allocator_boundary_tags/include/allocator_boundary_tags.h"

using byte = std::byte;

allocator_buddies_system::~allocator_buddies_system() {
    logger* logger_inst = get_logger();
    if(logger_inst){
    logger_inst->debug(get_typename() + "::~allocator_buddies_system() start");}
    if(_trusted_memory == nullptr){
        return;
    }
    get_mutex().~mutex();
    auto* parent_allocator = get_parent_resource();
    unsigned char* size_ptr = reinterpret_cast<unsigned char*>(
            static_cast<byte*>(_trusted_memory) +
            sizeof(logger*) +
            sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode)
    );
    size_t total_size = allocator_metadata_size + (1 << *size_ptr);
    if(parent_allocator != nullptr){
        parent_allocator->deallocate(_trusted_memory, total_size);
    }
    else{
        ::operator delete(_trusted_memory);
    }
    _trusted_memory = nullptr;
    if(logger_inst) {
        logger_inst->information("Free " + std::to_string(1 << *size_ptr) + " bytes");
        logger_inst->debug(get_typename() + "::~allocator_buddies_system() finish");
    }
}

allocator_buddies_system::allocator_buddies_system(
        allocator_buddies_system &&other) noexcept
        : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        logger *logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if(logger){
        logger->debug(get_typename() + "::allocator_buddies_system started");
    }

    size_t actual_space_size_power = 0;
    size_t temp = space_size;

    if (space_size == 0) {
        actual_space_size_power = 0;
    } else {
        while (temp > 1) {
            temp >>= 1;
            actual_space_size_power++;
        }

        // Если исходное число не было степенью двойки, увеличиваем степень
        if ((static_cast<size_t>(1) << actual_space_size_power) < space_size) {
            actual_space_size_power++;
        }
    }

    size_t actual_block_size = static_cast<size_t>(1) << actual_space_size_power;

    if(actual_block_size < allocator_metadata_size){
        if(logger){
            logger->error("Block size is too small to fit metadata");
        }
        throw std::logic_error("Block size is too small to fit metadata");
    }

    parent_allocator = parent_allocator ? parent_allocator : std::pmr::get_default_resource();
    size_t total_size = allocator_metadata_size + actual_block_size;
    _trusted_memory = parent_allocator->allocate(total_size);
    auto *memory = reinterpret_cast<std::byte *>(_trusted_memory);

    *reinterpret_cast<class logger **>(memory) = logger;
    memory += sizeof(class logger *);

    *reinterpret_cast<std::pmr::memory_resource **>(memory) = parent_allocator;
    memory += sizeof(std::pmr::memory_resource *);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(memory) = allocate_fit_mode;
    memory += sizeof(allocator_with_fit_mode::fit_mode);

    *reinterpret_cast<unsigned char*>(memory) = static_cast<unsigned char>(actual_space_size_power);
    memory += sizeof(unsigned char);

    new(reinterpret_cast<std::mutex *>(memory)) std::mutex();
    memory += sizeof(std::mutex);

    void* first_block = memory;
    block_metadata* meta = reinterpret_cast<block_metadata*>(first_block);
    meta->size = static_cast<unsigned char>(actual_space_size_power);
    meta->occupied = false;

    if(logger)
    {
        logger->information("Initial memory: " + std::to_string(actual_block_size));
        logger->debug(get_typename() + "::allocator_buddies_system finished. " +
                      "Initial memory: " + std::to_string(actual_block_size));
    }
}

void* allocator_buddies_system::allocate_first_fit(size_t size) {
    std::lock_guard<std::mutex> guard(get_mutex());
    size_t k = __detail::nearest_greater_k_of_2(size + occupied_block_metadata_size);

    unsigned char total_size = *reinterpret_cast<unsigned char*>(
            (byte*)_trusted_memory + sizeof(logger*) + sizeof(memory_resource*) + sizeof(fit_mode)
    );

    void* current_block = (byte*)_trusted_memory + allocator_metadata_size;
    while (current_block < (byte*)_trusted_memory + allocator_metadata_size + (1 << total_size)) {

        block_metadata* meta = reinterpret_cast<block_metadata*>(current_block);

        if (!meta->occupied && meta->size >= k) {
            while (meta->size > k) {
                split_block(current_block);
                meta = reinterpret_cast<block_metadata*>(current_block);
            }

            meta->occupied = true;


            return (byte*)current_block + sizeof(block_metadata);
        }

        current_block = (byte*)current_block + (1 << meta->size);
    }

    return nullptr;
}

void* allocator_buddies_system::allocate_best_fit(size_t size) {
    std::lock_guard<std::mutex> guard(get_mutex());
    size_t k = __detail::nearest_greater_k_of_2(size + occupied_block_metadata_size);

    unsigned char total_size_power = *reinterpret_cast<unsigned char*>(
            (byte*)_trusted_memory + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));

    void* best_block = nullptr;
    unsigned char best_size = 255;

    void* current_block = (byte*)_trusted_memory + allocator_metadata_size;
    void* heap_end = (byte*)_trusted_memory + allocator_metadata_size + (1 << total_size_power);

    while (current_block < heap_end) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(current_block);

        if (!meta->occupied && meta->size >= k && meta->size < best_size) {
            best_block = current_block;
            best_size = meta->size;

            if (best_size == k) break;
        }

        current_block = (byte*)current_block + (1 << meta->size);
    }

    if (best_block) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(best_block);

        while (meta->size > k) {
            split_block(best_block);
            meta = reinterpret_cast<block_metadata*>(best_block);
        }

        meta->occupied = true;
        return (byte*)best_block + sizeof(block_metadata);
    }

    return nullptr;
}

void* allocator_buddies_system::allocate_worst_fit(size_t size) {
    std::lock_guard<std::mutex> guard(get_mutex());
    size_t k = __detail::nearest_greater_k_of_2(size + occupied_block_metadata_size);

    unsigned char total_size_power = *reinterpret_cast<unsigned char*>(
            (byte*)_trusted_memory + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));

    void* worst_block = nullptr;
    unsigned char worst_size = 0;

    void* current_block = (byte*)_trusted_memory + allocator_metadata_size;
    void* heap_end = (byte*)_trusted_memory + allocator_metadata_size + (1 << total_size_power);

    while (current_block < heap_end) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(current_block);

        if (!meta->occupied && meta->size >= k && meta->size > worst_size) {
            worst_block = current_block;
            worst_size = meta->size;
        }

        current_block = (byte*)current_block + (1 << meta->size);
    }

    if (worst_block) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(worst_block);

        // Делим блок, пока не получим нужный размер
        while (meta->size > k) {
            split_block(worst_block);
            meta = reinterpret_cast<block_metadata*>(worst_block);
        }

        meta->occupied = true;
        return (byte*)worst_block + sizeof(block_metadata);
    }

    return nullptr;
}

void allocator_buddies_system::split_block(void* block) {
    block_metadata* meta = reinterpret_cast<block_metadata*>(block);

    if (meta->occupied || meta->size == 0) return;

    size_t new_size = meta->size - 1;
    size_t block_size = 1 << new_size;

    // Левый подблок
    block_metadata* first = meta;
    first->size = new_size;
    first->occupied = false;

    // Правый подблок
    block_metadata* second = reinterpret_cast<block_metadata*>((byte*)block + block_size);
    second->size = new_size;
    second->occupied = false;
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(
        size_t size) {
    logger* logger_t = get_logger();
    if(logger_t) { logger_t->debug(get_typename() + "::do_allocate_sm start"); }
    size_t k = __detail::nearest_greater_k_of_2(size + occupied_block_metadata_size);
    unsigned char size_alloc = *reinterpret_cast<unsigned char*>((byte*)_trusted_memory + sizeof(logger*) + sizeof(memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode));
    if(k > size_alloc){
        throw std::bad_alloc();
    }
    void* allocated_memory = nullptr;
    switch(get_fit_mode()){
        case fit_mode::first_fit:
            allocated_memory = allocate_first_fit(size);
            break;
        case fit_mode::the_best_fit:
            allocated_memory = allocate_best_fit(size);
            break;
        case fit_mode::the_worst_fit:
            allocated_memory = allocate_worst_fit(size);
            break;
    }
    if(allocated_memory == nullptr){
        throw std::bad_alloc();
    }
    if(logger_t) {
        logger_t->information("Allocate " + std::to_string(size) + " bytes");
        logger_t->debug(get_typename() + "::do_allocate_sm finish");
    }
    return allocated_memory;

}
void allocator_buddies_system::do_deallocate_sm(void *at) {
    logger* logger_t = get_logger();
    std::lock_guard<std::mutex> guard(get_mutex());
    if(logger_t) { logger_t->debug(get_typename() + "::do_deallocate_sm(void *at) start"); }
    if (at == nullptr) return;

    void* block_start = (byte*)at - sizeof(block_metadata);

    byte* heap_start = reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size;
    unsigned char heap_size_power = *reinterpret_cast<unsigned char*>(
            reinterpret_cast<byte*>(_trusted_memory) +
            sizeof(class logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));
    byte* heap_end = heap_start + (1 << heap_size_power);

    if (block_start < (void*)heap_start || block_start >= (void*)heap_end) {
        if(logger_t) { logger_t->error("Pointer does not belong to this allocator"); }
        throw std::invalid_argument("Pointer does not belong to this allocator");
    }

    block_metadata* meta = reinterpret_cast<block_metadata*>(block_start);
    meta->occupied = false;

    try_merge_with_buddy(block_start);
    if(logger_t) { logger_t->debug("Deallocated block at " + std::to_string(reinterpret_cast<uintptr_t>(at))); }
}


void allocator_buddies_system::try_merge_with_buddy(void *block) {
    block_metadata* meta = reinterpret_cast<block_metadata*>(block);
    size_t block_size = (1 << meta->size);

    // Преобразуем указатель в uintptr_t для арифметики
    uintptr_t block_addr = reinterpret_cast<uintptr_t>(block);
    uintptr_t buddy_addr = block_addr ^ block_size; // Теперь XOR допустим
    void* buddy = reinterpret_cast<void*>(buddy_addr);

    // Проверяем границы кучи (как было ранее)
    byte* heap_start = reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size;
    unsigned char heap_size_power = *reinterpret_cast<unsigned char*>(
            reinterpret_cast<byte*>(_trusted_memory) +
            sizeof(class logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));
    byte* heap_end = heap_start + (1 << heap_size_power);

    if (buddy < (void*)heap_start || buddy >= (void*)heap_end) {
        return;
    }

    // Дальнейшая логика объединения...
    block_metadata* buddy_meta = reinterpret_cast<block_metadata*>(buddy);
    if (!buddy_meta->occupied && buddy_meta->size == meta->size) {
        void* left_block = (block < buddy) ? block : buddy;
        block_metadata* left_meta = reinterpret_cast<block_metadata*>(left_block);
        left_meta->size += 1;
        try_merge_with_buddy(left_block);
    }
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
    if (this == &other) return true;
    const auto *derived = dynamic_cast<const allocator_buddies_system *>(&other);
    if (derived == nullptr) return false;

    return this->_trusted_memory == derived->_trusted_memory;
}

inline void allocator_buddies_system::set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) {
    char* memory = reinterpret_cast<char*>(_trusted_memory);
    allocator_with_fit_mode::fit_mode* mode_ptr =
        reinterpret_cast<allocator_with_fit_mode::fit_mode*>(
            memory + sizeof(logger*) + sizeof(std::pmr::memory_resource*)); // ✅ Исправлено
    *mode_ptr = mode;
}


std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept {

    std::lock_guard<std::mutex> guard(get_mutex());
    logger* logger_t = get_logger();
    if(logger_t) { logger_t->debug(get_typename() + "::get_blocks_info() start"); }
    auto result = get_blocks_info_inner();
    if(logger_t) { logger_t->debug(get_typename() + "::get_blocks_info() finish"); }
    return result;
}

std::pmr::memory_resource* allocator_buddies_system::get_parent_resource() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<std::pmr::memory_resource**>(
            reinterpret_cast<std::byte*>(_trusted_memory) + sizeof(logger*));
}

allocator_with_fit_mode::fit_mode allocator_buddies_system::get_fit_mode() const {
    unsigned char* memory = reinterpret_cast<unsigned char*>(_trusted_memory);
    memory += sizeof(logger*) + sizeof(memory_resource*);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory);
}

inline std::mutex &allocator_buddies_system::get_mutex() const
{
    if (_trusted_memory == nullptr)
    {
        throw std::logic_error("Access to mutex in invalid allocator state");
    }

    auto *ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    ptr += sizeof(memory_resource*);
    ptr += sizeof(allocator_with_fit_mode::fit_mode);
    ptr += sizeof(unsigned char);

    return *reinterpret_cast<std::mutex*>(ptr);
}

inline logger *allocator_buddies_system::get_logger() const
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<logger**>(_trusted_memory);
}

inline std::string allocator_buddies_system::get_typename() const {
    return "allocator_buddies_system";
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const {
    std::vector<allocator_test_utils::block_info> blocks_info;

    // Получаем общий размер доверенной памяти
    unsigned char total_size = *reinterpret_cast<const unsigned char*>(
            static_cast<const byte*>(_trusted_memory) +
            sizeof(logger*) +
            sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode)
    );

    // Начинаем с первого блока после метаданных аллокатора
    const void* current_block = static_cast<const byte*>(_trusted_memory) + allocator_metadata_size;
    const void* memory_end = static_cast<const byte*>(_trusted_memory) + allocator_metadata_size +(1<< total_size);

    while (current_block < memory_end) {
        const block_metadata* meta = reinterpret_cast<const block_metadata*>(current_block);

        // Создаем информацию о блоке
        allocator_test_utils::block_info info;
        info.is_block_occupied = meta->occupied;
        info.block_size = 1 << meta->size; // 2^size
        //std::cout<<info.block_size << " " << info.is_block_occupied << std::endl;
        // Добавляем в вектор
        blocks_info.push_back(info);

        // Переходим к следующему блоку
        current_block = static_cast<const byte*>(current_block) + info.block_size;
    }

    return blocks_info;
}


