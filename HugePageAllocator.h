#pragma once

#include <memory>
#include <vector>

#define NOMINMAX
#include <windows.h>

namespace slowballs
{

template<typename T>
struct HugePageAllocator
{
    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;

    size_t largePageMinimum;
    unsigned char* ptr;

    HugePageAllocator()
    : largePageMinimum(GetLargePageMinimum())
    , ptr(static_cast<unsigned char*>(VirtualAlloc(nullptr, largePageMinimum, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE)))
    {
    }

    T* allocate(size_t n)
    {
        const auto bytes = sizeof(T) * n;
        auto data = new (ptr) T[n];
        ptr += bytes;
        return data;
    }

    void deallocate(T* ptr, size_t n)
    {
        //VirtualFree(ptr, 0, MEM_RELEASE);
    }
};

} // namespace slowballs
