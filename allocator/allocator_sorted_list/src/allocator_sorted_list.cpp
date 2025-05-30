#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"
allocator_sorted_list::~allocator_sorted_list() {

    logger* logger_instance = get_logger();

    if (logger_instance) {
        logger_instance->debug(get_typename() + "::~allocator_sorted_list() : called.");
    }

    if (_trusted_memory == nullptr) {
        if (logger_instance) {
            logger_instance->error(get_typename() + "::~allocator_sorted_list() : no memory to deallocate.");
        }
        return;
    }

    auto total_size = get_total_size();
    auto* parent_allocator = get_parent_resource();

    try {
        if (parent_allocator) {
            parent_allocator->deallocate(_trusted_memory, total_size);
            if (logger_instance) {
                logger_instance->trace(get_typename() + "::~allocator_sorted_list() : deallocated via parent.");
            }
        } else {
            ::operator delete(_trusted_memory);
            if (logger_instance) {
                logger_instance->trace(get_typename() + "::~allocator_sorted_list() : deallocated via global delete.");
            }
        }
    } catch (const std::exception& ex) {
        if (logger_instance) {
            logger_instance->error(get_typename() + "::~allocator_sorted_list() : deallocation failed - " + std::string(ex.what()));
        }
    }

    _trusted_memory = nullptr;

    if (logger_instance) {
        logger_instance->trace(get_typename() + "::~allocator_sorted_list() : finished.");
    }
}


allocator_sorted_list::allocator_sorted_list(allocator_sorted_list&& other) noexcept
    : _trusted_memory(nullptr)
{
    trace_with_guard(get_typename() + "::allocator_sorted_list(allocator_sorted_list&&) : called.");

    std::lock_guard<std::mutex> lock(other.get_mutex());

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if (auto* logger = get_logger()) {
        logger->debug(get_typename() + "::allocator_sorted_list(allocator_sorted_list&&) : resources moved successfully.");
    }

    trace_with_guard(get_typename() + "::allocator_sorted_list(allocator_sorted_list&&) : successfully finished.");
}

allocator_sorted_list& allocator_sorted_list::operator=(allocator_sorted_list&& other) noexcept
{
    trace_with_guard(get_typename() + "::operator=(allocator_sorted_list&&) : called.");

    if (this != &other)
    {
        // Заблокировать оба мьютекса
        std::unique_lock<std::mutex> lock_this(get_mutex(), std::defer_lock);
        std::unique_lock<std::mutex> lock_other(other.get_mutex(), std::defer_lock);
        std::lock(lock_this, lock_other);

        // Освобождаем текущую память
        if (_trusted_memory != nullptr)
        {
            try
            {
                size_t total_size = *reinterpret_cast<size_t*>(
                    reinterpret_cast<std::byte*>(_trusted_memory) +
                    sizeof(logger*) +
                    sizeof(std::pmr::memory_resource*) +
                    sizeof(fit_mode) + 4 + sizeof(void*) + sizeof(std::mutex));

                auto* parent_alloc = get_parent_resource();
                if (parent_alloc)
                {
                    parent_alloc->deallocate(_trusted_memory, total_size + allocator_metadata_size);
                }
                else
                {
                    ::operator delete(_trusted_memory);
                }
            }
            catch (const std::exception& ex)
            {
                if (auto* logger = get_logger())
                {
                    logger->error(get_typename() + "::operator=(allocator_sorted_list&&) : deallocation failed - " + std::string(ex.what()));
                }
            }
        }

        // Перемещаем указатель
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;

        if (auto* logger = get_logger()) {
            logger->debug(get_typename() + "::operator=(allocator_sorted_list&&) : resources moved successfully.");
        }
    }

    trace_with_guard(get_typename() + "::operator=(allocator_sorted_list&&) : successfully finished.");
    return *this;
}


allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    std::pmr::memory_resource* parent_allocator,
    logger* logger_instance,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + 4 + sizeof(void*) + sizeof(std::mutex) + sizeof(size_t) + block_metadata_size) {
        throw std::invalid_argument("allocator_sorted_list: space too small");
    }

    if (parent_allocator) {
        _trusted_memory = parent_allocator->allocate(space_size + allocator_metadata_size);
    } else {
        _trusted_memory = ::operator new(space_size + allocator_metadata_size);
    }

    std::byte* ptr = reinterpret_cast<std::byte*>(_trusted_memory);

    *reinterpret_cast<logger**>(ptr) = logger_instance;
    ptr += sizeof(logger*);

    *reinterpret_cast<std::pmr::memory_resource**>(ptr) = parent_allocator;
    ptr += sizeof(std::pmr::memory_resource*);

    *reinterpret_cast<fit_mode*>(ptr) = allocate_fit_mode;
    ptr += sizeof(fit_mode);

    ptr += 4; // ля выравнивания

    *reinterpret_cast<void**>(ptr) = nullptr; // first_free
    ptr += sizeof(void*);

    new (ptr) std::mutex();
    ptr += sizeof(std::mutex);

    *reinterpret_cast<size_t*>(ptr) = space_size;
    ptr += sizeof(size_t);

    // Указатель на первый свободный блок
    std::byte* block = ptr;
    *reinterpret_cast<void**>(block) = nullptr;  // next = nullptr
    block += sizeof(void*);
    *reinterpret_cast<size_t*>(block) = space_size - (block - reinterpret_cast<std::byte*>(_trusted_memory));

    set_first_free(ptr);

    if (logger_instance) {
        logger_instance->trace("allocator_sorted_list: metadata initialized");
        logger_instance->trace("allocator_sorted_list: free block initialized");
        logger_instance->information("Available memory: " +
                                     std::to_string(space_size - (block - reinterpret_cast<std::byte*>(_trusted_memory))));
    }
}

