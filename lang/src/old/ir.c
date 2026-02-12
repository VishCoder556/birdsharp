char *IrData = "";
int tempN = 0;
int regN = 0;

// Register allocation system
typedef struct {
    char name[10];
    int size;
    bool inUse;
    int refCount;
} IRReg;

IRReg registers[16];

void init_registers() {
    for (int i = 0; i < 16; i++) {
        snprintf(registers[i].name, 10, "t%d", i);
        registers[i].size = 8;  // Default to 8 bytes
        registers[i].inUse = false;
        registers[i].refCount = 0;
    }
}

char* alloc_register(int size) {
    for (int i = 0; i < 16; i++) {
        if (!registers[i].inUse) {
            registers[i].inUse = true;
            registers[i].size = size;
            registers[i].refCount = 1;
            return registers[i].name;
        }
    }
    // Fallback - reuse oldest register
    int oldest = 0;
    for (int i = 1; i < 16; i++) {
        if (registers[i].refCount < registers[oldest].refCount) {
            oldest = i;
        }
    }
    registers[oldest].size = size;
    registers[oldest].refCount = 1;
    return registers[oldest].name;
}

void free_register(char* regname) {
    for (int i = 0; i < 16; i++) {
        if (strcmp(registers[i].name, regname) == 0) {
            registers[i].inUse = false;
            registers[i].refCount = 0;
            break;
        }
    }
}

char* get_sized_reg(char* regname, int size) {
    static char sized_reg[20];
    switch (size) {
        case 1: snprintf(sized_reg, 20, "%sb", regname); break;
        case 2: snprintf(sized_reg, 20, "%sw", regname); break;
        case 4: snprintf(sized_reg, 20, "%sd", regname); break;
        default: snprintf(sized_reg, 20, "%s", regname); break;
    }
    return sized_reg;
}

void ir_init(void *generator){
    IrData = malloc(5000);
    strncpy(IrData, "", 100);
    init_registers();
    Generator *gen = (Generator*)generator;
    strcat(gen->output->filename, ".ir");
    generator_open_text(generator);
};

bool is_imm_or_reg(AST *ast){
    if (ast->type == AST_INT || ast->type == AST_CHAR){
        return 1;
    }else {
        return 0;
    };
}

char *ir_generate_expr(void *generator, AST ast);

bool is_r(char *reg){
    if (strstr(reg, ">") || strstr(reg, "<") || strstr(reg, "==") || strstr(reg, "!=")) {
        return false;
    }
    char last = reg[strlen(reg)-1];
    if (strncmp(reg, "dword", 5) == 0 && last == ']'){
        return true;
    }else if (strncmp(reg, "word", 4) == 0 && last == ']'){
        return true;
    }else if (strncmp(reg, "byte", 4) == 0 && last == ']'){
        return true;
    }else if (strncmp(reg, "qword", 5) == 0 && last == ']'){
        return true;
    }else if((reg[0] == 'a' || reg[0] == 't' || reg[0] == 's' || reg[0] == 'v') && strlen(reg) == 2){
        return true;
    }
    return false;
};

