#include <stdlib.h>
#include <sys/mman.h>
#include "slab.h"

void **slab::current = nullptr;

void *slab::malloc() {
    if (current == nullptr) {
        allocate();
        if (current == nullptr)
            return nullptr;
    }
    void *ans = current;
    current = (void **) *current;
    return ans;
}

void slab::free(void *ptr) {
    *(void **) ptr = current;
    current = (void **) ptr;
}

void slab::allocate() {
    void *memory = mmap(nullptr, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    current = (void **) memory;
    if (memory != nullptr) {
        for (auto i = 0; i < 4096; i += SIZE) {
            auto temp = static_cast<char *>(memory) + i;
            *(void **) temp = 0;
            if (i != 0)
                *(void **) (temp - SIZE) = temp;
        }
    }
}