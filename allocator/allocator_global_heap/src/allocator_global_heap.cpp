#include "../include/allocator_global_heap.h"
#include <new>
#include <sstream>
#include <cstdint>

allocator_global_heap::allocator_global_heap(logger* logger)
        : _logger(logger) {
    if (_logger) _logger->debug(get_typename() + "::ctor(): begin");
    if (_logger) _logger->trace(get_typename() + "::ctor(): constructed");
    if (_logger) _logger->debug(get_typename() + "::ctor(): end");
}

allocator_global_heap::~allocator_global_heap() {
    if (_logger) _logger->debug(get_typename() + "::dtor(): begin");
    if (_logger) _logger->trace(get_typename() + "::dtor(): destructed");
    if (_logger) _logger->debug(get_typename() + "::dtor(): end");
}

allocator_global_heap::allocator_global_heap(const allocator_global_heap& other)
        : _logger(other._logger) {
    if (_logger) _logger->debug(get_typename() + "::copy_ctor(): begin");
    if (_logger) _logger->trace(get_typename() + "::copy_ctor(): copying logger");
    if (_logger) _logger->debug(get_typename() + "::copy_ctor(): end");
}

allocator_global_heap& allocator_global_heap::operator=(const allocator_global_heap& other) {
    if (this != &other) {
        if (_logger) _logger->debug(get_typename() + "::copy_assign(): begin");
        _logger = other._logger;
        if (_logger) _logger->trace(get_typename() + "::copy_assign(): assigned logger");
        if (_logger) _logger->debug(get_typename() + "::copy_assign(): end");
    }
    return *this;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap&& other) noexcept
        : _logger(other._logger) {
    if (_logger) _logger->debug(get_typename() + "::move_ctor(): begin");
    if (_logger) _logger->trace(get_typename() + "::move_ctor(): moving logger");
    other._logger = nullptr;
    if (_logger) _logger->debug(get_typename() + "::move_ctor(): end");
}

allocator_global_heap& allocator_global_heap::operator=(allocator_global_heap&& other) noexcept {
    if (this != &other) {
        if (_logger) _logger->debug(get_typename() + "::move_assign(): begin");
        _logger = other._logger;
        other._logger = nullptr;
        if (_logger) _logger->trace(get_typename() + "::move_assign(): moved logger");
        if (_logger) _logger->debug(get_typename() + "::move_assign(): end");
    }
    return *this;
}

[[nodiscard]] void* allocator_global_heap::do_allocate_sm(size_t size) {
    if (_logger) _logger->debug(get_typename() + "::do_allocate_sm(): begin");

    const size_t total_size = size + sizeof(size_t);

    if (_logger) {
        _logger->trace(get_typename() + "::do_allocate_sm(): requested " + std::to_string(size) + " bytes");
        _logger->trace(get_typename() + "::do_allocate_sm(): allocating " + std::to_string(total_size) + " bytes (with metadata)");
    }

    try {
        void* raw_memory = ::operator new(total_size);
        *reinterpret_cast<size_t*>(raw_memory) = size;
        void* user_ptr = static_cast<char*>(raw_memory) + sizeof(size_t);

        if (_logger) {
            _logger->trace(get_typename() + "::do_allocate_sm(): metadata @ " + std::to_string(reinterpret_cast<uintptr_t>(raw_memory)));
            _logger->trace(get_typename() + "::do_allocate_sm(): user_ptr @ " + std::to_string(reinterpret_cast<uintptr_t>(user_ptr)));
        }

        if (_logger) _logger->debug(get_typename() + "::do_allocate_sm(): end");
        return user_ptr;
    } catch (const std::bad_alloc& e) {
        if (_logger) _logger->error(get_typename() + "::do_allocate_sm(): allocation failed: " + std::string(e.what()));
        throw;
    }
}

void allocator_global_heap::do_deallocate_sm(void* at) {
    if (_logger) _logger->debug(get_typename() + "::do_deallocate_sm(): begin");

    if (!at) {
        if (_logger) _logger->trace(get_typename() + "::do_deallocate_sm(): null pointer — nothing to deallocate");
        if (_logger) _logger->debug(get_typename() + "::do_deallocate_sm(): end");
        return;
    }

    void* raw_memory = static_cast<char*>(at) - sizeof(size_t);
    size_t size = *reinterpret_cast<size_t*>(raw_memory);

    if (_logger) {
        _logger->trace(get_typename() + "::do_deallocate_sm(): user_ptr @ " + std::to_string(reinterpret_cast<uintptr_t>(at)));
        _logger->trace(get_typename() + "::do_deallocate_sm(): metadata @ " + std::to_string(reinterpret_cast<uintptr_t>(raw_memory)));
        _logger->trace(get_typename() + "::do_deallocate_sm(): deallocating " + std::to_string(size) + " bytes");
    }

    ::operator delete(raw_memory);

    if (_logger) _logger->debug(get_typename() + "::do_deallocate_sm(): end");
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

inline logger* allocator_global_heap::get_logger() const {
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const {
    return "allocator_global_heap";
}
