                                   // "v0", "s0", "a3", "a2", "a1", "a0", "a4", "a5", "t0", "__", "t1", "__", "s1", "t2",
char *x86_64_regs64[REGISTERS] = { "rax", "rbx", "r10", "rdx", "rsi", "rdi", "r8", "r9", "r15", "r12", "rcx", "r11", "r13", "r14",
    "qword [rbp - 8]", "qword [rbp - 16]", "qword [rbp - 24]", "qword [rbp - 32]",
    "qword [rbp - 40]", "qword [rbp - 48]", "qword [rbp - 56]", "qword [rbp - 64]",
    "qword [rbp - 72]", "qword [rbp - 80]", "qword [rbp - 88]", "qword [rbp - 96]", "qword [rbp - 104]",
    "qword [rbp - 112]", "qword [rbp - 120]", "qword [rbp - 128]", "qword [rbp - 136]", "qword [rbp - 144]",
};

char *x86_64_regs32[REGISTERS] = { "eax", "ebx", "r10d", "edx", "esi", "edi", "r8d", "r9d", "r15d", "r12d", "ecx", "r11d", "r13d", "r14d",
    "dword [rbp - 8]", "dword [rbp - 16]", "dword [rbp - 24]", "dword [rbp - 32]",
    "dword [rbp - 40]", "dword [rbp - 48]", "dword [rbp - 56]", "dword [rbp - 64]",
    "dword [rbp - 72]", "dword [rbp - 80]", "dword [rbp - 88]", "dword [rbp - 96]", "dword [rbp - 104]",
    "dword [rbp - 112]", "dword [rbp - 120]", "dword [rbp - 128]", "dword [rbp - 136]", "dword [rbp - 144]",
};

char *x86_64_regs16[REGISTERS] = { "ax", "bx", "r10w", "dx", "si", "di", "r8w", "r9w", "r15w", "r12w", "cx", "r11w", "r13w", "r14w",
    "word [rbp - 8]", "word [rbp - 16]", "word [rbp - 24]", "word [rbp - 32]",
    "word [rbp - 40]", "word [rbp - 48]", "word [rbp - 56]", "word [rbp - 64]",
    "word [rbp - 72]", "word [rbp - 80]","word [rbp - 88]", "word [rbp - 96]", "word [rbp - 104]",
    "word [rbp - 112]", "word [rbp - 120]", "word [rbp - 128]", "word [rbp - 136]", "word [rbp - 144]",
};

char *x86_64_regs8[REGISTERS] = { "al", "bl", "r10b", "dl", "sil", "dil", "r8b", "r9b", "r15b", "r12b", "cl", "r11b", "r13b", "r14b",
    "byte [rbp - 8]", "byte [rbp - 16]", "byte [rbp - 24]", "byte [rbp - 32]",
    "byte [rbp - 40]", "byte [rbp - 48]", "byte [rbp - 56]", "byte [rbp - 64]",
    "byte [rbp - 72]", "byte [rbp - 80]","byte [rbp - 88]", "byte [rbp - 96]", "byte [rbp - 104]",
    "byte [rbp - 112]", "byte [rbp - 120]", "byte [rbp - 128]", "byte [rbp - 136]", "byte [rbp - 144]",
};

char *x86_64_size_to_name(int size){
    switch (size){
        case 1: return "byte";
        case 2: return "word";
        case 4: return "dword";
        case 8: return "qword";
    };
    return 0;
};