[[nodiscard]] void* allocator_sorted_list::do_allocate_sm(size_t size) {
    trace_with_guard(get_typename() + "::do_allocate_sm(" + std::to_string(size) + ") : called.");

    void* user_ptr = nullptr;

    {
        std::lock_guard<std::mutex> lock(get_mutex());

        logger* logger = get_logger();
        if (logger) {
            logger->debug(get_typename() + "::do_allocate_sm(): called");
        }

        if (size == 0) {
            if (logger) {
                logger->warning(get_typename() + "::do_allocate_sm(): request for 0 bytes, returning nullptr.");
            }
            return nullptr;
        }

        size_t required_size = size + block_metadata_size;
        size_t allocator_size = *reinterpret_cast<size_t*>(
            reinterpret_cast<std::byte*>(_trusted_memory) +
            sizeof(class logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(void*) + sizeof(std::mutex) + sizeof(size_t));


        if (required_size > allocator_size) {
            if (logger) {
                logger->error(get_typename() + "::do_allocate_sm(): requested size " +
                              std::to_string(size) + " is too large (max available: " +
                              std::to_string(allocator_size - block_metadata_size) + ")");
            }
        }


        void* current = get_first_free();

        // Поиск первого подходящего блока
        void* prev = nullptr;
        void* best_block = nullptr;
        void* best_prev = nullptr;
        size_t best_size = 0;


        while (current != nullptr) {
            size_t current_size = *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(current) + sizeof(void*));

            if (current_size >= required_size) {
                bool is_better = false;

                switch (get_fit_mode()) {
                    case fit_mode::first_fit:
                        best_block = current;
                        best_prev = prev;
                        best_size = current_size;
                        current = nullptr;
                        continue;
                    case fit_mode::the_best_fit:
                        if (!best_block || current_size < best_size) is_better = true;
                        break;
                    case fit_mode::the_worst_fit:
                        if (!best_block || current_size > best_size) is_better = true;
                        break;
                }

                if (is_better) {
                    best_block = current;
                    best_prev = prev;
                    best_size = current_size;
                }
            }

            prev = current;
            current = *reinterpret_cast<void**>(current);
        }

        if (!best_block) {
            if (logger) {
                logger->error(get_typename() + "::do_allocate_sm(): no suitable block found.");
            }
            throw std::bad_alloc();
        }

        std::byte* block_ptr = reinterpret_cast<std::byte*>(best_block);
        void* next_free = *reinterpret_cast<void**>(block_ptr);
        size_t free_block_size = *reinterpret_cast<size_t*>(block_ptr + sizeof(void*));

        size_t total_needed = size + block_metadata_size;
        size_t remaining = free_block_size - total_needed;

        // Разделяем блок, если можем
        if (remaining >= block_metadata_size) {
            std::byte* new_block = block_ptr + total_needed;
            *reinterpret_cast<void**>(new_block) = next_free; // Указатель на свободную память
            *reinterpret_cast<size_t*>(new_block + sizeof(void*)) = remaining; // Обновляем size

            if (best_prev) {
                *reinterpret_cast<void**>(best_prev) = new_block;
            } else {
                // Это первая аллокация
                set_first_free(new_block);
            }

            // Запись в метаданные занятого блока
            *reinterpret_cast<size_t*>(block_ptr + sizeof(void*)) = size;
        } else {
            // Слишком маленький остаток — забираем
            size += remaining;
            total_needed = free_block_size;
            if (best_prev) {
                *reinterpret_cast<void**>(best_prev) = next_free;
            } else {
                set_first_free(next_free);
            }
        }

        user_ptr = block_ptr + block_metadata_size;

        if (logger) {
            logger->information(get_typename() + "::do_allocate_sm(): allocated " + std::to_string(size) + " bytes.");
        }

    }

    std::string layout;
    for (const auto& b : get_blocks_info()) {
        layout += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
    }

    logger* logger = get_logger();

    if (logger) {
        logger->debug(get_typename() + "::do_allocate_sm(): memory layout: " + layout);
    }

    return user_ptr;
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
    if (this == &other) return true;

    const auto* derived = dynamic_cast<const allocator_sorted_list*>(&other);
    if (derived == nullptr) return false;

    return this->_trusted_memory == derived->_trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(void* at) {
    trace_with_guard(get_typename() + "::do_deallocate_sm() : called");
    logger* logger = get_logger();
    {
        std::lock_guard<std::mutex> lock(get_mutex());

        if (at == nullptr) {
            if (logger) logger->warning(get_typename() + "::do_deallocate_sm(): nullptr passed");
            return;
        }

        // Вычисляем начало блока и размер
        std::byte* block_to_delete_ptr = reinterpret_cast<std::byte*>(at) - block_metadata_size;
        size_t block_to_delete_size = *reinterpret_cast<size_t*>(block_to_delete_ptr + sizeof(void*));

        // Вставка в список свободных блоков
        void* prev = nullptr;
        void* current = get_first_free();


        while (current != nullptr && current < block_to_delete_ptr) {
            prev = current;
            current = *reinterpret_cast<void**>(current);  // Переходим к следующему в списке свободных блоков
        }

        // Заполняем поля в новом свободном блоке
        *reinterpret_cast<void**>(block_to_delete_ptr) = current; // Следующий свободный блок
        *reinterpret_cast<size_t*>(block_to_delete_ptr + sizeof(void*)) = block_to_delete_size;

        if (prev) {
            *reinterpret_cast<void**>(prev) = block_to_delete_ptr;
        } else {
            set_first_free(block_to_delete_ptr);
        }

        // Объединяем с соседями
        std::byte* inserted_ptr = reinterpret_cast<std::byte*>(block_to_delete_ptr);
        size_t inserted_size = *reinterpret_cast<size_t*>(inserted_ptr + sizeof(void*));
        std::byte* inserted_end = inserted_ptr + block_metadata_size + inserted_size;

        // 1. Объединение с правым соседом (current)
        // [inserted_ptr][current] -> [inserted_ptr] (merged)
        if (current && reinterpret_cast<std::byte*>(current) == inserted_end) {
            size_t current_size = *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(current) + sizeof(void*));
            *reinterpret_cast<void**>(block_to_delete_ptr) = *reinterpret_cast<void**>(current); // inserted_ptr.next = current.next (пропускаем current)
            *reinterpret_cast<size_t*>(inserted_ptr + sizeof(void*)) += block_metadata_size + current_size; // Увеличиваем размер A
        }

        // 2. Объединение с левым соседом (prev)
        if (prev) {
            size_t prev_size = *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(prev) + sizeof(void*));
            std::byte* prev_end = reinterpret_cast<std::byte*>(prev) + block_metadata_size + prev_size;

            if (prev_end == inserted_ptr) {
                *reinterpret_cast<void**>(prev) = *reinterpret_cast<void**>(block_to_delete_ptr); // skip inserted
                *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(prev) + sizeof(void*))
                    += block_metadata_size + *reinterpret_cast<size_t*>(inserted_ptr + sizeof(void*));
            }
        }

        if (logger) logger->information(get_typename() + "::do_deallocate_sm(): deallocated " + std::to_string(block_to_delete_size) + " bytes");

    }

    std::string layout;
    for (const auto& b : get_blocks_info()) {
        layout += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
    }

    if (logger) logger->debug(get_typename() + "::do_deallocate_sm(): memory layout: " + layout);
}



