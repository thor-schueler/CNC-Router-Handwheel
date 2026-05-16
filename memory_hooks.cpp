#include <stddef.h>
#include <cstddef>
#include <new>
#include <esp_heap_caps.h>

void* operator new(std::size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void* operator new[](std::size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void operator delete(void* ptr) noexcept {
    heap_caps_free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    heap_caps_free(ptr);
}

void operator delete[](void* ptr) noexcept {
    heap_caps_free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
    heap_caps_free(ptr);
}
