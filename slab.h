#ifndef SLAB_H
#define SLAB_H

class slab {
public:
    static void *malloc();
    static void free(void *ptr);

private:
    static void **current;
    static const int SIZE = 128;
    static const int PAGE_AMOUNT = 1;
    static const int PAGE_SIZE = 4096;
    static void allocate();
};

#endif // SLAB_H




