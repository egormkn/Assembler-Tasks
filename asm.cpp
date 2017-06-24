#include <cstdint>


const char *operations[] = {
        "\x48\x89\xfe" /*movq %%rdi, %%rsi;*/,
        "\x48\x89\xf2" /*movq %%rsi, %%rdx;*/,
        "\x48\x89\xd1" /*movq %%rdx, %%rcx;*/,
        "\x49\x89\xc8" /*movq %%rcx, %%r8;*/,
        "\x4d\x89\xc1" /*movq %%r8, %%r9;*/,
        "\x41\x51" /*push %%r9;*/
};

const char *jmp_rax = "\xff\xe0";
const char *jmp_r9 = "\x41\xff\xe1";

const char *call_rax = "\xff\xd0";
const char *call_r11 = "\x41\xff\xd3";

const char *mov_rax_in_r11 = "\x49\x89\xc3";
const char *mov_rsp_in_rsi = "\x48\x89\xe6";
const char *mov_r11_in_rax = "\x4c\x89\xd8";
const char *mov_rsp_in_rax = "\x48\x89\xe0";
const char *mov_rsi_in_rax = "\x48\x89\xf0";

const char *mov_rax_at_addres_rdi = "\x48\x89\x07"; // movq %%rax, (%%rdi);
const char *mov_rdi_at_addres_rsp = "\x48\x89\x3c\x24";
const char *mov_r11_at_addres_rsp = "\x4c\x89\x1c\x24";
const char *mov_rdi_at_addres_rsp_m8 = "\x48\x89\x7c\x24\xf8";

const char *mov_in_rdi_const_8b = "\x48\xbf";
const char *mov_in_rax_const_8b = "\x48\xb8";
const char *mov_in_r8_const_8b = "\x49\xb8";
const char *mov_in_r9_const_8b = "\x49\xb9";
const char *mov_in_r11_const_8b = "\x49\xbb";

const char *mov_at_addres_rdi_in_rax = "\x48\x8b\x07"; // movq (%%rdi), %%rax;
const char *mov_at_addres_r8_in_r9 = "\x4d\x8b\x08"; // movq (%%r8), %%r9;
const char *mov_at_addres_rsp_in_rdi = "\x48\x8b\x3c\x24";
const char *mov_at_addres_rsi_in_r11 = "\x4c\x8b\x1e";


const char *pop_rax = "\x58";
const char *pop_rdi = "\x5f";
const char *pop_rsi = "\x5e";
const char *pop_r9 = "\x41\x59";
const char *pop_r11 = "\x41\x5b";


const char *push_rax = "\x50";
const char *push_rdi = "\x57";
const char *push_r9 = "\x41\x51";
const char *push_r11 = "\x41\x53";

const char *push_0x0 = "\x6a\x00";


const char *sub_rsi_const_4b = "\x48\x81\xee";
const char *sub_rsp_const_4b = "\x48\x81\xec";
const char *add_rsp_const_4b = "\x48\x81\xc4";
const char *add_rsi_const_4b = "\x48\x81\xc6";

const char *sub_rax_rsi = "\x48\x29\xf0";


const char *cmp_rsi_rsp = "\x48\x39\xe6";

const char *je = "\x74";
const char *jmp = "\xeb";

const char *ret = "\xc3";

const char *int3 = "\xCC";
const char *nop = "\x90";



void add(char* &ptr, const char* operation) {
    for (const char *i = operation; *i; ++i) {
        *(ptr++) = *i;
    }
}

void add_8(char* &ptr, const char* operation, void* data) {
    add(ptr, operation);
    *(void**)ptr = data;
    ptr += 8;
}

void add_4(char* &ptr, const char* operation, int32_t data) {
    add(ptr, operation);
    *(int32_t*)ptr = data;
    ptr += 4;
}



/*
int test() {
    char *p = new char[100];
    if (num_not_fract_args >= 6) {
        add(p, pop_r11);
        add(p, push_rax);
        for (int w = 5; w >= 1; w--) {
            add(p, operations[w]);
        }
        add(p, push_rdi);
        //  rdi и rsi свободны, в r11 - адрес возврата, на вершине стека - первый аргумент.
        int size = 8 * (args_info<Args...>::num_not_fract_args - 6 +
                        std::max(args_info<Args...>::num_fractional_args - 8, 0));
        //cout << size << "   !!!!!\n";

        add(p, mov_rsp_in_rsi);
        add_4(p, add_rsi_const_4b, size + 8 * 2);
        add_4(p, add_rsp_const_4b, 8 * 2);

        char* label0 = p;

        add(p, cmp_rsi_rsp);
        add(p, je);
        char* pos_for_je = p;
        p++;

        {
            add_4(p, add_rsp_const_4b, 8);
            add(p, mov_at_addres_rsp_in_rdi);
            add(p, mov_rdi_at_addres_rsp_m8);
            add(p, jmp);
            *p = (label0 - (p + 1));
            p++;
        }
        (*pos_for_je) = (char)(p - (pos_for_je + 1));

        add(p, mov_r11_at_addres_rsp);
        add_4(p, sub_rsp_const_4b, size + 8 * 2);
        add(p, pop_rsi);

        add_8(p, mov_in_rdi_const_8b, func_obj);
        add_8(p, mov_in_r11_const_8b, (void*)&do_call<F>);

        add(p, call_r11);

        add(p, pop_r9);

        add(p, mov_rsp_in_rsi);
        add_4(p, add_rsi_const_4b, size);

        add(p, mov_at_addres_rsi_in_r11);
        add(p, pop_r9);
        add(p, push_r11);

        add(p, ret);

        //std::cout << (int)(p - (char*)code) << "\n\n";
    } else {
        for (int w = args_info<Args...>::num_not_fract_args - 1; w >= 0; w--) {
            add(p, operations[w]);
        }
        add_8(p, mov_in_rdi_const_8b, func_obj);
        add_8(p, mov_in_r9_const_8b, (void*)&do_call<F>);

        add(p, jmp_r9);
    }

}*/