#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

String_Builder IrData = {0};
int tempN = 0;

typedef struct {
    char *reg;
    short available;
}IRReg;
#define REGISTERS 32
IRReg IRRegs[REGISTERS];
int IRRegLen = 0;

char *regs[REGISTERS] = {
    "__", "s0", "__", "__", "__", "__", "__", "__", "t0", "__", "t1", "__", "s1", "t2",
    "t3", "t4", "t5", "t6",
    "t7", "t8", "t9", "t10", "t11", "t12", "t13", "t14", "t15",
    "s2", "s3", "s4", "s5", "s6"
};

static_assert((sizeof(regs) / sizeof(regs[0])) == REGISTERS, "Too many registers");

char *len_to_selector(int size){
    switch (size){
        case 1: return "i8";
        case 2: return "i16";
        case 4: return "i32";
        case 8: return "i64";
    }
    return "";
}
char *type_to_selector(AST_TypeInfo type){
    if (type.kind == KIND_ARRAY){
        return type_to_selector(*type.data.array.elem_type);
    }
    switch (typeinfo_to_len(type)){
        case 1: return "i8";
        case 2: return "i16";
        case 4: return "i32";
        case 8: return "i64";
        default: return "i64";
    }
    return "";
}
char *ir_alloc_reg() {
    for (int i=0; i<REGISTERS; i++){
        if (IRRegs[i].available == true){
            if (strcmp(IRRegs[i].reg, "__") != 0){
                IRRegs[i].available = false;
                return IRRegs[i].reg;
            };
        };
    }
    return "";
}
void ir_allocate_reg(char *reg){
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(IRRegs[i].reg, reg) == 0){
            IRRegs[i].available = true;
        }
    };
}

void ir_free_reg(char *reg){
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(IRRegs[i].reg, reg) == 0){
            IRRegs[i].available = true;
            break;
        };
    };
};

void ir_init(void *generator){
    Generator *gen = (Generator*)generator;

    for (int i=0; i<REGISTERS; i++){
        IRRegs[i].reg = regs[i];
        IRRegs[i].available = true;
    };
    char *v0_reg = ir_alloc_reg();
    generator_open_text(generator);
    // generator_write_text();
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
    if((reg[0] == 'a' || reg[0] == 't' || reg[0] == 's' || reg[0] == 'v') && strlen(reg) == 2){
        return true;
    }
    return false;
};
bool is_r_or_m(char *reg){
    if (strstr(reg, ">") || strstr(reg, "<") || strstr(reg, "==") || strstr(reg, "!=")) {
        return false;
    }
    char last = reg[strlen(reg)-1];
    if (strncmp(reg, "i8", 2) == 0 && last == ']'){
        return true;
    }else if (strncmp(reg, "i16", 3) == 0 && last == ']'){
        return true;
    }else if (strncmp(reg, "i32", 3) == 0 && last == ']'){
        return true;
    }else if (strncmp(reg, "i64", 3) == 0 && last == ']'){
        return true;
    }else if(is_r(reg)){
        return true;
    }
    return false;
};
void ir_free_smart(char *s) {
    if (!s) return;

    char *open = strchr(s, '[');
    char *close = strchr(s, ']');

    if (open && close) {
        int len = close - (open + 1);
        char *inner = malloc(len + 1);
        memcpy(inner, open + 1, len);
        inner[len] = '\0';
        
        ir_free_reg(inner);
        free(inner); 
    } else if (is_r(s)) {
        ir_free_reg(s);
    }
}
#define free_temp(s) ir_free_smart(s)

int typeinfo_from_reg(char *reg) {
    if (!reg || strlen(reg) == 0) return 0;

    if (strncmp(reg, "i64", 3) == 0) return 8;
    if (strncmp(reg, "i32", 3) == 0) return 4;
    if (strncmp(reg, "i16", 3) == 0)  return 2;
    if (strncmp(reg, "i8", 2) == 0)  return 1;

    int len = strlen(reg);
    char last_char = reg[len - 1];

    if (len == 3) {
        if (last_char == 'd') return 4;
        if (last_char == 'w') return 2;
        if (last_char == 'b') return 1;
    }

    if (is_r(reg)) {
        return 8;
    }

    return 8;
}