char *r_based_on_size(char *reg, int type){
    int len = strlen(reg);
    bool access = false;
    if (strncmp(reg, "dword", 5) == 0){
        access = true;
        reg += 5;
    }else if (strncmp(reg, "word", 4) == 0){
        access = true;
        reg += 4;
    }else if (strncmp(reg, "byte", 4) == 0){
        access = true;
        reg += 4;
    }else if (strncmp(reg, "qword", 5) == 0){
        access = true;
        reg += 5;
    }else if((reg[0] == 'a' || reg[0] == 't' || reg[0] == 's' || reg[0] == 'v') && strlen(reg) == 2){
        ;
    }else {
        return reg;
    }
    char out[20];
    int i = 0;
    int j = 0;
    int len1 = 0;
    if (access == true){
        char *access = len_to_selector(type);
        for (; i<strlen(access); i++){
            out[i] = access[i];
        };
        len1 = i;
    }
    for (; i<len + len1; i++){
        out[i] = reg[j];
        j++;
    };
    if (access == true){
        out[strlen(out)] = '\0';
        return strdup(out);
    }
    len--;
    if (out[len] != 'b' && out[len] != 'w' && out[len] != 'd'){
        len++;
    };
    if (type == 8){
        out[len] = '\0';
    }else if (type == 4){
        out[len] = 'd';
    }else if (type == 2){
        out[len] = 'w';
    }else if (type == 1){
        out[len] = 'b';
    };
    out[len+1] = '\0';
    return strdup(out);
}
char *move_imm(void *generator, char *str, char *reg, AST_TypeInfo typeinfo){
    if (strcmp(str, reg) == 0) {
        return "";
    }
    generator_write_text(generator, "\t");
    int type = typeinfo_to_len(typeinfo);
    // generator_write_text(generator, );
    char *newreg = r_based_on_size(reg, type);
    generator_write_text(generator, newreg);
    generator_write_text(generator, " = ");
    if (is_r(str)){
        str = r_based_on_size(strdup(str), type);
    }
    generator_write_text(generator, str);
    generator_write_text(generator, "\n");
    return strdup(newreg);
};

char *move(void *generator, AST ast, char *reg){
    return move_imm(generator, ir_generate_expr(generator, ast), reg, ast.typeinfo);
};