inline void allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
    trace_with_guard(get_typename() + "::set_fit_mode() : called");

    auto* fit_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(
        reinterpret_cast<char*>(_trusted_memory) +
        sizeof(logger*) + sizeof(std::pmr::memory_resource*)
    );

    *fit_ptr = mode;

    trace_with_guard(get_typename() + "::set_fit_mode() : finished");
}


std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{

    logger* logger = get_logger();

    if (logger) logger->trace(get_typename() + "::get_blocks_info(): called");

    std::lock_guard<std::mutex> guard(get_mutex());

    auto result = get_blocks_info_inner();

    if (logger) logger->debug(get_typename() + "::get_blocks_info() : retrieved " + std::to_string(result.size()) + " blocks.");

    if (logger) logger->trace(get_typename() + "::get_blocks_info(): finished");
    return result;
}

inline std::string allocator_sorted_list::get_typename() const
{
    return "allocator_sorted_list";
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const {
    logger* logger = get_logger();
    if (logger) logger->trace(get_typename() + "::get_blocks_info_inner(): called");

    std::vector<allocator_test_utils::block_info> result;

    if (_trusted_memory == nullptr) {
        if (logger) logger->error(get_typename() + "::get_blocks_info_inner(): memory not initialized");
        return result;
    }
    std::byte* base = reinterpret_cast<std::byte*>(_trusted_memory);

    // Шаги те же, что в конструкторе: перескакиваем logger, parent, fit, padding, first_free
    base += sizeof(class logger*);
    base += sizeof(std::pmr::memory_resource*);
    base += sizeof(fit_mode);
    base += 4;  // padding
    base += sizeof(void*); // first_free
    base += sizeof(std::mutex); // mutex
    base += sizeof(size_t); // total_size

    std::byte* heap_start = base;
    size_t heap_size = get_total_size();
    std::byte* heap_end = heap_start + heap_size;

    void* free_block = get_first_free();

    std::byte* current = heap_start;
    while (current + sizeof(void*) + sizeof(size_t) <= heap_end) {
        bool is_free = false;

        if (current == reinterpret_cast<std::byte*>(free_block)) {
            is_free = true;
            free_block = *reinterpret_cast<void**>(free_block);
        }

        size_t block_size = *reinterpret_cast<size_t*>(current + sizeof(void*));

        if (block_size == 0 || block_size > static_cast<size_t>(heap_end - current)) {
            if (logger) logger->error(get_typename() + "::get_blocks_info_inner(): invalid block_size = " +
                          std::to_string(block_size) + ", breaking");
            break;
        }

        result.push_back({
            .block_size = block_size,
            .is_block_occupied = !is_free
        });

        current += block_metadata_size + block_size;
    }

    return result;
}


allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept {
    return sorted_free_iterator(get_first_free());
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept {
    return sorted_free_iterator(nullptr);
}


allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept {
    return sorted_iterator(_trusted_memory);
}


allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept {
    std::byte* heap_start = reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size;
    size_t heap_size = *reinterpret_cast<size_t*>(
        reinterpret_cast<std::byte*>(_trusted_memory) +
        sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode)
    );
    std::byte* heap_end = heap_start + heap_size;

    return sorted_iterator(heap_end, get_first_free(), _trusted_memory);
}


allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()
    : _free_ptr(nullptr) {}


allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void* trusted)
    : _free_ptr(trusted) {}


void* allocator_sorted_list::sorted_free_iterator::operator*() const noexcept {
    return _free_ptr;
}

allocator_sorted_list::sorted_free_iterator& allocator_sorted_list::sorted_free_iterator::operator++() & noexcept {
    if (_free_ptr) {
        _free_ptr = *reinterpret_cast<void**>(_free_ptr);
    }
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int) {
    sorted_free_iterator old = *this;
    ++(*this);
    return old;
}