char *r_based_on_size(char *reg, int type){
    if (!reg) return reg;
    
    bool is_memory_access = false;
    char *base_reg = reg;
    
    if (strncmp(reg, "i32", 3) == 0){
        is_memory_access = true;
        base_reg = reg + 5;
        base_reg[strlen(base_reg)-1] = '\0';
        while (*base_reg == ' ' || *base_reg == '[') base_reg++;
    }else if (strncmp(reg, "i16", 3) == 0){
        is_memory_access = true;
        base_reg = reg + 4;
        base_reg[strlen(base_reg)-1] = '\0';
        while (*base_reg == ' ' || *base_reg == '[') base_reg++;
    }else if (strncmp(reg, "i8", 2) == 0){
        is_memory_access = true;
        base_reg = reg + 4;
        base_reg[strlen(base_reg)-1] = '\0';
        while (*base_reg == ' ' || *base_reg == '[') base_reg++;
    }else if (strncmp(reg, "i64", 3) == 0){
        is_memory_access = true;
        base_reg = reg + 5;
        base_reg[strlen(base_reg)-1] = '\0';
        while (*base_reg == ' ' || *base_reg == '[') base_reg++;
    }
    
if (is_memory_access) {
    if (strchr(base_reg, '[')) {
        return strdup(base_reg);
    }

    char out[100];
    char *size_selector = len_to_selector(type);
    snprintf(out, sizeof(out), "%s [%s]", size_selector, base_reg);
    return strdup(out);
}
    
    if ((reg[0] == 'a' || reg[0] == 't' || reg[0] == 's' || reg[0] == 'v')) {
        char out[20];
        strcpy(out, reg);
        
        char c = out[strlen(out)-1];
        if (c == 'd' || c == 'b' || c == 'w'){
            out[strlen(out)-1] = '\0';
        }
        if (type == 8 || strstr(reg, ":")) {
            return strdup(out);
        } else if (type == 4) {
            strcat(out, ":i32");
        } else if (type == 2) {
            strcat(out, ":i16");
        } else if (type == 1) {
            strcat(out, ":i8");
        }
        
        return strdup(out);
    }
    
    return reg;
}

char *move_imm(void *generator, char *str, char *reg, AST_TypeInfo typeinfo){
    if (strcmp(str, reg) == 0) {
        return "";
    }
    generator_write_text(generator, "\t");
    int type = 0;
    if (is_r_or_m(str) && (!is_r(str))){
        type = typeinfo_from_reg(str);
    }else if (is_r_or_m(reg) && (!is_r(reg))){
        type = typeinfo_from_reg(reg);
    }else if (typeinfo_from_reg(reg) > typeinfo_from_reg(str)) {
        type = typeinfo_from_reg(reg);
    }else if (typeinfo_from_reg(reg) < typeinfo_from_reg(str)) {
        type = typeinfo_from_reg(str);
    }else {
        type = typeinfo_to_len(typeinfo);
    }
    char *newstr = str;
    char *newreg = r_based_on_size(strdup(reg), type);
    if (is_r_or_m(str)){
        newstr = r_based_on_size(strdup(str), type);
    }
    generator_write_text(generator, newreg);
    generator_write_text(generator, " = ");
    generator_write_text(generator, newstr);
    generator_write_text(generator, "\n");
    free_temp(strdup(str));
    return strdup(newreg);
};

char *move(void *generator, AST ast, char *reg){
    char *a = move_imm(generator, ir_generate_expr(generator, ast), reg, ast.typeinfo);
    return a;
};

#define bin_op(_t) \
    char *reg1 = ir_alloc_reg(); \
    char *reg2 = ir_alloc_reg(); \
    char *lhs_ptr = move(generator, *(ast.data.expr.left), reg1); \
    char *right_ptr = move(generator, *(ast.data.expr.right), reg2); \
    int left = typeinfo_from_reg(lhs_ptr); \
    int right = typeinfo_from_reg(right_ptr); \
    int type = 0; \
    if (is_r_or_m(right_ptr) && (!is_r(right_ptr))){ \
        type = typeinfo_from_reg(right_ptr); \
    }else if (is_r_or_m(lhs_ptr) && (!is_r(lhs_ptr))){ \
        type = typeinfo_from_reg(lhs_ptr); \
    }else if (left > right) { \
        type = left; \
    }else if (left < right) { \
        type = right; \
    }else { \
        type = left; \
    } \
    char *sized_reg1 = r_based_on_size(lhs_ptr, type); \
    right_ptr = r_based_on_size(right_ptr, type); \
    generator_write_text(generator, "\t"); \
    generator_write_text(generator, _t); \
    generator_write_text(generator, " "); \
    generator_write_text(generator, sized_reg1); \
    generator_write_text(generator, ", "); \
    generator_write_text(generator, right_ptr); \
    generator_write_text(generator, "\n"); \
    free_temp(reg2);\
    return reg1;


