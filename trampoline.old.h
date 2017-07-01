int test() {
/*
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
            int sz = 8 * (args.INTEGER - 6 + std::max(args.SSE - 8, 0));

            // rax on top of stack
            // 4889E0    mov rax, rsp
            generator.add(mov_rax_rsp);

            // rax on first-1 arg on stack
            // 4805    add rax, imm
            generator.add4(add_rax_imm, sz + 8);
            // rsp on place for arg
            // 4881C4    add rsp, imm
            generator.add4(add_rsp_imm, 8);


            char *lbl = (char *) generator.get_ptr();
            // if no more args on stack then goto lbl2, else shift all args by 1 place
            // 4839E0    cmp rax, rsp
            generator.add(cmp_rax_rsp);

            // 74    je_imm
            generator.add(je_imm);

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
            *(void **) pcode = (void *) caller;
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
            *pcode++ = 0xc3;*/
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
*(void **) pcode = (void *) caller;
pcode += sizeof(caller);
// FFE0            jmp rax
*pcode++ = 0xFF;
*pcode++ = 0xE0;
*/
}