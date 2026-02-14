AST *load_ast_compiler(Compiler *compiler){
    if (compiler->cur == 0){
        return compiler->asts;
    }else {
        AST *ast1 = compiler->asts;
        for (int i=0; i<compiler->cur; i++){
            ast1 = ast1->next;
        };
        return ast1;
    };
}



char *regs64[] = { "rax", "rbx", "r10", "rdx", "rsi", "rdi", "r8", "r9", "r15", "r12", "rcx", "r11", "r13", "r14" };

char *regs32[] = { "eax", "ebx", "r10d", "edx", "esi", "edi", "r8d", "r9d", "r15d", "r12d", "ecx", "r11d", "r13d", "r14d" };

char *regs16[] = { "ax", "bx", "r10w", "dx", "si", "di", "r8w", "r9w", "r15w", "r12w", "cx", "r11w", "r13w", "r14w" };

char *regs8[] = { "al", "bl", "r10b", "dl", "sil", "dil", "r8b", "r9b", "r15b", "r12b", "cl", "r11b", "r13b", "r14b" };

char *change_reg_size(int reg, int size){
    switch (size){
        case 1: return regs8[reg];
        case 2: return regs16[reg];
        case 4: return regs32[reg];
        case 8: return regs64[reg];
    };
    return regs64[reg];
}
char *reg_to_name(AST_Reg reg){
    switch (reg.size){
        case 1: return regs8[reg.reg];
        case 2: return regs16[reg.reg];
        case 4: return regs32[reg.reg];
        case 8: return regs64[reg.reg];
        case 0: return regs64[reg.reg];
    };
    char string[100];
    snprintf(string, 100, "Register doesn't exist");
    error_generate("InvalidRegisterError", string);
    return "";
};
AST_Reg reg_real_to_num(char *reg){
    AST_Reg reg1 = (AST_Reg){0};
    reg1.reg = -1;
    for (int i=0; i<14; i++){
        if (string_compare(regs8[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 1;
        };
    };
    for (int i=0; i<14; i++){
        if (string_compare(regs16[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 2;
        };
    };
    for (int i=0; i<14; i++){
        if (string_compare(regs32[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 4;
        };
    };
    for (int i=0; i<14; i++){
        if (string_compare(regs64[i], reg, strlen(reg)) == 0){
            reg1.reg = i;
            reg1.size = 8;
        };
    };
    return reg1;
}

char* find_available_tmp_file(char *ext) {
    static char filename[256];
    int i = 0;

    while (true) {
        snprintf(filename, sizeof(filename), "tmp_%d.%s", i, ext);

        if (access(filename, F_OK) != 0) {
            return strdup(filename);
        }

        i++;
        
        if (i > 1000) return NULL; 
    }
}

Compiler *compiler_init(Reviser *reviser, char *file){
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

void compiler_close(Compiler *compiler){
    fprintf(compiler->f, "section .data\n%s\n", compiler->data);
    fclose(compiler->f);
    char string[100];
    compiler->_o = find_available_tmp_file("o");
    snprintf(string, 100, "bat %s", compiler->_asm);
    system(string);
    snprintf(string, 100, "cat %s | pbcopy", compiler->_asm);
    system(string);
    snprintf(string, 100, "yasm -f macho64 %s -o %s", compiler->_asm, compiler->_o);
    system(string);
    remove(compiler->_asm);
    snprintf(string, 100, "clang -arch x86_64 %s -o %s -e main -Wl,-w -Wl,-platform_version,macos,11.0,11.0", compiler->_o, compiler->name);
    system(string);
    remove(compiler->_o);
}

void compiler_peek(Compiler *compiler){
    compiler->cur++;
}

void compiler_write_text(Compiler *compiler, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    fprintf(compiler->f, "%s", add);
};

void compiler_write_text_line(Compiler *compiler, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    fprintf(compiler->f, "\t%s\n", add);
};

void compiler_write(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    if (strlen(buf) + n >= 4096) {
        n = 4096 - strlen(buf) - 1;
        if (n <= 0) return;
        add[n] = '\0';
    }

    strcat(buf, add);
}

void compiler_write_line(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    if (strlen(buf) + n >= 4096) {
        n = 4096 - strlen(buf) - 1;
        if (n <= 0) return;
        add[n] = '\0';
    }

    strcat(buf, "\t");
    strcat(buf, add);
    strcat(buf, "\n");
}

char *format(char *str, int type){
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

char *compiler_eat_expr(Compiler *compiler, AST *ast, int size);



char *move(Compiler *compiler, char *reg, char *buf, int typeinfo){
    if ((reg[0] == 'w' || reg[0] == 'd' || reg[0] == 'b' || reg[0] == 'q') && (buf[0] == 'w' || buf[0] == 'd' || buf[0] == 'b' || buf[0] == 'q')){
        char *right = move(compiler, "r14", buf, typeinfo);
        AST_Reg _reg = reg_real_to_num("r14");
        char *s = change_reg_size(_reg.reg, typeinfo);
        return move(compiler, reg, s, typeinfo);
    }
    
    char *op = "mov";
    char *final_reg = reg;
    char *final_buf = buf;
    
    AST_Reg _reg = reg_real_to_num(reg);
    if (_reg.reg != -1) {
        final_reg = change_reg_size(_reg.reg, typeinfo);
    } else {
        final_reg = reg; 
    }
    
    AST_Reg _buf = reg_real_to_num(buf);
    if (_buf.reg != -1) {
        final_buf = change_reg_size(_buf.reg, typeinfo);
    } else {
        final_buf = buf;
    }
    compiler_write_text_line(compiler, "%s %s, %s", op, final_reg, final_buf);
    return final_reg;
    
}

void compiler_compare(Compiler *compiler, AST *ast){
        char *left = compiler_eat_expr(compiler, ast->data.expr.left, -1);
        char *right = compiler_eat_expr(compiler, ast->data.expr.right, -1);
        // compiler_write_text_line(compiler, "mov r12, %s", left);
        char *reg = move(compiler, "r12", left, ast->data.expr.left->typeinfo);
        compiler_write_text_line(compiler, "cmp %s, %s", reg, right);
}

bool is_immediate_value(Compiler *compiler, AST *ast){
    if (ast->type == AST_INT || ast->type == AST_CHAR){
        return 1;
    }else {
        return 0;
    };
}


char *compiler_eat_expr(Compiler *compiler, AST *ast, int size){
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
        if (is_reg(ast->data.text.value) == 0){
            AST_Reg reg = _reg_to_num(ast->data.text.value, ast->typeinfo);
            return change_reg_size(reg.reg, reg.size);
        }
    }else if(ast->type == AST_GT){
        compiler_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setg r15b");
        return "r15";
    }else if(ast->type == AST_GTE){
        compiler_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setge r15b");
        return "r15";
    }else if(ast->type == AST_LT){
        compiler_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setl r15b");
        return "r15";
    }else if(ast->type == AST_LTE){
        compiler_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setle r15b");
        return "r15";
    }else if(ast->type == AST_ARRAY){
        char string[100];
        string[0] = '\0';
        AST *ast_current = ast->data.astarray.block;
        for (int i=0; i<ast->data.astarray.blocklen; i++){
            strcat(string, compiler_eat_expr(compiler, ast_current, -1));
            if (i != ast->data.astarray.blocklen-1)
                strcat(string, ", ");
            ast_current = ast_current->next;
        }
        strcat(string, "\0");
        return strdup(string);
    }else if(ast->type == AST_EQ){
        compiler_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "sete r15b");
        return "r15";
    }else if(ast->type == AST_NEQ){
        compiler_compare(compiler, ast);
        compiler_write_text_line(compiler, "mov r15, 0");
        compiler_write_text_line(compiler, "setne r15b");
        return "r15";
    }else if(ast->type == AST_REF){
        char *buf = compiler_eat_expr(compiler, ast->data.expr.left, -1);
        char str[100];
        if (buf[0] == '['){
            snprintf(str, 100, "qword %s", buf);
        }else {
            snprintf(str, 100, "qword [%s]", buf);
        }
        compiler_write_text_line(compiler, "lea r12, %s", str);
        return "r12";
    }else if(ast->type == AST_DEREF){
        AST *left = ast->data.expr.left;
        char *buf = "";
        bool shortcut = 0;
        if (left->type == AST_PLUS) {
            if (is_immediate_value(compiler, left->data.expr.right)){
                shortcut = 1;
            }
        }
        if (!shortcut) {
            buf = compiler_eat_expr(compiler, left, -1);
        }

        char str[100];
        char *name = size_to_name(ast->typeinfo);
        if (shortcut){
            char *left1 = compiler_eat_expr(compiler, left->data.expr.left, -1);
            char *right1 = compiler_eat_expr(compiler, left->data.expr.right, -1);
            snprintf(str, 100, "%s [%s + %s]", name, left1, right1);
        }else {
            // if (buf[0] == '['){
            //     snprintf(str, 100, "%s %s", name, buf);
            // }else {
            snprintf(str, 100, "%s [%s]", name, buf);
            // }
        }
        return strdup(str);
    }else if(ast->type == AST_PLUS){
        char *left = compiler_eat_expr(compiler, ast->data.expr.left, -1);
        // compiler_write_text_line(compiler, "lea r12, [%s]", left);
        compiler_write_text_line(compiler, "mov r12, %s", left);
        compiler_write_text_line(compiler, "push r12");
        char *right = compiler_eat_expr(compiler, ast->data.expr.right, -1);
        compiler_write_text_line(compiler, "pop r12");
        compiler_write_text_line(compiler, "add r12, %s", right);
        return "r12";
    };
    return "";
}

char *compiler_eat_lhs(Compiler *compiler, AST ast, int size){
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
        if (is_reg(left->data.text.value) == 0){
            AST_Reg reg = _reg_to_num(left->data.text.value, left->typeinfo);
            return change_reg_size(reg.reg, reg.size);
        }
    }else if(left->type == AST_REF){
        char *buf = compiler_eat_expr(compiler, left->data.expr.left, -1);
        compiler_write_text_line(compiler, "lea r12, [%s]", buf);
        return "r12";
    }else if(left->type == AST_DEREF){
        AST *leftleft = left->data.expr.left;
        char *buf = "";
        bool shortcut = 0;
        if (leftleft->type == AST_PLUS) {
            if (is_immediate_value(compiler, leftleft->data.expr.right)){
                shortcut = 1;
            }
        }
        if (!shortcut) {
            buf = compiler_eat_expr(compiler, leftleft, -1);
        };
        char *name = size_to_name(left->typeinfo);
        char string[100];
        if (shortcut){
            char *left1 = compiler_eat_expr(compiler, leftleft->data.expr.left, -1);
            char *right1 = compiler_eat_expr(compiler, leftleft->data.expr.right, -1);
            snprintf(string, 100, "%s [%s + %s]", name, left1, right1);
        }else {
            if (buf[0] == '['){
                snprintf(string, 100, "%s %s", name, buf);
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
void compile_end(Compiler *compiler){
        compiler_write_text_line(compiler, "mov rsp, rbp");
        compiler_write_text_line(compiler, "pop rbp");
        compiler_write_text_line(compiler, "ret");
};

void compiler_eat_ast(Compiler *compiler, AST ast){
    if (ast.type == AST_MOV){
        AST *right = ast.data.opexpr.right;
        char *left = compiler_eat_lhs(compiler, ast, ast.typeinfo);
        char *buf = compiler_eat_expr(compiler, right, ast.typeinfo);
        int typeinfo = ast.typeinfo;
        move(compiler, left, buf, right->typeinfo);
    }else if(ast.type == AST_SYSCALL){
        compiler_write_text_line(compiler, "mov rax, %s", ast.data.text.value);
        compiler_write_text_line(compiler, "syscall");
    }else if(ast.type == AST_RET){
        compile_end(compiler);
    }else if(ast.type == AST_PUSH){
        char *buf = compiler_eat_expr(compiler, ast.data.operation.right, -1);
        compiler_write_text_line(compiler, "push %s", buf);
    }else if(ast.type == AST_POP){
        compiler_write_text_line(compiler, "pop %s", reg_to_name(ast.data.operation.reg));
    }else if(ast.type == AST_CALL){
        char string[100];
        snprintf(string, 100, "%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "call %s", strdup(string));
    }else if(ast.type == AST_CALLIF){
        char string[100];
        snprintf(string, 100, ".L%d", compiler->templabel++);
        char *reg = compiler_eat_expr(compiler, ast.data.jmpif.reg, -1);
        compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        compiler_write_text_line(compiler, "jz %s", strdup(string));
        char string1[100];
        snprintf(string1, 100, "%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "call %s", strdup(string1));
        compiler_write_text(compiler, "%s:\n", strdup(string));
    }else if(ast.type == AST_CALLIFN){
        char string[100];
        snprintf(string, 100, ".L%d", compiler->templabel++);
        char *reg = compiler_eat_expr(compiler, ast.data.jmpif.reg, -1);
        compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        compiler_write_text_line(compiler, "jnz %s", strdup(string));
        char string1[100];
        snprintf(string1, 100, "%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "call %s", strdup(string1));
        compiler_write_text(compiler, "%s:\n", strdup(string));
    }else if(ast.type == AST_ADD){
        char *buf = compiler_eat_expr(compiler, ast.data.operation.right, -1);
        char *reg = reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = reg_real_to_num(reg);
        AST_Reg _buf = reg_real_to_num(buf);
        if (_buf.size != _reg.size){
            compiler_write_text_line(compiler, "mov %s, %s", buf, buf);
        };
        compiler_write_text_line(compiler, "add %s, %s", reg, change_reg_size(_buf.reg, _reg.size));
    }else if(ast.type == AST_SUB){
        char *buf = compiler_eat_expr(compiler, ast.data.operation.right, -1);
        char *reg = reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = reg_real_to_num(reg);
        AST_Reg _buf = reg_real_to_num(buf);
        compiler_write_text_line(compiler, "sub %s, %s", reg, change_reg_size(_buf.reg, _reg.size));
    }else if(ast.type == AST_MUL){
        char *buf = compiler_eat_expr(compiler, ast.data.operation.right, -1);
        compiler_write_text_line(compiler, "imul %s, %s", reg_to_name(ast.data.operation.reg), buf);
    }else if(ast.type == AST_DIV){
        char *reg = reg_to_name(ast.data.operation.reg);
        compiler_write_text_line(compiler, "push rax");
        compiler_write_text_line(compiler, "push rdx");
        move(compiler, "rax", reg, ast.data.operation.reg.size);
        compiler_write_text_line(compiler, "cqo");
        char *buf = compiler_eat_expr(compiler, ast.data.operation.right, -1);
        if (is_immediate_value(compiler, ast.data.operation.right)) {
            compiler_write_text_line(compiler, "mov %s, %s", reg, buf);
            compiler_write_text_line(compiler, "idiv %s", reg);
        }else {
            compiler_write_text_line(compiler, "idiv %s", buf);
        }
        move(compiler, reg, "rax", 8);
        compiler_write_text_line(compiler, "pop rdx");
        compiler_write_text_line(compiler, "pop rax");
    }else if(ast.type == AST_MOD){
        char *reg = reg_to_name(ast.data.operation.reg);
        compiler_write_text_line(compiler, "push rax");
        compiler_write_text_line(compiler, "push rdx");
        move(compiler, "rax", reg, ast.data.operation.reg.size);
        compiler_write_text_line(compiler, "cqo");
        char *buf = compiler_eat_expr(compiler, ast.data.operation.right, -1);
        if (is_immediate_value(compiler, ast.data.operation.right)) {
            compiler_write_text_line(compiler, "mov %s, %s", reg, buf);
            compiler_write_text_line(compiler, "idiv %s", reg);
        }else {
            compiler_write_text_line(compiler, "idiv %s", buf);
        }
        move(compiler, reg, "rdx", 8);
        compiler_write_text_line(compiler, "pop rdx");
        compiler_write_text_line(compiler, "pop rax");
    }else if(ast.type == AST_JMP){
        compiler_write_text_line(compiler, "jmp %s", ast.data.jmpif.label);
    }else if(ast.type == AST_JMPIF){
        char *reg = compiler_eat_expr(compiler, ast.data.jmpif.reg, -1);
        compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        compiler_write_text_line(compiler, "jz %s", ast.data.jmpif.label);
    }else if(ast.type == AST_JMPIFN){
        char *reg = compiler_eat_expr(compiler, ast.data.jmpif.reg, -1);
        compiler_write_text_line(compiler, "test %s, %s", reg, reg);
        compiler_write_text_line(compiler, "jnz %s", ast.data.jmpif.label);
    }else if(ast.type == AST_LABEL){
        compiler_write_text(compiler, ".%s:\n", ast.data.text.value);
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            compiler_eat_ast(compiler, *current);
            current = current->next;
        }
        compiler_write_text(compiler, ".%s_end:\n", ast.data.text.value);
    };
};

char *typeinfo_to_specifier(AST_TypeInfo type){
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

int funN = 0;


void compiler_eat_body(Compiler *compiler){
    AST ast = *(load_ast_compiler(compiler));
    if (ast.type == AST_FUNCDEF){
        char string[100];
        snprintf(string, 100, "%s", ast.data.funcdef.name);
        char *name = strdup(string);
        if (string_compare(name, "_funmain", strlen(name)) == 0) {
            name = "main";
        }
        compiler->templabel = 0;
        compiler_write_text(compiler, "%s:\n", name);
        compiler_write_text_line(compiler, "push rbp");
        compiler_write_text_line(compiler, "mov rbp, rsp");
        int size = 0;
        Reviser_Function *function = reviser_get_function(compiler->global->functions, funN);
        for (int i=0; i<function->scope.variablelen; i++){
            size += reviser_get_variable(function->scope.variables, i)->size;
        };
        if (size > 0) {
            compiler_write_text_line(compiler, "sub rsp, %d", (size + 15) & ~((size_t)15));
        }
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            compiler_eat_ast(compiler, *current);
            current = current->next;
        }

        funN++;
        compile_end(compiler);
    }else if(ast.type == AST_LABEL){
        compiler_write_text(compiler, "%s:\n", ast.data.text.value);
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            compiler_eat_ast(compiler, *current);
            current = current->next;
        }
    }else if (ast.type == AST_VAR){
        AST *data = ast.data.var.value;
        compiler_write(compiler->data, "\talign 8\n\t_LBC%s: %s %s\n", ast.data.var.name, typeinfo_to_specifier(ast.typeinfo), format(compiler_eat_expr(compiler, data, -1), data->type));
    }else if(ast.type == AST_EXTERN){
        compiler_write(compiler->data, "extrn %s\n", ast.data.text.value);
    }
    compiler_peek(compiler);
}

char compiler_eat(Compiler *compiler){
    if(compiler->cur >= compiler->astlen)
        return -1;
    compiler_eat_body(compiler);
    return 0;
}