int safe = 0;

void ir_generate_mode(void *generator, AST ast){
    if (strcmp(ast.data.mode.name, "mode.safe") == 0){
        safe = 1;
    }else if (strcmp(ast.data.mode.name, "mode.unsafe") == 0){
        safe = 0;
    }
}
int lblN = 0;

char *ir_generate_expr(void *generator, AST ast){
    if (ast.type == AST_MODE){
        ir_generate_mode(generator, ast);
    }else if (ast.type == AST_PLUS){
        bin_op("add");
    }else if (ast.type == AST_SUB){
        bin_op("sub");
    }else if (ast.type == AST_MUL){
        bin_op("mul");
    }else if (ast.type == AST_DIV){
        bin_op("div");
    }else if(ast.type == AST_INT){
        return ast.data.arg.value;
    }else if(ast.type == AST_CHAR){
        char string[100];
        snprintf(string, 100, "%d", ast.data.arg.value[0]);
        return strdup(string);
    }else if (ast.type == AST_STRING){
        char *reg = ir_alloc_reg();
        generator_write_text(generator, "\t");
        generator_write_text(generator, reg);
        generator_write_text(generator, " = &temp_");
        char string[100];
        snprintf(string, 100, "%d", tempN++);
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, "\n");
        arena_sb_append_cstr(&arena, &IrData, "%i8 temp_");
        arena_sb_append_cstr(&arena, &IrData, strdup(string));
        arena_sb_append_cstr(&arena, &IrData, " = \"");
        for (int i=0; i<strlen(ast.data.arg.value); i++){
            if (ast.data.arg.value[i] == '\n'){
                arena_sb_append_cstr(&arena, &IrData, "\\n");
            }else {
                char str[2];
                str[0] = ast.data.arg.value[i];
                str[1] = '\0';
                arena_sb_append_cstr(&arena, &IrData, strdup(str));
            };
        }
        arena_sb_append_cstr(&arena, &IrData, "\"\n");
        return reg;
    }else if(ast.type == AST_FUNCALL){
        AST *argcur = ast.data.funcall.args;
        AST *args = malloc(sizeof(AST) * ast.data.funcall.argslen);
        for (int i=0; i<ast.data.funcall.argslen; i++){
            args[i] = *argcur;
            argcur = argcur->next;
        };
        for (int i=ast.data.funcall.argslen; i > 0; i--){
            char string[100];
            snprintf(string, 100, "a%d", i-1);
            ir_allocate_reg(string);
            int idx = i == 0 ? 0 : i-1;
            move(generator, args[idx], string);
            free_temp(string);
        };
        generator_write_text(generator, "\tcall ");
        generator_write_text(generator, ast.data.funcall.name);
        generator_write_text(generator, "\n");
        return "v0";
    }else if(ast.type == AST_CAST){
        char *inner = ir_generate_expr(generator, *ast.data.expr.left);
        free_temp(strdup(inner));
        char string[100];
        snprintf(string, 100, "%s", r_based_on_size(inner, typeinfo_to_len(ast.typeinfo)));
        return strdup(string);
    }else if(ast.type == AST_EXPR){
        return ir_generate_expr(generator, *ast.data.expr.left);
    }else if (ast.type == AST_VAR){
        char string[100];
        if (ast.typeinfo.kind == KIND_ARRAY) {
            snprintf(string, 100, "&%s", ast.data.arg.value);
            return strdup(string); 
        }
        snprintf(string, 100, "%s [%s]", type_to_selector(ast.typeinfo), ast.data.arg.value);
        return strdup(string);
    }else if(ast.type == AST_SYSCALL){
        AST *current = ast.data.funcall.args;
        int arg_count = ast.data.funcall.argslen;

        AST **tmp_array = malloc(sizeof(AST*) * arg_count);
        AST *iter = current;
        for (int i = 0; i < arg_count; i++) {
            tmp_array[i] = iter;
            iter = iter->next;
        }

        for (int i = arg_count; i > 0; i--) {
            char string[100];
            snprintf(string, 100, "a%d", i - 1);
            ir_allocate_reg(string);
            move(generator, *tmp_array[i - 1], string);
            free_temp(string);
        }

        free(tmp_array);
        int syscallLen = strlen("syscall");
        if (strncmp(ast.data.funcall.name, "syscall", syscallLen) == 0){
            ast.data.funcall.name += syscallLen + 1;
        }
        generator_write_text(generator, "\tsyscall.");
        generator_write_text(generator, ast.data.funcall.name);
        generator_write_text(generator, "\n");
        return "v0";
    }else if(ast.type == AST_NEQ){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s != %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_EQ){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s == %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_GT){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s > %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_GTE){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s >= %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_LT){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s < %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_LTE){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        char string[100];
        snprintf(string, 100, "%s <= %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_MODULO){
        bin_op("mod");
    }else if(ast.type == AST_INDEX){
        int len = typeinfo_to_len(ast.typeinfo);
        char *reg2 = ir_alloc_reg();
        char *reg1 = ir_alloc_reg();
        move(generator, *ast.data.expr.right, reg2);
        if (len != 1){
            generator_write_text(generator, "\tmul ");
            generator_write_text(generator, reg2);
            generator_write_text(generator, ", ");
            char string[100];
            snprintf(string, 100, "%d", len);
            generator_write_text(generator, strdup(string));
            generator_write_text(generator, "\n");
        }
        if (safe == 1){
            char lbl[100];
            generator_write_text(generator, "\t");
            generator_write_text(generator, reg1);
            generator_write_text(generator, " = ");
            generator_write_text(generator, reg2);
            generator_write_text(generator, " > ");
            snprintf(lbl, 100, "%d", typeinfo_to_len(ast.data.expr.left->typeinfo));
            generator_write_text(generator, lbl);
            snprintf(lbl, 100, "_LBC_%d", lblN++);
            generator_write_text(generator, "\n\tjump labelend ");
            generator_write_text(generator, lbl);
            generator_write_text(generator, " if ");
            generator_write_text(generator, reg1);
            generator_write_text(generator, "\n%label ");
            generator_write_text(generator, lbl);
            generator_write_text(generator, "{\n\ta0 = -1\n\tcall exit\n}\n"); // We don't have real error messages yet
        }
        move(generator, *ast.data.expr.left, reg1);
        generator_write_text(generator, "\tadd ");
        generator_write_text(generator, reg1);
        generator_write_text(generator, ", ");
        generator_write_text(generator, reg2);
        generator_write_text(generator, "\n");
        free_temp(strdup(reg2));
        char string[100];
        snprintf(string, 100, "%s [%s]", type_to_selector(ast.typeinfo), reg1);
        return strdup(string);
    }else if(ast.type == AST_DEREF){
        char string[100];
        char *address_reg = ir_generate_expr(generator, *ast.data.expr.left);
        char *val_reg = ir_alloc_reg();
        snprintf(string, 100, "%s [%s]", type_to_selector(ast.typeinfo), address_reg);
        int typeinfo = typeinfo_to_len(ast.typeinfo);
        char *r = move_imm(generator, strdup(string), r_based_on_size(val_reg, typeinfo), ast.typeinfo);
        free_temp(strdup(address_reg));
        return r;
    }else if(ast.type == AST_UNKNOWN){

    }else {
        char string[100];
        snprintf(string, 100, "Unknown type found: '%d'", ast.type);
        error_generate_parser("TypeError", string, ast.row, ast.col, ast.filename);
    };
    return "";
};