int x86_64_is_real_reg(char *reg){
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, x86_64_regs64[i]) == 0) {
            return 0;
        };
    }
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, x86_64_regs32[i]) == 0) {
            return 0;
        };
    }
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, x86_64_regs16[i]) == 0) {
            return 0;
        };
    }
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, x86_64_regs8[i]) == 0) {
            return 0;
        };
    }
    return -1;
}
char *x86_64_change_reg_size(int reg, int size){
    switch (size){
        case 1: return x86_64_regs8[reg];
        case 2: return x86_64_regs16[reg];
        case 4: return x86_64_regs32[reg];
        case 8: return x86_64_regs64[reg];
    };
    return x86_64_regs64[reg];
}
char *x86_64_reg_to_name(AST_Reg reg){
    switch (reg.size){
        case 1: return x86_64_regs8[reg.reg];
        case 2: return x86_64_regs16[reg.reg];
        case 4: return x86_64_regs32[reg.reg];
        case 8: return x86_64_regs64[reg.reg];
        case 0: return x86_64_regs64[reg.reg];
    };
    char string[100];
    snprintf(string, 100, "Register doesn't exist");
    error_generate("InvalidRegisterError", string);
    return "";
};
AST_Reg x86_64_reg_real_to_num(char *reg){
    AST_Reg reg1 = (AST_Reg){0};
    reg1.reg = -1;
    for (int i=0; i<REGISTERS; i++){
        if (string_compare(x86_64_regs8[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 1;
        };
    };
    for (int i=0; i<REGISTERS; i++){
        if (string_compare(x86_64_regs16[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 2;
        };
    };
    for (int i=0; i<REGISTERS; i++){
        if (string_compare(x86_64_regs32[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 4;
        };
    };
    for (int i=0; i<REGISTERS; i++){
        if (string_compare(x86_64_regs64[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 8;
        };
    };
    return reg1;
}
Compiler *x86_64_init(Reviser *reviser, char *file){
    Compiler *compiler = malloc(sizeof(Compiler));
    FILE *f;
    compiler->_asm = find_available_tmp_file("asm");
    f = fopen(compiler->_asm, "w"); 
    if (!f) {
        perror("fopen");
        exit(1);
    }
    compiler->f = f;
    compiler->data = malloc(500);
    compiler->name = file;
    compiler->asts = reviser->asts;
    compiler->astlen = reviser->astlen;
    compiler->cur = 0;
    compiler->templabel = 0;
    compiler->global = reviser->global;
    fprintf(compiler->f, "default rel\nBITS 64\nsection .text\nglobal main\n");
    return compiler;
}

void x86_64_close(Compiler *compiler){
    fprintf(compiler->f, "section .data\n%s\n", compiler->data);
    fclose(compiler->f);
    char string[100];
    compiler->_o = find_available_tmp_file("o");
    snprintf(string, 100, "yasm -f macho64 %s -o %s", compiler->_asm, compiler->_o);
    system(string);

    // snprintf(string, 100, "bat %s", compiler->_asm);
    // system(string);
    // snprintf(string, 100, "cat %s | pbcopy", compiler->_asm);
    // system(string);

    remove(compiler->_asm);
    snprintf(string, 100, "clang -arch x86_64 %s -o %s -e main -Wl,-w -Wl,-platform_version,macos,11.0,11.0", compiler->_o, compiler->name);
    system(string);
    remove(compiler->_o);
}


char *x86_64_format(char *str, int type){
    if (type != AST_STRING){
        return strdup(str);
    }
    char *out = malloc(100);
    int b = 0;
    out[b++] = '\'';
    for (int i=0; i<strlen(str); i++){
        if (str[i] == '\n'){
            out[b++] = '\'';
            out[b++] = ',';
            out[b++] = '1';
            out[b++] = '0';
            out[b++] = ',';
            out[b++] = '\'';
        }else if (str[i] == '\\'){
            if (i == strlen(str)) break;
            if (str[i + 1] == '0'){
                out[b++] = '\'';
                out[b++] = ',';
                out[b++] = '0';
                out[b++] = ',';
                out[b++] = '\'';
            }
            i++;
        }else {
            out[b++] = str[i];
        }
    }
    out[b++] = '\'';
    out[b++] = ',';
    out[b++] = ' ';
    out[b++] = '0';
    return out;
}

char *x86_64_eat_expr(Compiler *compiler, AST *ast, int size);



char *x86_64_move(Compiler *compiler, char *reg, char *buf, int typeinfo){
    if (strchr(reg, '[') && strchr(buf, '[')){
        char *right = x86_64_move(compiler, "r11", buf, typeinfo);
        AST_Reg _reg = x86_64_reg_real_to_num("r11");
        char *s = x86_64_change_reg_size(_reg.reg, typeinfo);
        return x86_64_move(compiler, reg, s, typeinfo);
    }
    
    char *op = "mov";
    char *final_reg = reg;
    char *final_buf = buf;
    
    AST_Reg _reg = x86_64_reg_real_to_num(reg);
    if (_reg.reg != -1) {
        final_reg = x86_64_change_reg_size(_reg.reg, typeinfo);
    } else {
        final_reg = reg; 
    }
    
    AST_Reg _buf = x86_64_reg_real_to_num(buf);
    if (_buf.reg != -1) {
        final_buf = x86_64_change_reg_size(_buf.reg, typeinfo);
    } else {
        final_buf = buf;
    }

    compiler_write_text_line(compiler, "%s %s, %s", op, final_reg, final_buf);
    return final_reg;
    
}

void x86_64_compare(Compiler *compiler, AST *ast){
        char *left = x86_64_eat_expr(compiler, ast->data.expr.left, -1);
        char *right = x86_64_eat_expr(compiler, ast->data.expr.right, -1);
        char *reg = x86_64_move(compiler, "r12", left, ast->data.expr.left->typeinfo);
        compiler_write_text_line(compiler, "cmp %s, %s", reg, right);
}


void x86_64_eat_call(Compiler *compiler, AST ast){
    if(ast.type == AST_CALL){
        char string[100];
        snprintf(string, 100, "_fun%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "call %s", strdup(string));
    }else if(ast.type == AST_CALLIF){
        char string[100];
        snprintf(string, 100, ".L%d", compiler->templabel++);
        char *reg = x86_64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        if (strchr(reg, '[')){
            x86_64_move(compiler, "r12", reg, ast.data.jmpif.reg->typeinfo);
            char *r12=x86_64_change_reg_size(x86_64_reg_real_to_num("r12").reg, ast.data.jmpif.reg->typeinfo);compiler_write_text_line(compiler, "test %s, %s", r12, r12);
        }else {
            compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        };
        compiler_write_text_line(compiler, "jz %s", strdup(string));
        char string1[100];
        snprintf(string1, 100, "_fun%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "call %s", strdup(string1));
        compiler_write_text(compiler, "%s:\n", strdup(string));
    }else if(ast.type == AST_CALLIFN){
        char string[100];
        snprintf(string, 100, ".L%d", compiler->templabel++);
        char *reg = x86_64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        if (strchr(reg, '[')){
            x86_64_move(compiler, "r12", reg, ast.data.jmpif.reg->typeinfo);
            char *r12=x86_64_change_reg_size(x86_64_reg_real_to_num("r12").reg, ast.data.jmpif.reg->typeinfo);compiler_write_text_line(compiler, "test %s, %s", r12, r12);
        }else {
            compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        };
        compiler_write_text_line(compiler, "jnz %s", strdup(string));
        char string1[100];
        snprintf(string1, 100, "_fun%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "call %s", strdup(string1));
        compiler_write_text(compiler, "%s:\n", strdup(string));
    }
}
void x86_64_eat_syscall(Compiler *compiler, AST ast){
    compiler_write_text_line(compiler, "mov rax, %s", ast.data.text.value);
    compiler_write_text_line(compiler, "syscall");
}



char *x86_64_eat_expr(Compiler *compiler, AST *ast, int size){
    if (ast->type == AST_INT){
        return strdup(ast->data.text.value);
    }else if (ast->type == AST_STRING){
        return strdup(ast->data.text.value);
    }else if (ast->type == AST_CHAR){
        return strdup(string_int_to_string(ast->data.text.value[0]));
    }else if(ast->type == AST_VAR){
        char string[100];
        if (ast->data.var.offset == -1){
            snprintf(string, 100, "rel _LBC%s", strdup(ast->data.text.value));
        }else {
            snprintf(string, 100, "rbp - %d", ast->data.var.offset);
        }
        return strdup(string);
    }else if(ast->type == AST_REG){
        AST_Reg reg = ast->data.operation.reg;
        return x86_64_change_reg_size(reg.reg, reg.size);
    }else if(ast->type == AST_GT){
        x86_64_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setg r15b");
        return "r15";
    }else if(ast->type == AST_GTE){
        x86_64_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setge r15b");
        return "r15";
    }else if(ast->type == AST_LT){
        x86_64_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setl r15b");
        return "r15";
    }else if(ast->type == AST_LTE){
        x86_64_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setle r15b");
        return "r15";
    }else if(ast->type == AST_ARRAY){
        char string[100];
        string[0] = '\0';
        AST *ast_current = ast->data.astarray.block;
        for (int i=0; i<ast->data.astarray.blocklen; i++){
            strcat(string, x86_64_eat_expr(compiler, ast_current, -1));
            if (i != ast->data.astarray.blocklen-1)
                strcat(string, ", ");
            ast_current = ast_current->next;
        }
        strcat(string, "\0");
        return strdup(string);
    }else if(ast->type == AST_EQ){
        x86_64_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "sete r15b");
        return "r15";
    }else if(ast->type == AST_NEQ){
        x86_64_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setne r15b");
        return "r15";
    }else if(ast->type == AST_REF){
        char *buf = x86_64_eat_expr(compiler, ast->data.expr.left, -1);
        char str[100];
        if (buf[0] == '['){
            snprintf(str, 100, "qword %s", buf);
        }else {
            snprintf(str, 100, "qword [%s]", buf);
        }
        compiler_write_text_line(compiler, "lea r12, %s", str);
        return "r12";
    }else if(ast->type == AST_CALL || ast->type == AST_CALLIF || ast->type == AST_CALLIFN){
        x86_64_eat_call(compiler, *ast);
        return "";
    }else if(ast->type == AST_DEREF){
        AST *left = ast->data.expr.left;
        char *buf = "";
        bool shortcut = 0;
        if (left->type == AST_PLUS) {
            if (is_immediate_value(left->data.expr.right)){
                shortcut = 1;
            }
        }
        if (!shortcut) {
            buf = x86_64_eat_expr(compiler, left, -1);
        }

        char str[100];
        char *name = x86_64_size_to_name(ast->typeinfo);
        if (shortcut){
            char *left1 = x86_64_eat_expr(compiler, left->data.expr.left, -1);
            char *right1 = x86_64_eat_expr(compiler, left->data.expr.right, -1);
            snprintf(str, 100, "%s [%s + %s]", name, left1, right1);
        }else {
            if (strchr(buf, '[')){
                x86_64_move(compiler, "r12", buf, ast->data.expr.left->typeinfo);
                snprintf(str, 100, "%s [r12]", name);
            }else if(x86_64_is_real_reg(buf) == 0){
                int reg = x86_64_reg_real_to_num(buf).reg;
                snprintf(str, 100, "%s [%s]", name, x86_64_change_reg_size(reg, 8));
            }else {
                snprintf(str, 100, "%s [%s]", name, buf);
            }
        }
        return strdup(str);
    }else if(ast->type == AST_PLUS){
        char *left = x86_64_eat_expr(compiler, ast->data.expr.left, -1);
        // compiler_write_text_line(compiler, "lea r12, [%s]", left);
        x86_64_move(compiler, "r12", left, ast->data.expr.left->typeinfo);
        compiler_write_text_line(compiler, "push r12");
        char *right = x86_64_eat_expr(compiler, ast->data.expr.right, -1);
        compiler_write_text_line(compiler, "pop r12");
        compiler_write_text_line(compiler, "add r12, %s", right);
        return "r12";
    };
    return "";
}

char *x86_64_eat_lhs(Compiler *compiler, AST ast, int size){
    AST *left = ast.data.opexpr.left;
    if (left->type == AST_VAR) {
        if (left->data.var.offset == -1){
            return left->data.text.value;
        }else {
            char string[100];
            snprintf(string, 100, "rbp - %d", left->data.var.offset);
            return strdup(string);
        }
    }else if(left->type == AST_REG){
        AST_Reg reg = left->data.operation.reg;
        return x86_64_change_reg_size(reg.reg, reg.size);
    }else if(left->type == AST_REF){
        char *buf = x86_64_eat_expr(compiler, left->data.expr.left, -1);
        compiler_write_text_line(compiler, "lea r12, [%s]", buf);
        return "r12";
    }else if(left->type == AST_DEREF){
        AST *leftleft = left->data.expr.left;
        char *buf = "";
        bool shortcut = 0;
        if (leftleft->type == AST_PLUS) {
            if (is_immediate_value(leftleft->data.expr.right)){
                shortcut = 1;
            }
        }
        if (!shortcut) {
            buf = x86_64_eat_expr(compiler, leftleft, -1);
        };
        char *name = x86_64_size_to_name(left->typeinfo);
        char string[100];
        if (shortcut){
            char *left1 = x86_64_eat_expr(compiler, leftleft->data.expr.left, -1);
            char *right1 = x86_64_eat_expr(compiler, leftleft->data.expr.right, -1);
            snprintf(string, 100, "%s [%s + %s]", name, left1, right1);
        }else {
            if (strchr(buf, '[')){
                x86_64_move(compiler, "r12", buf, leftleft->typeinfo);
                snprintf(string, 100, "%s [r12]", name);
            }else {
                snprintf(string, 100, "%s [%s]", name, buf);
            }
        }

        return strdup(string);
    }else {
        ;
    }
    return "";
};
void x86_64_end(Compiler *compiler){
        compiler_write_text_line(compiler, "mov rsp, rbp");
        compiler_write_text_line(compiler, "pop rbp");
        compiler_write_text_line(compiler, "ret");
};

void x86_64_eat_ast(Compiler *compiler, AST ast){
    if (ast.type == AST_MOV){
        AST *right = ast.data.opexpr.right;
        char *left = x86_64_eat_lhs(compiler, ast, ast.typeinfo);
        char *buf = x86_64_eat_expr(compiler, right, ast.typeinfo);
        int typeinfo = ast.typeinfo;
        x86_64_move(compiler, left, buf, ast.typeinfo);
    }else if(ast.type == AST_SYSCALL){
        x86_64_eat_syscall(compiler, ast);
    }else if(ast.type == AST_RET){
        x86_64_end(compiler);
    }else if(ast.type == AST_PUSH){
        char *buf = x86_64_eat_expr(compiler, ast.data.operation.right, -1);
        if (buf[0] == '['){
            compiler_write_text_line(compiler, "push qword %s", buf);
        }else {
            compiler_write_text_line(compiler, "push %s", buf);
        }
    }else if(ast.type == AST_POP){
        char *reg = x86_64_reg_to_name(ast.data.operation.reg);
        if (reg[0] == '['){
            compiler_write_text_line(compiler, "pop qword %s", reg);
        }else {
            compiler_write_text_line(compiler, "pop %s", reg);
        }
    }else if(ast.type == AST_CALL || ast.type == AST_CALLIF || ast.type == AST_CALLIFN){
        x86_64_eat_call(compiler, ast);
    }else if(ast.type == AST_ADD){
        char *reg = x86_64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = x86_64_reg_real_to_num(reg);
        char *buf = x86_64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(buf, '[')){
            x86_64_move(compiler, "r12", buf, _reg.size);
            compiler_write_text_line(compiler, "add %s, %s", reg, x86_64_change_reg_size(x86_64_reg_real_to_num("r12").reg, _reg.size));
        }else if (x86_64_is_real_reg(buf) == 0){
            AST_Reg _buf = x86_64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "add %s, %s", reg, x86_64_change_reg_size(_buf.reg, _reg.size));
        }else {
            compiler_write_text_line(compiler, "add %s, %s", reg, buf);
        }
    }else if(ast.type == AST_SUB){

        char *reg = x86_64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = ast.data.operation.reg;
        char *buf = x86_64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(buf, '[')){
            x86_64_move(compiler, "r11", buf, _reg.size);
            compiler_write_text_line(compiler, "sub %s, %s", reg, x86_64_change_reg_size(x86_64_reg_real_to_num("r11").reg, _reg.size));
        }else if (x86_64_is_real_reg(buf) == 0){
            AST_Reg _buf = x86_64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "sub %s, %s", reg, x86_64_change_reg_size(_buf.reg, _reg.size));
        }else {
            compiler_write_text_line(compiler, "sub %s, %s", reg, buf);
        }

    }else if(ast.type == AST_MUL){
        char *reg = x86_64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = ast.data.operation.reg;
        
        compiler_write_text_line(compiler, "push %s", x86_64_change_reg_size(_reg.reg, 8));
        
        char *buf = x86_64_eat_expr(compiler, ast.data.operation.right, -1);
        
        x86_64_move(compiler, "r11", buf, _reg.size);
        
        compiler_write_text_line(compiler, "pop rax");
        
        if (strchr(reg, '[')) {
            compiler_write_text_line(compiler, "imul eax, r11d");
            x86_64_move(compiler, reg, "eax", _reg.size);
        } else {
            compiler_write_text_line(compiler, "imul %s, %s", 
                x86_64_change_reg_size(_reg.reg, _reg.size), 
                x86_64_change_reg_size(x86_64_reg_real_to_num("r11").reg, _reg.size));
        }
    }else if (ast.type == AST_DIV) {
        char *buf = x86_64_eat_expr(compiler, ast.data.operation.right, -1);
        char *reg = x86_64_reg_to_name(ast.data.operation.reg);

        compiler_write_text_line(compiler, "push rax");
        compiler_write_text_line(compiler, "push rdx");

        x86_64_move(compiler, "rax", reg, ast.data.operation.reg.size);
        x86_64_move(compiler, "r12", buf, ast.data.operation.right->typeinfo);

        compiler_write_text_line(compiler, "cqo");
        compiler_write_text_line(compiler, "idiv r12");

        x86_64_move(compiler, reg, "rax", 8);

        compiler_write_text_line(compiler, "pop rdx");
        compiler_write_text_line(compiler, "pop rax");
    }else if(ast.type == AST_MOD){
        char *buf = x86_64_eat_expr(compiler, ast.data.operation.right, -1);
        char *reg = x86_64_reg_to_name(ast.data.operation.reg);

        compiler_write_text_line(compiler, "push rax");
        compiler_write_text_line(compiler, "push rdx");

        x86_64_move(compiler, "rax", reg, ast.data.operation.reg.size);
        x86_64_move(compiler, "r12", buf, ast.data.operation.right->typeinfo);

        compiler_write_text_line(compiler, "cqo");
        compiler_write_text_line(compiler, "idiv r12");

        x86_64_move(compiler, reg, "rdx", 8);

        compiler_write_text_line(compiler, "pop rdx");
        compiler_write_text_line(compiler, "pop rax");
    }else if(ast.type == AST_JMP){
        compiler_write_text_line(compiler, "jmp %s", ast.data.jmpif.label);
    }else if(ast.type == AST_JMPIF){
        char *reg = x86_64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        if (strchr(reg, '[')){
            x86_64_move(compiler, "r12", reg, 8);
            char *r12=x86_64_change_reg_size(x86_64_reg_real_to_num("r12").reg, ast.data.jmpif.reg->typeinfo);compiler_write_text_line(compiler, "test %s, %s", r12, r12);
        }else {
            compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        };
        compiler_write_text_line(compiler, "jz %s", ast.data.jmpif.label);
    }else if(ast.type == AST_JMPIFN){
        char *reg = x86_64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        if (strchr(reg, '[')){
            x86_64_move(compiler, "r12", reg, 8);
            char *r12=x86_64_change_reg_size(x86_64_reg_real_to_num("r12").reg, ast.data.jmpif.reg->typeinfo);compiler_write_text_line(compiler, "test %s, %s", r12, r12);
        }else {
            compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        };
        compiler_write_text_line(compiler, "jnz %s", ast.data.jmpif.label);
    }else if(ast.type == AST_LABEL){
        compiler_write_text(compiler, ".%s:\n", ast.data.text.value);
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            x86_64_eat_ast(compiler, *current);
            current = current->next;
        }
        compiler_write_text(compiler, ".%s_end:\n", ast.data.text.value);
    };
};

char *x86_64_typeinfo_to_specifier(AST_TypeInfo type){
    if (type == 1){
        return "db";
    }else if (type == 2){
        return "dw";
    }else if (type == 8){
        return "dq";
    }else if (type == 4){
        return "dd";
    }

    return "";
}

int x86_64_function_num = 0;


void x86_64_eat_body(Compiler *compiler){
    AST ast = *(load_ast_compiler(compiler));
    if (ast.type == AST_FUNCDEF){
        char string[100];
        snprintf(string, 100, "_fun%s", ast.data.funcdef.name);
        char *name = strdup(string);
        if (string_compare(name, "_funmain", strlen(name)) == 0) {
            name = "main";
        }
        compiler->templabel = 0;
        compiler_write_text(compiler, "%s:\n", name);
        compiler_write_text_line(compiler, "push rbp");
        compiler_write_text_line(compiler, "mov rbp, rsp");
        int size = 152;
        Reviser_Function *function = reviser_get_function(compiler->global->functions, x86_64_function_num);
        for (int i=0; i<function->scope.variablelen; i++){
            size += reviser_get_variable(function->scope.variables, i)->size;
        };
        if (size > 0) {
            compiler_write_text_line(compiler, "sub rsp, %d", (size + 15) & ~((size_t)15));
        }
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            x86_64_eat_ast(compiler, *current);
            current = current->next;
        }

        x86_64_function_num++;
        x86_64_end(compiler);
    }else if(ast.type == AST_LABEL){
        compiler_write_text(compiler, "%s:\n", ast.data.text.value);
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            x86_64_eat_ast(compiler, *current);
            current = current->next;
        }
    }else if (ast.type == AST_VAR){
        AST *data = ast.data.var.value;
        compiler_write(compiler->data, "\talign 8\n\t_LBC%s: %s %s\n", ast.data.var.name, x86_64_typeinfo_to_specifier(ast.typeinfo), x86_64_format(x86_64_eat_expr(compiler, data, -1), data->type));
    }
    compiler_peek(compiler);
}

char x86_64_eat(Compiler *compiler){
    if(compiler->cur >= compiler->astlen)
        return -1;
    x86_64_eat_body(compiler);
    return 0;
}
