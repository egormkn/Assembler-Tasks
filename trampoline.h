#ifndef ASSEMBLER_TRAMPOLINE_H
#define ASSEMBLER_TRAMPOLINE_H

#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>
#include <mmintrin.h>
#include "slab.h"
#include "memorystream.h"

template<typename ...Args>
struct trampoline_args;

template<>
struct trampoline_args<> {
    static const int INTEGER = 0;
    static const int SSE = 0;
};

template<typename First, typename ...Other>
struct trampoline_args<First, Other...> {
    static const int INTEGER = trampoline_args<Other...>::INTEGER + 1;
    static const int SSE = trampoline_args<Other...>::SSE;
    static_assert(sizeof(First) <= 8, "Arguments larger than 8 bytes are unsupported.");
};

template<typename ...Other>
struct trampoline_args<float, Other...> {
    static const int INTEGER = trampoline_args<Other...>::INTEGER;
    static const int SSE = trampoline_args<Other...>::SSE + 1;
};

template<typename ...Other>
struct trampoline_args<double, Other...> {
    static const int INTEGER = trampoline_args<Other...>::INTEGER;
    static const int SSE = trampoline_args<Other...>::SSE + 1;
};

template<typename ...Other>
struct trampoline_args<__m64, Other...> {
    static const int INTEGER = trampoline_args<Other...>::INTEGER;
    static const int SSE = trampoline_args<Other...>::SSE + 1;
};

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
    typedef R (*func_t)(Args ...);             // func_t is a pointer to function R(Args...)
    typedef R (*caller_t)(void *, Args ...);   // caller_t is a pointer to function R(void *, Args...)
    typedef void (*deleter_t)(void *);         // deleter_t is a pointer to function void(void *)

    template<typename F>
    trampoline(F const &func) {
        func_obj = new F(std::move(func));
        caller = &do_call<F>;
        deleter = &do_delete<F>;
        code = slab::malloc();
        generate((char *) code);
    }

    func_t get() const {
        return reinterpret_cast<func_t>(code);
    }

    ~trampoline() {
        deleter(func_obj);
        slab::free(code);
    }

    trampoline(trampoline &&other) {
        func_obj = other.func_obj;
        code = other.code;
        deleter = other.deleter;
        other.func_obj = nullptr;
    }

    trampoline &operator=(trampoline &&other) {
        func_obj = other.func_obj;
        code = other.code;
        deleter = other.deleter;
        other.func_obj = nullptr;
        return *this;
    }

    trampoline(const trampoline &) = delete;

private:
    void generate(char *code) {
        trampoline_args<Args ...> args;
        memorystream generator(code);

        typedef const char* asm_t;

        asm_t push_r9 = "\x41\x51",
                mov_r9_r8 = "\x4D\x89\xC1",
                mov_r8_rcx = "\x49\x89\xC8",
                mov_rcx_rdx = "\x48\x89\xD1",
                mov_rdx_rsi = "\x48\x89\xF2",
                mov_rsi_rdi = "\x48\x89\xFE";

        asm_t shift[] = {
                mov_rsi_rdi,
                mov_rdx_rsi,
                mov_rcx_rdx,
                mov_r8_rcx,
                mov_r9_r8
        };

        asm_t mov_rdi_imm = "\x48\xBF";
        asm_t mov_rax_imm = "\x48\xB8";
        asm_t jmp_rax = "\xFF\xE0";

        asm_t mov_r11_mem_rsp = "\x4C\x8B\x1C\x24";

        asm_t mov_rax_rsp = "\x48\x89\xE0";
        asm_t add_rax_imm = "\x48\x05";
        asm_t add_rsp_imm = "\x48\x81\xC4";

        asm_t cmp_rax_rsp = "\x48\x39\xE0";
        asm_t je_imm = "\x74";
        asm_t mov_rdi_mem_rsp = "\x48\x8B\x3C\x24";
        asm_t mov_mem_rsp_minus_0x8_rdi = "\x48\x89\x7C\x24\xF8";
        asm_t jmp_imm = "\xEB";

        asm_t mov_mem_rsp_r11 = "\x4c\x89\x1c\x24";
        asm_t sub_rsp_imm = "\x48\x81\xec";
        asm_t call_rax = "\xff\xd0";
        asm_t pop_r9 = "\x41\x59";
        asm_t ret = "\xc3";
        asm_t mov_r11_mem_rsp_plus_imm = "\x4c\x8b\x9c\x24";

        if (args.INTEGER < 6) {                             // If shifted integer arguments fit on the stack
            for (int i = args.INTEGER - 1; i >= 0; --i) {   // Shift integer arguments
                generator.add(shift[i]);
            }
            generator.add8(mov_rdi_imm, func_obj);          // Put function object to rdi as first argument
            generator.add8(mov_rax_imm, (void *) caller);   // Put caller pointer to rax
            generator.add(jmp_rax);                         // Jump to caller
        } else {
            // 6 INTEGER args and up to 8 SSE args are in registers, others are on stack
            int stack_size = 8 * (args.INTEGER - 6 + std::max(args.SSE - 8, 0));

            generator.add(mov_r11_mem_rsp);  // Save return address from the top of the stack to r11
            generator.add(push_r9);          // Push 6th argument on stack
            for (int i = 4; i >= 0; --i) {   // Shift integer arguments
                generator.add(shift[i]);
            }
            generator.add(mov_rax_rsp);      // Put top of the current stack to rax
            generator.add4(add_rax_imm, stack_size + 8);   // Set rax to the last argument
            generator.add4(add_rsp_imm, 8);  // Set rsp to return address

            // loop:
            char *loop = (char *) generator.get_ptr();
            generator.add(cmp_rax_rsp);      // Check if all arguments were shifted
            generator.add(je_imm);           // If so, go to label_2
            char *label_2 = (char *) generator.reserve(1); // Reserve 1 byte for label

            generator.add4(add_rsp_imm, 8);            // Set rsp to next argument
            generator.add(mov_rdi_mem_rsp);            // Copy argument to rdi
            generator.add(mov_mem_rsp_minus_0x8_rdi);  // Put argument from rdi to rsp-8

            generator.add(jmp_imm);
            char *label_loop = (char *) generator.reserve(1);
            *label_loop = (char) (loop - (char *) generator.get_ptr());
            *label_2 = (char) ((char *) generator.get_ptr() - label_2 - 1);

            generator.add(mov_mem_rsp_r11); // move saved in r11 return adress to rsp (onto the bottom of stack)
            generator.add4(sub_rsp_imm, stack_size + 8); // transfer rsp to the top of stack

            generator.add8(mov_rdi_imm, func_obj);          // Put function object to rdi as first argument
            generator.add8(mov_rax_imm, (void *) caller);   // Put caller pointer to rax

            generator.add(call_rax);  // Call the function
            generator.add(pop_r9);    // Remove 6th argument from stack because of callconv
            generator.add4(mov_r11_mem_rsp_plus_imm, stack_size); // save into r11 address of current rsp plus shift on current stack size
            //have one less argument in stack cause of one argument have already been deleted from stack

            generator.add(mov_mem_rsp_r11); // set correct value, previously stored in r11, to rsp
            generator.add(ret); // Return
        }
    }

private:
    template<typename F>
    static void do_delete(void *ptr) {
        delete static_cast<F *>(ptr);
    }

    template<typename F>
    static R do_call(void *ptr, Args ...args) {
        F func = *reinterpret_cast<F *>(ptr);
        return func(args...);
    }

    void *func_obj;
    void *code;
    caller_t caller;
    deleter_t deleter;
};

#endif // ASSEMBLER_TRAMPOLINE_H
