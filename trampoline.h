#ifndef ASSEMBLER_TRAMPOLINE_H
#define ASSEMBLER_TRAMPOLINE_H

#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>
#include "trampoline_args.h"

void memprint(const char *src, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02hhX ", src[i]);
    }
    printf("\n");
}

template<typename T>
class trampoline {
public:
    template<typename F>
    trampoline(F func) {}

    T *get() const;
};

template<typename R, typename... Args>
class trampoline<R(Args...)> {
public:
    typedef R (*func_ptr_t)(Args ...);           // func_ptr_t is a pointer to function R(Args...)
    typedef R (*caller_ptr_t)(void *, Args ...); // caller_ptr_t is a pointer to function R(void *, Args...)
    typedef void (*deleter_ptr_t)(void *);       // deleter_ptr_t is a pointer to function void(void *)

    template<typename F>
    trampoline(F const &func) {
        printf("\n<constructor>\n");
        func_obj = new F(func);
        //printf("%s\n", typeid(F).name());
        printf("func_obj created: %p\n", func_obj);
        caller_ptr = &do_call<F>;
        deleter_ptr = &do_delete<F>;
        code = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        unsigned char *pcode = (unsigned char *) code;
        // 4889FE          mov rsi, rdi
        *pcode++ = 0x48;
        *pcode++ = 0x89;
        *pcode++ = 0xfe;
        // 48BF            movabs rdi, imm
        *pcode++ = 0x48;
        *pcode++ = 0xbf;
        *(void **) pcode = func_obj;
        pcode += sizeof(func_obj);
        // 48B8            movabs rax, imm
        *pcode++ = 0x48;
        *pcode++ = 0xb8;
        *(void **) pcode = (void *) caller_ptr;
        pcode += sizeof(caller_ptr);
        // FFE0            jmp rax
        *pcode++ = 0xFF;
        *pcode++ = 0xE0;

        printf("Code: ");
        memprint((char *) code, pcode - (unsigned char *) code);
        printf("</constructor>\n\n");
    }

    func_ptr_t get() const {
        return reinterpret_cast<func_ptr_t>(code);
    }

    ~trampoline() {
        printf("\n<destructor>\n");
        munmap(code, 4096);
        deleter_ptr(func_obj);
        printf("</destructor>\n\n");
    }

private:
    template<typename F>
    static void do_delete(void *ptr) {
        printf("do_delete(%p)\n", ptr);
        delete static_cast<F *>(ptr);
    }

    template<typename F>
    static R do_call(void *ptr, Args ...args) {
        printf("do_call(%p)\n", ptr);
        F func = *reinterpret_cast<F *>(ptr);
        return func(args...);
    }

    void *func_obj;
    void *code;
    caller_ptr_t caller_ptr;
    deleter_ptr_t deleter_ptr;
};

#endif // ASSEMBLER_TRAMPOLINE_H
