#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include <cstdint>
#include <cstddef>

class memorystream {
public:
    memorystream(char *data) : data(data), ptr(data) {}

    /*memorystream &operator<<(memorystream &stream, const char *str) {
        for (const char *i = str; *i; ++i) {
            *(ptr++) = *i;
        }
        return stream;
    }*/

    void *reserve(size_t size) {
        void *start = ptr;
        ptr += size;
        return start;
    }

    void add(const char *operation) {
        for (const char *i = operation; *i; ++i) {
            *(ptr++) = *i;
        }
    }

    void add8(const char *operation, void *data) {
        add(operation);
        *(void **) ptr = data;
        ptr += 8;
    }

    void add4(const char *operation, int32_t data) {
        add(operation);
        *(int32_t *) ptr = data;
        ptr += 4;
    }

    void add1(const char *operation, int8_t data) {
        add(operation);
        *(int8_t *) ptr = data;
        ptr += 1;
    }

    void *get() const {
        return data;
    }

    void *get_ptr() const {
        return ptr;
    }


private:
    char *data, *ptr;
};

#endif // MEMORYSTREAM_H