char *ir_generate_expr(void *generator, AST ast){
    if (ast.type == AST_PLUS){
        move(generator, *(ast.data.expr.left), "t0");
        move(generator, *(ast.data.expr.right), "t1");
        generator_write_text(generator, "\tadd t0, t1\n");
        return "t0";
    }else if (ast.type == AST_SUB){
        move(generator, *(ast.data.expr.left), "t0");
        move(generator, *(ast.data.expr.right), "t1");
        generator_write_text(generator, "\tsub t0, t1\n");
        return "t0";
    }else if (ast.type == AST_MUL){
        move(generator, *(ast.data.expr.left), "t0");
        move(generator, *(ast.data.expr.right), "t1");
        generator_write_text(generator, "\tmul t0, t1\n");
        return "t0";
    }else if (ast.type == AST_DIV){
        move(generator, *(ast.data.expr.left), "t0");
        move(generator, *(ast.data.expr.right), "t1");
        generator_write_text(generator, "\tdiv t0, t1\n");
        return "t0";
    }else if(ast.type == AST_INT){
        return ast.data.arg.value;
    }else if(ast.type == AST_CHAR){
        char string[100];
        snprintf(string, 100, "%d", ast.data.arg.value[0]);
        return strdup(string);
    }else if (ast.type == AST_STRING){
        generator_write_text(generator, "\tt0 = &temp_");
        char string[100];
        snprintf(string, 100, "%d", tempN++);
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, "\n");
        strcat(IrData, "%byte temp_");
        strcat(IrData, strdup(string));
        strcat(IrData, " = \"");
        for (int i=0; i<strlen(ast.data.arg.value); i++){
            if (ast.data.arg.value[i] == '\n'){
                strcat(IrData, "\\n");
            }else {
                char str[2];
                str[0] = ast.data.arg.value[i];
                str[1] = '\0';
                strcat(IrData, strdup(str));
            };
        }
        strcat(IrData, "\"\n");
        return "t0";
    }else if(ast.type == AST_FUNCALL){
        for (int i=ast.data.funcall.argslen; i > 0; i--){
            char string[100];
            snprintf(string, 100, "a%d", i-1);
            move(generator, *ast.data.funcall.args[i-1], strdup(string));
        };
        generator_write_text(generator, "\tcall ");
        generator_write_text(generator, ast.data.funcall.name);
        generator_write_text(generator, "\n");
        return "v0";
    }else if(ast.type == AST_CAST){
        char *inner = ir_generate_expr(generator, *ast.data.expr.left);
        // Generate a move that respects the cast target type
        char string[100];
        snprintf(string, 100, "%s", r_based_on_size(inner, typeinfo_to_len(ast.typeinfo)));
        return strdup(string);
    }else if(ast.type == AST_EXPR){
        return ir_generate_expr(generator, *ast.data.expr.left);
    }else if (ast.type == AST_VAR){
        char string[100];
        snprintf(string, 100, "%s [%s]", len_to_selector(typeinfo_to_len(ast.typeinfo)), ast.data.arg.value);
        return strdup(string);
    }else if(ast.type == AST_SYSCALL){
        for (int i=ast.data.funcall.argslen; i > 0; i--){
            char string[100];
            snprintf(string, 100, "a%d", i-1);
            move(generator, *ast.data.funcall.args[i-1], strdup(string));
        };
        int syscallLen = strlen("syscall");
        if (strncmp(ast.data.funcall.name, "syscall", syscallLen) == 0){
            ast.data.funcall.name += syscallLen + 1;
        }
        generator_write_text(generator, "\tv0 = syscall.");
        generator_write_text(generator, ast.data.funcall.name);
        generator_write_text(generator, "\n\tsyscall\n");
        return "v0";
    }else if(ast.type == AST_NEQ){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s != %s", lhs, rhs);
        return strdup(string);
    }else if(ast.type == AST_EQ){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s == %s", lhs, rhs);
        return strdup(string);
    }else if(ast.type == AST_GT){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s < %s", lhs, rhs);
        return strdup(string);
    }else if(ast.type == AST_GTE){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s <= %s", lhs, rhs);
        return strdup(string);
    }else if(ast.type == AST_LT){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s > %s", lhs, rhs);
        return strdup(string);
    }else if(ast.type == AST_LTE){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s >= %s", lhs, rhs);
        return strdup(string);
    }else if(ast.type == AST_MODULO){
        move(generator, *ast.data.expr.left, "t0");
        move(generator, *ast.data.expr.right, "t1");
        generator_write_text(generator, "\tmod t0, t1\n");
        return "t0";
    }else if(ast.type == AST_INDEX){
        int len = typeinfo_to_len(ast.typeinfo);
        move(generator, *ast.data.expr.left, "t0");
        move(generator, *ast.data.expr.right, "t1");
        generator_write_text(generator, "\tadd t0, t1\n");
        char string[100];
        snprintf(string, 100, "%s [t0]", len_to_selector(len));
        return strdup(string);
    }else if(ast.type == AST_DEREF){
        char string[100];
        move(generator, *ast.data.expr.left, "s0");
        snprintf(string, 100, "%s [s0]", len_to_selector(typeinfo_to_len(ast.typeinfo)));
        move_imm(generator, strdup(string), "t2", ast.typeinfo);
        return "t2";
    }else {
        char string[100];
        snprintf(string, 100, "Unknown type found: '%d'", ast.type);
        error_generate_parser("TypeError", string, ast.row, ast.col, ((Generator*)generator)->name);
    };
    return "";
};

char *ir_generate_lhs(void *generator, AST ast){
    if (ast.type == AST_VAR){
        return ast.data.arg.value;
    }else if(ast.type == AST_CAST){
        return ir_generate_lhs(generator, *ast.data.expr.left);
    }else if(ast.type == AST_DEREF){
        move(generator, *ast.data.expr.left, "s1");
        return "s1";
    }else if(ast.type == AST_INDEX){
        int len = typeinfo_to_len(ast.typeinfo);
        move(generator, *ast.data.expr.left, "t0");
        move(generator, *ast.data.expr.right, "t1");
        generator_write_text(generator, "\tadd t0, t1\n");
        generator_write_text(generator, "\tmov s1, t0\n");
        char string[100];
        snprintf(string, 100, "s1");
        return strdup(string);
    };
    return "";
}

int lblN = 0;