char *ir_generate_lhs(void *generator, AST ast){
    if (ast.type == AST_MODE){
        ir_generate_mode(generator, ast);
    }else if (ast.type == AST_VAR){
        return ast.data.arg.value;
    }else if(ast.type == AST_CAST){
        return ir_generate_lhs(generator, *ast.data.expr.left);
    }else if(ast.type == AST_DEREF){
        char *reg = ir_alloc_reg();
        move(generator, *ast.data.expr.left, reg);
        return reg;
    }else if(ast.type == AST_INDEX){
        int len = typeinfo_to_len(ast.typeinfo);
        char *reg2 = ir_alloc_reg();
        char *reg1 = ir_alloc_reg();
        move(generator, *ast.data.expr.left, reg1);
        move(generator, *ast.data.expr.right, reg2);
        if (len != 1){
            generator_write_text(generator, "\tmul ");
            generator_write_text(generator, reg2);
            generator_write_text(generator, ", ");
            char string[100];
            snprintf(string, 100, "%d", len);
            generator_write_text(generator, strdup(string));
            generator_write_text(generator, "\n");
        }
        generator_write_text(generator, "\tadd ");
        generator_write_text(generator, reg1);
        generator_write_text(generator, ", ");
        generator_write_text(generator, reg2);
        generator_write_text(generator, "\n");
        free_temp(strdup(reg2));
        return reg1;
    };
    return "";
}

