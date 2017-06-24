#ifndef ASSEMBLER_TRAMPOLINE_H
#define ASSEMBLER_TRAMPOLINE_H

#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>
#include "trampoline_args.h"
#include "slab.h"
#include "memorystream.h"


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
        func_obj = new F(func);
        caller_ptr = &do_call<F>;
        deleter_ptr = &do_delete<F>;

        trampoline_args<Args ...> args;
        int INTEGER_ARGS = args.INTEGER;
        int SSE_ARGS = args.SSE;

        code = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        //code = slab::malloc();

        memorystream generator((char *) code);
        char *pcode;




        const char* mov_r9_r8 = "\x4D\x89\xC1";
        const char* mov_r8_rcx = "\x49\x89\xC8";
        const char* mov_rcx_rdx = "\x48\x89\xD1";
        const char* mov_rdx_rsi = "\x48\x89\xF2";
        const char* mov_rsi_rdi = "\x48\x89\xF2";

        const char* mov_rdi_imm = "\x48\xBF";
        const char* mov_rax_imm = "\x48\xB8";
        const char* jmp_rax = "\xFF\xE0";

        const char* mov_r11_mem_rsp = "\x4C\x8B\x1C\x24";
        const char* push_r9 = "\x41\x51";

        const char* mov_rax_rsp = "\x48\x89\xE0";
        const char* add_rax_imm = "\x48\x05";
        const char* add_rsp_imm = "\x48\x81\xC4";

        if (INTEGER_ARGS < 6) {
            switch (INTEGER_ARGS) {
                case 5: // 4D89C1    mov r9, r8
                    generator.add(mov_r9_r8);
                case 4: // 4989C8    mov r8, rcx
                    generator.add(mov_r8_rcx);
                case 3: // 4889D1    mov rcx, rdx
                    generator.add(mov_rcx_rdx);
                case 2: // 4889F2    mov rdx, rsi
                    generator.add(mov_rdx_rsi);
                case 1: // 4889FE    mov rsi, rdi
                    generator.add(mov_rsi_rdi);
                default:
                    break;
            }
            // 48BF    mov rdi, imm
            generator.add8(mov_rdi_imm, func_obj);
            // 48B8    mov rax, imm
            generator.add8(mov_rax_imm, (void *) caller_ptr);
            // FFE0    jmp rax
            generator.add(jmp_rax);
        } else {
            // return address to r11
            // 415B              pop r11
            // *pcode++ = 0x41;
            // *pcode++ = 0x5B;
            // free place on stack
            //50                push rax
            // *pcode++ = 0x50;

            // 4C8B1C24    mov r11, [rsp]
            generator.add(mov_r11_mem_rsp);
            // 6th arg on stack
            // 4151    push r9
            generator.add(push_r9);

            // 4D89C1    mov r9, r8
            generator.add(mov_r9_r8);
            // 4989C8    mov r8, rcx
            generator.add(mov_r8_rcx);
            // 4889D1    mov rcx, rdx
            generator.add(mov_rcx_rdx);
            // 4889F2    mov rdx, rsi
            generator.add(mov_rdx_rsi);
            // 4889FE    mov rsi, rdi
            generator.add(mov_rsi_rdi);
            pcode = (char *) generator.get_ptr();

            // 8 SSE args go to xmm0 - xmm7. Others go on stack
            int sz = 8 * (INTEGER_ARGS - 6 + std::max(SSE_ARGS - 8, 0));

            // rax on top of stack
            // 4889E0    mov rax, rsp
            generator.add(mov_rax_rsp);

            // rax on first-1 arg on stack
            // 4805    add rax, imm
            generator.add4(add_rax_imm, sz + 8);
            // rsp on place for arg
            // 4881C4    add rsp, imm
            generator.add4(add_rsp_imm, 8);


            const char* cmp_rax_rsp = "\x48\x39\xE0";
            const char* je = "\x74";
            const char* mov_rdi_mem_rsp = "\x48\x8B\x3C\x24";

            char *lbl = (char *) generator.get_ptr();
            // if no more args on stack then goto lbl2, else shift all args by 1 place
            // 4839E0    cmp rax, rsp
            generator.add(cmp_rax_rsp);

            // 74    je
            generator.add(je);

            char *lbl2 = (char *) generator.get_ptr();
            generator.add(" ");
            pcode++;


            // shift all args by 1 place
            // rsp from free place to arg for shift
            // 4881C4    add rsp, imm
            generator.add4(add_rsp_imm, 8);
            // save arg for shift in rdi
            // 488B3C24    mov rdi, [rsp]
            generator.add(mov_rdi_mem_rsp);
            pcode = (char *) generator.get_ptr();
            // push saved arg to free place
            // 48897C24F8    mov [rsp-0x8],rdi
            *pcode++ = 0x48;
            *pcode++ = 0x89;
            *pcode++ = 0x7c;
            *pcode++ = 0x24;
            *pcode++ = 0xf8;
            // do that in loop
            //EB                jmp
            *pcode++ = 0xeb;

            *pcode = lbl - pcode - 1;
            pcode++;

            *lbl2 = pcode - lbl2 - 1;

            // return address (saved in r11) on bottom of stack
            //4C891C24          mov [rsp],r11
            *pcode++ = 0x4c;
            *pcode++ = 0x89;
            *pcode++ = 0x1c;
            *pcode++ = 0x24;

            // rsp on 1st arg
            //4881EC            sub rsp,...
            *pcode++ = 0x48;
            *pcode++ = 0x81;
            *pcode++ = 0xec;
            *(int32_t *) pcode = sz + 8;
            pcode += 4;

            // 48BF             mov rdi, imm
            *pcode++ = 0x48;
            *pcode++ = 0xbf;
            *(void **) pcode = func_obj;
            pcode += 8;

            //48B8              mov rax, imm
            *pcode++ = 0x48;
            *pcode++ = 0xb8;
            *(void **) pcode = (void *) &do_call<F>;
            pcode += 8;

            // func call
            //FFD0            call rax
            *pcode++ = 0xFF;
            *pcode++ = 0xd0;

            // remove 6th arg from stack
            //4159              pop r9
            *pcode++ = 0x41;
            *pcode++ = 0x59;


            // rax on ret addr
            // 4889E0            mov rax,rsp
            // *pcode++ = 0x48;
            // *pcode++ = 0x89;
            // *pcode++ = 0xe0;
            // 4881C0            add rax, ...
            // *pcode++ = 0x48;
            // *pcode++ = 0x81;
            // *pcode++ = 0xc0;
            // *(int32_t*)pcode = sz;
            // pcode += 4;


            // save ret addr to r11
            // 4C8B18            mov r11,[rax]
            // *pcode++ = 0x4c;
            // *pcode++ = 0x8b;
            // *pcode++ = 0x18;


            // 4C8B9C24const   mov r11,[rsp+const]
            *pcode++ = 0x4c;
            *pcode++ = 0x8b;
            *pcode++ = 0x9c;
            *pcode++ = 0x24;
            *(int32_t *) pcode = sz;
            pcode += 4;


            // ret addr on top of stack, as it was before the call
            // 4159              pop r9
            // *pcode++ = 0x41;
            // *pcode++ = 0x59;
            // 4153              push r11
            // *pcode++ = 0x41;
            // *pcode++ = 0x53;


            // 4C891C24          mov [rsp],r11
            *pcode++ = 0x4c;
            *pcode++ = 0x89;
            *pcode++ = 0x1c;
            *pcode++ = 0x24;


            // C3                ret
            *pcode++ = 0xc3;
        }



/*
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
*/
        /*printf("Code: ");
        memprint((char *) code, pcode - (unsigned char *) code);*/
    }

    func_ptr_t get() const {
        return reinterpret_cast<func_ptr_t>(code);
    }

    ~trampoline() {
        munmap(code, 4096);
        //slab::free(code);
        deleter_ptr(func_obj);
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
    caller_ptr_t caller_ptr;
    deleter_ptr_t deleter_ptr;
};

#endif // ASSEMBLER_TRAMPOLINE_H
