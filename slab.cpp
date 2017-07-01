#include <stdlib.h>
#include <sys/mman.h>
#include <exception>
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
    void *memory = mmap(nullptr, PAGE_SIZE * PAGE_AMOUNT, PROT_EXEC | PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    current = (void **) memory;
    for (int i = 0; i < PAGE_SIZE * PAGE_AMOUNT; i += SIZE) {
        char *temp = static_cast<char *>(memory) + i;
        *(void **) temp = 0;
        if (i != 0) {
            *(void **) (temp - SIZE) = temp;
        }
    }
}