void ir_generate_stmnt(void *generator, AST ast);
void ir_generate_stmnt(void *generator, AST ast){
    if (ast.type == AST_RET){
        move(generator, *ast.data.ret.ret, "v0");
        generator_write_text(generator, "\tret\n");
    }else if (ast.type == AST_ASSIGN){
        char string[100];
        int typeinfo = typeinfo_to_len(ast.typeinfo);
        char *var = ir_generate_lhs(generator, *ast.data.assign.from);
        if (ast.data.assign.from->type == AST_VAR && ast.data.assign.new == true) {
            generator_write_text(generator, "\t%local ");
            generator_write_text(generator, var);
            generator_write_text(generator, ", ");
            snprintf(string, 100, "%d", typeinfo);
            generator_write_text(generator, strdup(string));
            generator_write_text(generator, "\n");
        }
        
        if (ast.data.assign.from->type == AST_INDEX || ast.data.assign.from->type == AST_DEREF) {
            // Handle array indexing and pointer dereferencing assignments
            char *rhs = ir_generate_expr(generator, *(ast.data.assign.assignto));
            generator_write_text(generator, "\t");
            generator_write_text(generator, len_to_selector(typeinfo));
            generator_write_text(generator, " [");
            generator_write_text(generator, var);
            generator_write_text(generator, "] = ");
            generator_write_text(generator, rhs);
            generator_write_text(generator, "\n");
        } else {
            snprintf(string, 100, "%s [%s]", len_to_selector(typeinfo), var);
            move(generator, *(ast.data.assign.assignto), strdup(string));
        }
    }else if(ast.type == AST_IF){
        char string[100];
        snprintf(string, 100, "_LBC%d", lblN++);
        char *reg = move(generator, *ast.data.if1.block.condition, "t2");
        generator_write_text(generator, "\tjump labelend ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, " ifnot ");
        generator_write_text(generator, reg);
        generator_write_text(generator, "\n%label ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, " {\n");
        for (int i=0; i<ast.data.if1.block.statementlen; i++){
            ir_generate_stmnt(generator, *(ast.data.if1.block.statements[i]));
        };
        generator_write_text(generator, "}\n");
    }else if(ast.type == AST_WHILE){
        char string[100];
        snprintf(string, 100, "_LBC%d", lblN++);
        generator_write_text(generator, "%label ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, " {\n");
        char *reg = move(generator, *ast.data.while1.condition, "t2");
        generator_write_text(generator, "\tjump labelend ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, " ifnot ");
        generator_write_text(generator, reg);
        generator_write_text(generator, "\n");
        for (int i=0; i<ast.data.while1.blocklen; i++){
            ir_generate_stmnt(generator, *(ast.data.while1.block[i]));
        };
        generator_write_text(generator, "\tjump label ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, "\n");
        generator_write_text(generator, "}\n");
    }else {
        move(generator, ast, "v0");
    };
};

void ir_generate_ast(void *generator, AST ast){
    if (ast.type == AST_FUNCDEF){
        lblN = 0;
        generator_write_text(generator, "%func ");
        generator_write_text(generator, ast.data.funcdef.name);
        generator_write_text(generator, " {\n");
        for (int i=0; i<ast.data.funcdef.argslen; i++){
            Argument arg = *ast.data.funcdef.args[i];
            int typeinfo = typeinfo_to_len(arg.type);
            char argS[100];
            snprintf(argS, 100, "a%d", i);
            char string[100];
            snprintf(string, 100, "\t%%local %s, %d\n\t%s [%s] = %s\n", arg.arg, typeinfo, len_to_selector(typeinfo), arg.arg, r_based_on_size(strdup(argS), typeinfo));
            generator_write_text(generator, strdup(string));
        };
        for (int i=0; i<ast.data.funcdef.blocklen; i++){
            ir_generate_stmnt(generator, *(ast.data.funcdef.block[i]));
        };
        generator_write_text(generator, "}\n");
    }
};

void ir_close(void *generator){
    generator_write_text(generator, IrData);
    generator_close(generator);
};
