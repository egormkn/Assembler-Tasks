#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>

template<typename T>
struct trampoline {
    template<typename F>
    trampoline(F func) {}

    ~trampoline();

    T *get() const;
};

template<typename R, typename A0>
struct trampoline<R(A0)> {
    template<typename F>
    trampoline(F const &func)
            : func_obj(new F(func)), caller(&do_call < F > ) {
        code = mmap(nullptr, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char *pcode = (char *) code;
        //4889FE        mov rsi, rdi
        *pcode++ = 0x48;
        *pcode++ = 0x89;
        *pcode++ = 0xfe;
        // 48BF         mov rdi, imm
        *pcode++ = 0x48;
        *pcode++ = 0xbf;
        *(void **) pcode = func_obj;
        pcode += 8;
        // 48B8         mov rax, imm        
        *pcode++ = 0x48;
        *pcode++ = 0xb8;
        *(void **) pcode = (void *) &do_call<F>;
        pcode += 8;
        // FFE0         jmp rax
        *pcode++ = 0xFF;
        *pcode++ = 0xE0;
    }

    template<typename F>
    static R do_call(void *obj, A0 a0) {
        return (*(F *) obj)(a0);
    }

    R (*get() const )(A0 a0) {
        return (R (*)(A0 a0)) code;
    }

private:
    void *func_obj;
    void *code;

    R (*caller)(void *obj, A0 a0);
};

int main() {
    int b = 123;

    trampoline<int(int)> tr([&](int a) { return printf("%d %d\n", a, b); });
    auto p = tr.get();

    p(5);
    b = 124;

    p(6);
}