char *current_function = "";

void ir_generate_stmnt(void *generator, AST ast);
void ir_generate_stmnt(void *generator, AST ast){
    if (ast.type == AST_MODE){
        ir_generate_mode(generator, ast);
    }else if (ast.type == AST_RET){
        if (strcmp(current_function, "main") == 0){
            move(generator, *ast.data.ret.ret, "a0");
            generator_write_text(generator, "\tsyscall.exit\n");
        }else {
            move(generator, *ast.data.ret.ret, "v0");
            generator_write_text(generator, "\tret\n");
        }
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
            if (ast.data.assign.assignto->type){
                char *rhs = ir_generate_expr(generator, *(ast.data.assign.assignto));
                snprintf(string, 100, "%s [%s]", type_to_selector(ast.typeinfo), var);
                move_imm(generator, rhs, strdup(string), ast.data.assign.assignto->typeinfo);
            };
            return;
        }
        
        if (ast.data.assign.from->type == AST_INDEX || ast.data.assign.from->type == AST_DEREF) {
            generator_write_text(generator, "\tpush ");
            generator_write_text(generator, var);
            generator_write_text(generator, "\n");
            char *rhs = ir_generate_expr(generator, *(ast.data.assign.assignto));
            char *sized_rhs = rhs;
            if (is_r_or_m(rhs)) {
                sized_rhs = r_based_on_size(strdup(rhs), typeinfo);
            }
            generator_write_text(generator, "\tpop ");
            generator_write_text(generator, var);
            generator_write_text(generator, "\n");
            generator_write_text(generator, "\t");
            generator_write_text(generator, type_to_selector(ast.typeinfo));
            generator_write_text(generator, " [");
            generator_write_text(generator, var);
            generator_write_text(generator, "] = ");
            generator_write_text(generator, sized_rhs);
            generator_write_text(generator, "\n");
            if (sized_rhs != rhs) {
                free(sized_rhs);
            }
            free_temp(rhs);
        } else {
            snprintf(string, 100, "%s [%s]", type_to_selector(ast.typeinfo), var);
            move(generator, *(ast.data.assign.assignto), strdup(string));
        }
    }else if(ast.type == AST_IF){
        char end_lbl[100];
        snprintf(end_lbl, 100, "_LBC_END_%d", lblN++);

        char next_branch_lbl[100];
        snprintf(next_branch_lbl, 100, "_LBC_NEXT_%d", lblN++);

        char *reg = ir_alloc_reg();
        move(generator, *ast.data.if1.block.condition, reg);

        generator_write_text(generator, "\tjump label ");
        generator_write_text(generator, next_branch_lbl);
        generator_write_text(generator, " ifnot ");
        generator_write_text(generator, reg);
        generator_write_text(generator, "\n");
        
        for (int i=0; i < ast.data.if1.block.statementlen; i++){
            ir_generate_stmnt(generator, *(ast.data.if1.block.statements[i]));
        }
        generator_write_text(generator, "\tjump label ");
        generator_write_text(generator, end_lbl);
        generator_write_text(generator, "\n");

        for (int j = 0; j < ast.data.if1.elseiflen; j++) {
            generator_write_text(generator, "%label ");
            generator_write_text(generator, next_branch_lbl);
            generator_write_text(generator, " {\n");

            snprintf(next_branch_lbl, 100, "_LBC_NEXT_%d", lblN++);

            move(generator, *ast.data.if1.elseif[j].condition, reg);

            generator_write_text(generator, "\tjump label ");
            generator_write_text(generator, next_branch_lbl);
            generator_write_text(generator, " ifnot ");
            generator_write_text(generator, reg);
            generator_write_text(generator, "\n");

            for (int i=0; i < ast.data.if1.elseif[j].statementlen; i++){
                ir_generate_stmnt(generator, *(ast.data.if1.elseif[j].statements[i]));
            }
            generator_write_text(generator, "\tjump label ");
            generator_write_text(generator, end_lbl);
            generator_write_text(generator, "\n}\n");
        }

        generator_write_text(generator, "%label ");
        generator_write_text(generator, next_branch_lbl);
        generator_write_text(generator, " {\n");
        for (int i=0; i < ast.data.if1.elselen; i++){
            ir_generate_stmnt(generator, *ast.data.if1.else1[i]);
        }
        generator_write_text(generator, "\tjump label ");
        generator_write_text(generator, end_lbl);
        generator_write_text(generator, "\n");
        generator_write_text(generator, "}\n");

        generator_write_text(generator, "%label ");
        generator_write_text(generator, end_lbl);
        generator_write_text(generator, " {\n}\n");

        free_temp(reg);
    }else if(ast.type == AST_WHILE){
        char string[100];
        snprintf(string, 100, "_LBC%d", lblN++);
        generator_write_text(generator, "%label ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, " {\n");
        char *reg = ir_alloc_reg();
        char *r = move(generator, *ast.data.while1.condition, reg);
        generator_write_text(generator, "\tjump labelend ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, " ifnot ");
        generator_write_text(generator, r);
        generator_write_text(generator, "\n");
        free_temp(r);
        for (int i=0; i<ast.data.while1.blocklen; i++){
            ir_generate_stmnt(generator, *(ast.data.while1.block[i]));
        };
        generator_write_text(generator, "\tjump label ");
        generator_write_text(generator, strdup(string));
        generator_write_text(generator, "\n");
        generator_write_text(generator, "}\n");
        free_temp(reg);
    }else {
        char *res = ir_generate_expr(generator, ast);
        free_temp(res);
    };
};

void ir_generate_ast(void *generator, AST ast){
    if (ast.type == AST_MODE){
        ir_generate_mode(generator, ast);
    }else if (ast.type == AST_FUNCDEF){
        for (int i=0; i<REGISTERS; i++){
            IRRegs[i].available = true;
        }
        lblN = 0;
        generator_write_text(generator, "%func ");
        generator_write_text(generator, ast.data.funcdef.name);
        generator_write_text(generator, " {\n");
        current_function = ast.data.funcdef.name;
        for (int i=0; i<ast.data.funcdef.argslen; i++){
            Argument arg = ast.data.funcdef.args[i];
            int typeinfo = typeinfo_to_len(arg.type);
            char argS[100];
            snprintf(argS, 100, "a%d", i);
            char string[100];
            snprintf(string, 100, "\t%%local %s, %d\n\t%s [%s] = %s\n", arg.arg, typeinfo, len_to_selector(typeinfo), arg.arg, r_based_on_size(strdup(argS), typeinfo));
            generator_write_text(generator, strdup(string));
        };
        AST *cur = ast.data.funcdef.block;
        for (int i=0; i<ast.data.funcdef.blocklen; i++){
            ir_generate_stmnt(generator, *cur);
            cur = cur->next;
        };
        generator_write_text(generator, "}\n");
    }else if (ast.type == AST_ASSIGN){
        arena_sb_append_cstr(&arena, &IrData, "%");
        arena_sb_append_cstr(&arena, &IrData, type_to_selector(ast.typeinfo));
        arena_sb_append_cstr(&arena, &IrData, " ");
        arena_sb_append_cstr(&arena, &IrData, ast.data.assign.from->data.arg.value);
        arena_sb_append_cstr(&arena, &IrData, " = ");
        arena_sb_append_cstr(&arena, &IrData, ir_generate_expr(generator, *ast.data.assign.assignto));
        arena_sb_append_cstr(&arena, &IrData, "\n");
    }else if(ast.type == AST_UNKNOWN){
        ;
    }else if(ast.type == AST_STRUCT){
        ;
    }else {
        char string[100];
        snprintf(string, 100, "Unknown type found: '%d'", ast.type);
        error_generate_parser("TypeError", string, ast.row, ast.col, ast.filename);
    };
};

void ir_close(void *generator){
    arena_sb_append_null(&arena, &IrData);
    generator_write_text(generator, IrData.items);
    arena_reset(&arena);
    arena_free(&arena);
    generator_close(generator);
}
