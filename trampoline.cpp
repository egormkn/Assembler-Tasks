#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>

template<typename T>
class trampoline {
public:
    template<typename F>
    trampoline(F func) {}

    ~trampoline() {}

    T *get() const {};
};

template<typename R, typename... Args>
class trampoline<R(Args...)> {
public:
    template<typename F>
    trampoline(F const &func) {
        func_obj = new F(func);
        caller = &do_call<F>;
        code = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        unsigned char *pcode = (unsigned char *) code;
        // 4889FE          mov rsi, rdi
        *pcode++ = 0x48;
        *pcode++ = 0x89;
        *pcode++ = 0xfe;
        // 48BF            mov rdi, imm
        *pcode++ = 0x48;
        *pcode++ = 0xbf;
        *(void **) pcode = func_obj;
        pcode += sizeof(func_obj);
        // 48B8            mov rax, imm
        *pcode++ = 0x48;
        *pcode++ = 0xb8;
        *(void **) pcode = (void *) caller;
        pcode += sizeof(caller);
        // FFE0            jmp rax
        *pcode++ = 0xFF;
        *pcode++ = 0xE0;
    }

    template<typename F>
    static R do_call(void *obj, Args ...args) {
        return (*(F *) obj)(args...);
    }

    R (*get() const )(Args ...args) {
        return (R (*)(Args ...args)) code;
    }

private:
    void *func_obj;
    void *code;

    R (*caller)(void *obj, Args ...args);
};

int main() {
    int b = 123;

    auto lambda = [&](int a) {
        return printf("%d %d %d\n", a, b, 0);
    };

    trampoline<int(int)> tr(lambda);
    auto p = tr.get();

    p(5);

    b = 124;
    p(6);
}