bool allocator_sorted_list::sorted_free_iterator::operator==(const sorted_free_iterator& other) const noexcept {
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(const sorted_free_iterator& other) const noexcept {
    return !(*this == other);
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept {
    if (!_free_ptr) return 0;
    return *reinterpret_cast<size_t*>(
        reinterpret_cast<std::byte*>(_free_ptr) + sizeof(void*));
}

bool allocator_sorted_list::sorted_iterator::operator==(const sorted_iterator& other) const noexcept {
    return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(const sorted_iterator& other) const noexcept {
    return !(*this == other);
}


allocator_sorted_list::sorted_iterator& allocator_sorted_list::sorted_iterator::operator++() & noexcept {
    std::byte* curr = reinterpret_cast<std::byte*>(_current_ptr);
    size_t block_size = *reinterpret_cast<size_t*>(curr + sizeof(void*));
    _current_ptr = curr + block_metadata_size + block_size;
    return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int) {
    sorted_iterator old = *this;
    ++(*this);
    return old;
}


size_t allocator_sorted_list::sorted_iterator::size() const noexcept {
    return *reinterpret_cast<size_t*>(
        reinterpret_cast<std::byte*>(_current_ptr) + sizeof(void*));
}


void* allocator_sorted_list::sorted_iterator::operator*() const noexcept {
    return _current_ptr;
}


allocator_sorted_list::sorted_iterator::sorted_iterator()
    : _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr) {}

allocator_sorted_list::sorted_iterator::sorted_iterator(void* trusted)
    : _trusted_memory(trusted) {
    _current_ptr = reinterpret_cast<void*>(
        reinterpret_cast<std::byte*>(trusted) + allocator_metadata_size);
    _free_ptr = *reinterpret_cast<void**>(
        reinterpret_cast<std::byte*>(trusted) + allocator_metadata_size - sizeof(void*));
}

allocator_sorted_list::sorted_iterator::sorted_iterator(void* current_ptr, void* free_ptr, void* trusted_memory)
        : _current_ptr(current_ptr), _free_ptr(free_ptr), _trusted_memory(trusted_memory) {}


bool allocator_sorted_list::sorted_iterator::occupied() const noexcept {
    void* free = _free_ptr;

    while (free) {
        if (free == _current_ptr) return false; // нашли в списке свободных — значит свободен
        free = *reinterpret_cast<void**>(free);
    }

    return true;
}
logger* allocator_sorted_list::get_logger() const {
    auto* ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<logger**>(ptr);
}

std::pmr::memory_resource* allocator_sorted_list::get_parent_resource() const noexcept {
    auto* ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    return *reinterpret_cast<std::pmr::memory_resource**>(ptr);
}

allocator_with_fit_mode::fit_mode allocator_sorted_list::get_fit_mode() const noexcept {
    auto* ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    ptr += sizeof(std::pmr::memory_resource*);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr);
}

void* allocator_sorted_list::get_first_free() const noexcept {
    auto* ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    ptr += sizeof(std::pmr::memory_resource*);
    ptr += sizeof(allocator_with_fit_mode::fit_mode);
    ptr += sizeof(std::byte[4]);
    return *reinterpret_cast<void**>(ptr);
}

void allocator_sorted_list::set_first_free(void* value) noexcept {
    auto* ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    ptr += sizeof(std::pmr::memory_resource*);
    ptr += sizeof(allocator_with_fit_mode::fit_mode);
    ptr += sizeof(std::byte[4]);
    *reinterpret_cast<void**>(ptr) = value;
}

std::mutex& allocator_sorted_list::get_mutex() const noexcept {
    auto* ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    ptr += sizeof(std::pmr::memory_resource*);
    ptr += sizeof(allocator_with_fit_mode::fit_mode);
    ptr += sizeof(std::byte[4]);
    ptr += sizeof(void*);
    return *reinterpret_cast<std::mutex*>(ptr);
}

size_t allocator_sorted_list::get_total_size() const noexcept {
    auto* ptr = reinterpret_cast<std::byte*>(&get_mutex());
    ptr += sizeof(std::mutex);
    return *reinterpret_cast<size_t*>(ptr);
}
