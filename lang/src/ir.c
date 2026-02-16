#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} StringBuilder;

StringBuilder *sb_create(size_t initial_capacity) {
    StringBuilder *sb = malloc(sizeof(StringBuilder));
    if (!sb) return NULL;
    
    sb->data = malloc(initial_capacity);
    if (!sb->data) {
        free(sb);
        return NULL;
    }
    
    sb->data[0] = '\0';
    sb->len = 0;
    sb->capacity = initial_capacity;
    return sb;
}

void sb_append(StringBuilder *sb, const char *str) {
    if (!sb || !str) return;
    
    size_t str_len = strlen(str);
    
    // Grow if needed (double capacity)
    while (sb->len + str_len + 1 > sb->capacity) {
        sb->capacity *= 2;
        char *new_data = realloc(sb->data, sb->capacity);
        if (!new_data) {
            fprintf(stderr, "StringBuilder: Out of memory!\n");
            exit(1);
        }
        sb->data = new_data;
    }
    
    memcpy(sb->data + sb->len, str, str_len);
    sb->len += str_len;
    sb->data[sb->len] = '\0';
}

void sb_append_char(StringBuilder *sb, char c) {
    char str[2] = {c, '\0'};
    sb_append(sb, str);
}

char *sb_get(StringBuilder *sb) {
    return sb->data;
}

void sb_free(StringBuilder *sb) {
    if (sb) {
        free(sb->data);
        free(sb);
    }
}

void sb_clear(StringBuilder *sb) {
    if (sb) {
        sb->len = 0;
        sb->data[0] = '\0';
    }
}
StringBuilder *IrData = NULL;
int tempN = 0;

typedef struct {
    char *reg;
    bool available;
}IRReg;
IRReg IRRegs[14];
int IRRegLen = 0;

char *regs[14] = {
    "__", "s0", "a3", "a2", "a1", "a0", "a4", "a5", "t0", "__", "t1", "__", "s1", "t2"
};


char *len_to_selector(int size){
    switch (size){
        case 1: return "i8";
        case 2: return "i16";
        case 4: return "i32";
        case 8: return "i64";
    }
    return "i64";
}

char *ir_alloc_reg() {
    for (int i=0; i<14; i++){
        if (IRRegs[i].available == true){
            if (strcmp(IRRegs[i].reg, "__") != 0){
                IRRegs[i].available = false;
                return IRRegs[i].reg;
            };
        };
    }
    return "i64 [_temp_0]";
}
void ir_allocate_reg(char *reg){
    for (int i=0; i<14; i++){
        if (strcmp(IRRegs[i].reg, reg) == 0){
            IRRegs[i].available = true;
        }
    };
}

void ir_free_reg(char *reg){
    for (int i=0; i<14; i++){
        if (strcmp(IRRegs[i].reg, reg) == 0){
            IRRegs[i].available = true;
            break;
        };
    };
};

void IRRegs_clear(){
    for (int i=0; i<14; i++){
        IRRegs[i].available = true;
    }
}

void ir_init(void *generator){
    IrData = sb_create(8192);
    Generator *gen = (Generator*)generator;
    strcat(gen->output->filename, ".ir");
    for (int i=0; i<14; i++){
        IRRegs[i].reg = regs[i];
    };
    IRRegs_clear();
    char *v0_reg = ir_alloc_reg();
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


char *r_based_on_size(char *reg, int type) {
    if (!reg || reg[0] == '\0') return reg;

    char out[128] = {0};
    char *size_selector = len_to_selector(type);
    char *base_reg = reg;
    bool is_memory_access = false;

    if (strncmp(reg, "i32", 3) == 0) {
        is_memory_access = true;
        base_reg = reg + 3; 
    } else if (strncmp(reg, "i16", 3) == 0) {
        is_memory_access = true;
        base_reg = reg + 3;
    } else if (strncmp(reg, "i8", 2) == 0) {
        is_memory_access = true;
        base_reg = reg + 2;
    } else if (strncmp(reg, "i64", 3) == 0) {
        is_memory_access = true;
        base_reg = reg + 3;
    }

    if (is_memory_access) {
        while (*base_reg == ' ' || *base_reg == '[') base_reg++;
        
        char *bracket_end = strchr(base_reg, ']');
        if (bracket_end) {
            int reg_name_len = (int)(bracket_end - base_reg);
            snprintf(out, sizeof(out), "%s [%.*s]", size_selector, reg_name_len, base_reg);
        } else {
            snprintf(out, sizeof(out), "%s [%s]", size_selector, base_reg);
        }
        return strdup(out);
    }

    if (reg[0] == 'a' || reg[0] == 't' || reg[0] == 's' || reg[0] == 'v') {
        char clean_reg[32] = {0};
        char *colon = strchr(reg, ':');
        int name_len = colon ? (int)(colon - reg) : (int)strlen(reg);
        
        if (name_len > 31) name_len = 31;
        memcpy(clean_reg, reg, name_len);

        if (type == 8) {
            return strdup(clean_reg); 
        }else if(type > 8){
            return strdup(clean_reg);  // Use ptr
        }else {
            snprintf(out, sizeof(out), "%s:i%d", clean_reg, type * 8);
            return strdup(out);
        }
    }

    // 4. Final Fallback
    return strdup(reg);
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
    // generator_write_text(generator, );
    char *newstr = str;
    char *newreg = r_based_on_size(reg, type);
    if (is_r_or_m(str)){
        newstr = r_based_on_size(str, type);
    }
    generator_write_text(generator, newreg);
    generator_write_text(generator, " = ");
    generator_write_text(generator, newstr);
    generator_write_text(generator, "\n");
    free_temp(str);
    return strdup(newreg);
};

char *move(void *generator, AST ast, char *reg){
    return move_imm(generator, ir_generate_expr(generator, ast), reg, ast.typeinfo);
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
    free_temp(reg1); \
    free_temp(reg2); \
    return reg1;


char *ir_generate_expr(void *generator, AST ast){
    if (ast.type == AST_PLUS){
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
        generator_write_text(generator, string);
        generator_write_text(generator, "\n");
        sb_append(IrData, "%i8 temp_");
        sb_append(IrData, string);
        sb_append(IrData, " = \"");
        for (int i=0; i<strlen(ast.data.arg.value); i++){
            if (ast.data.arg.value[i] == '\n'){
                sb_append(IrData, "\\n");
            }else {
                char str[2];
                str[0] = ast.data.arg.value[i];
                str[1] = '\0';
                sb_append(IrData, str);
            };
        }
        sb_append(IrData, "\"\n");
        return reg;
    }else if(ast.type == AST_FUNCALL){
        for (int i=ast.data.funcall.argslen; i > 0; i--){
            char string[100];
            snprintf(string, 100, "a%d", i-1);
            ir_allocate_reg(string);
            int idx = i == 0 ? 0 : i-1;
            move(generator, *ast.data.funcall.args[idx], string);
            free_temp(string);
        };
        generator_write_text(generator, "\tcall ");
        generator_write_text(generator, ast.data.funcall.name);
        generator_write_text(generator, "\n");
        return "v0";
    }else if(ast.type == AST_CAST){
        char *inner = ir_generate_expr(generator, *ast.data.expr.left);
        free_temp(inner);
        // Generate a move that respects the cast target type
        char string[100];
        snprintf(string, 100, "%s", r_based_on_size(inner, typeinfo_to_len(ast.typeinfo)));
        return strdup(string);
    }else if(ast.type == AST_EXPR){
        return ir_generate_expr(generator, *ast.data.expr.left);
    }else if (ast.type == AST_VAR){
        AST *assign = ast.data.optvar.opt;
        if (assign != NULL){
            if (assign->data.assign.alias){
                return ir_generate_expr(generator, *assign->data.assign.assignto);
            }
        }
        char string[100];
        
        if (ast.typeinfo.kind == KIND_ARRAY || 
            (ast.typeinfo.type != NULL && strncmp(ast.typeinfo.type, "struct", 6) == 0)){
            snprintf(string, 100, "&%s", ast.data.arg.value);
            return strdup(string);
        }
        
        snprintf(string, 100, "%s [%s]", len_to_selector(typeinfo_to_len(ast.typeinfo)), ast.data.arg.value);
        return strdup(string);
    }else if(ast.type == AST_SYSCALL){
        for (int i=ast.data.funcall.argslen; i > 0; i--){
            char string[100];
            snprintf(string, 100, "a%d", i-1);
            ir_allocate_reg(string);
            move(generator, *ast.data.funcall.args[i-1], string);
            free_temp(string);
        };
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
        int lhslen = typeinfo_to_len(ast.data.expr.left->typeinfo);
        int rhslen = typeinfo_to_len(ast.data.expr.right->typeinfo);
        rhs = r_based_on_size(rhs, rhslen);
        lhs = r_based_on_size(lhs, lhslen);
        if (lhslen > rhslen){
            rhs = r_based_on_size(rhs, lhslen);
        }else {
            lhs = r_based_on_size(lhs, rhslen);
        }
        char string[100];
        snprintf(string, 100, "%s != %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_EQ){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        int lhslen = typeinfo_to_len(ast.data.expr.left->typeinfo);
        int rhslen = typeinfo_to_len(ast.data.expr.right->typeinfo);
        rhs = r_based_on_size(rhs, rhslen);
        lhs = r_based_on_size(lhs, lhslen);
        if (lhslen > rhslen){
            rhs = r_based_on_size(rhs, lhslen);
        }else {
            lhs = r_based_on_size(lhs, rhslen);
        }
        char string[100];
        snprintf(string, 100, "%s == %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_GT){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        int lhslen = typeinfo_to_len(ast.data.expr.left->typeinfo);
        int rhslen = typeinfo_to_len(ast.data.expr.right->typeinfo);
        rhs = r_based_on_size(rhs, rhslen);
        lhs = r_based_on_size(lhs, lhslen);
        if (lhslen > rhslen){
            rhs = r_based_on_size(rhs, lhslen);
        }else {
            lhs = r_based_on_size(lhs, rhslen);
        }
        char string[100];
        snprintf(string, 100, "%s < %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_GTE){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        int lhslen = typeinfo_to_len(ast.data.expr.left->typeinfo);
        int rhslen = typeinfo_to_len(ast.data.expr.right->typeinfo);
        rhs = r_based_on_size(rhs, rhslen);
        lhs = r_based_on_size(lhs, lhslen);
        if (lhslen > rhslen){
            rhs = r_based_on_size(rhs, lhslen);
        }else {
            lhs = r_based_on_size(lhs, rhslen);
        }
        char string[100];
        snprintf(string, 100, "%s <= %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_LT){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        int lhslen = typeinfo_to_len(ast.data.expr.left->typeinfo);
        int rhslen = typeinfo_to_len(ast.data.expr.right->typeinfo);
        rhs = r_based_on_size(rhs, rhslen);
        lhs = r_based_on_size(lhs, lhslen);
        if (lhslen > rhslen){
            rhs = r_based_on_size(rhs, lhslen);
        }else {
            lhs = r_based_on_size(lhs, rhslen);
        }
        char string[100];
        snprintf(string, 100, "%s > %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_LTE){
        char *lhs = ir_generate_expr(generator, *ast.data.expr.left);
        char *rhs = ir_generate_expr(generator, *ast.data.expr.right);
        int lhslen = typeinfo_to_len(ast.data.expr.left->typeinfo);
        int rhslen = typeinfo_to_len(ast.data.expr.right->typeinfo);
        rhs = r_based_on_size(rhs, rhslen);
        lhs = r_based_on_size(lhs, lhslen);
        if (lhslen > rhslen){
            rhs = r_based_on_size(rhs, lhslen);
        }else {
            lhs = r_based_on_size(lhs, rhslen);
        }
        char string[100];
        snprintf(string, 100, "%s >= %s", lhs, rhs);
        free_temp(lhs);
        free_temp(rhs);
        return strdup(string);
    }else if(ast.type == AST_MODULO){
        bin_op("mod");
    }else if(ast.type == AST_INDEX){
        int len = typeinfo_to_len(ast.typeinfo);
        char *reg2 = ir_alloc_reg();
        char *reg1 = ir_alloc_reg();
        char *l = move(generator, *ast.data.expr.left, reg1);
        char *r = move(generator, *ast.data.expr.right, reg2);
        if (len != 1){
            generator_write_text(generator, "\tmul ");
            generator_write_text(generator, r);
            generator_write_text(generator, ", ");
            char string[100];
            snprintf(string, 100, "%d", len);
            generator_write_text(generator, string);
            generator_write_text(generator, "\n");
        }
        generator_write_text(generator, "\tadd ");
        generator_write_text(generator, l);
        generator_write_text(generator, ", ");
        generator_write_text(generator, r);
        generator_write_text(generator, "\n");
        free_temp(reg2);
        char string[100];
        snprintf(string, 100, "%s [%s]", len_to_selector(len), reg1);
        return strdup(string);
    }else if(ast.type == AST_DEREF){
        char string[100];
        char *address_reg = ir_generate_expr(generator, *ast.data.expr.left);
        char *val_reg = ir_alloc_reg();
        int typeinfo = typeinfo_to_len(ast.typeinfo);
        snprintf(string, 100, "%s [%s]", len_to_selector(typeinfo), address_reg);
        char *r = move_imm(generator, string, r_based_on_size(val_reg, typeinfo), ast.typeinfo);
        free_temp(address_reg);
        return r;
    }else if(ast.type == AST_MODE){
        ;
    }else {
        char string[100];
        snprintf(string, 100, "Unknown type found: '%d'", ast.type);
        error_generate_parser("TypeError", string, ast.row, ast.col, ast.filename);
    };
    return "";
};

char *ir_generate_lhs(void *generator, AST ast){
    if (ast.type == AST_VAR){
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
            generator_write_text(generator, string);
            generator_write_text(generator, "\n");
        }
        generator_write_text(generator, "\tadd ");
        generator_write_text(generator, reg1);
        generator_write_text(generator, ", ");
        generator_write_text(generator, reg2);
        generator_write_text(generator, "\n");
        free_temp(reg2);
        return reg1;
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
        if (ast.data.assign.alias){
            return;
        }
        if (ast.data.assign.from->type == AST_VAR && ast.data.assign.new == true) {
            generator_write_text(generator, "\t%local ");
            generator_write_text(generator, var);
            generator_write_text(generator, ", ");
            snprintf(string, 100, "%d", typeinfo);
            generator_write_text(generator, string);
            generator_write_text(generator, "\n");
            if (ast.data.assign.assignto->type){
                char *rhs = ir_generate_expr(generator, *(ast.data.assign.assignto));
                snprintf(string, 100, "%s [%s]", len_to_selector(typeinfo), var);
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
            // Size the RHS to match the destination type
            if (is_r_or_m(rhs)) {
                sized_rhs = r_based_on_size(rhs, typeinfo);
            }
            generator_write_text(generator, "\tpop ");
            generator_write_text(generator, var);
            generator_write_text(generator, "\n");
            generator_write_text(generator, "\t");
            generator_write_text(generator, len_to_selector(typeinfo));
            generator_write_text(generator, " [");
            generator_write_text(generator, var);
            generator_write_text(generator, "] = ");
            generator_write_text(generator, sized_rhs);
            generator_write_text(generator, "\n");
            if (sized_rhs != rhs) {
                free(sized_rhs);
            }
            free_temp(rhs);
            free_temp(var);
        } else {
            snprintf(string, 100, "%s [%s]", len_to_selector(typeinfo), var);
            move(generator, *(ast.data.assign.assignto), string);
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

        AST *statement = ast.data.if1.block.statements;
        for (int i=0; i < ast.data.if1.block.statementlen; i++){
            ir_generate_stmnt(generator, *statement);
            statement = statement->next;
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
            statement = ast.data.if1.elseif[j].statements;
            for (int i=0; i < ast.data.if1.elseif[j].statementlen; i++){
                ir_generate_stmnt(generator, *statement);
                statement = statement->next;
            }
            generator_write_text(generator, "\tjump label ");
            generator_write_text(generator, end_lbl);
            generator_write_text(generator, "\n}\n");
        }

        generator_write_text(generator, "%label ");
        generator_write_text(generator, next_branch_lbl);
        generator_write_text(generator, " {\n");
        statement = ast.data.if1.else1;
        for (int i=0; i < ast.data.if1.elselen; i++){
            ir_generate_stmnt(generator, *statement);
            statement = statement->next;
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
        generator_write_text(generator, string);
        generator_write_text(generator, " {\n");
        char *reg = ir_alloc_reg();
        char *r = move(generator, *ast.data.while1.condition, reg);
        generator_write_text(generator, "\tjump labelend ");
        generator_write_text(generator, string);
        generator_write_text(generator, " ifnot ");
        generator_write_text(generator, r);
        generator_write_text(generator, "\n");
        free_temp(r);
        for (int i=0; i<ast.data.while1.blocklen; i++){
            ir_generate_stmnt(generator, *(ast.data.while1.block[i]));
        };
        generator_write_text(generator, "\tjump label ");
        generator_write_text(generator, string);
        generator_write_text(generator, "\n");
        generator_write_text(generator, "}\n");
        free_temp(reg);
    }else {
        char *res = ir_generate_expr(generator, ast);
        free_temp(res);
    };
};

void ir_generate_ast(void *generator, AST ast){
    if (ast.type == AST_FUNCDEF){
        if (ast.data.funcdef.blocklen == -1){
            char string[100];
            snprintf(string, 100, "%%extern %s\n", ast.data.funcdef.name);
            generator_write_text(generator, string);
            return;
        }
        lblN = 0;
        IRRegs_clear();
        generator_write_text(generator, "%func ");
        generator_write_text(generator, ast.data.funcdef.name);
        generator_write_text(generator, " {\n");
        generator_write_text(generator, "\t%local _temp_0, 8\n");
        for (int i=0; i<ast.data.funcdef.argslen; i++){
            Argument arg = *ast.data.funcdef.args[i];
            int typeinfo = typeinfo_to_len(arg.type);
            char argS[100];
            snprintf(argS, 100, "a%d", i);
            char string[100];
            char *sized_arg = r_based_on_size(argS, typeinfo);
            snprintf(string, 100, "\t%%local %s, %d\n\t%s [%s] = %s\n", arg.arg, typeinfo, len_to_selector(typeinfo), arg.arg, sized_arg);
            generator_write_text(generator, string);
            free(sized_arg);
        };
        for (int i=0; i<ast.data.funcdef.blocklen; i++){
            ir_generate_stmnt(generator, *(ast.data.funcdef.block[i]));
        };
        generator_write_text(generator, "}\n");
    }else if (ast.type == AST_ASSIGN){
        sb_append(IrData, "%");
        sb_append(IrData, len_to_selector(typeinfo_to_len(ast.typeinfo)));
        sb_append(IrData, " ");
        sb_append(IrData, ast.data.assign.from->data.arg.value); // Typechecker made sure it's AST_VAR
        sb_append(IrData, " = ");
        sb_append(IrData, ir_generate_expr(generator, *ast.data.assign.assignto));
        sb_append(IrData, "\n");
    }else if(ast.type == AST_MODE){

    }else if(ast.type == AST_STRUCT){
        ;
    }else {
        char string[100];
        snprintf(string, 100, "Unknown type found: '%d'", ast.type);
        error_generate_parser("TypeError", string, ast.row, ast.col, ast.filename);
    };
};

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

void ir_close(void *generator){
    Generator *gen = (Generator*)generator;

    generator_write_text(generator, sb_get(IrData));
    generator_close(generator);
    char ir_file[512];
    snprintf(ir_file, sizeof(ir_file), "%s", gen->output->filename);

    char final_output[512];
    strncpy(final_output, gen->output->filename, sizeof(final_output) - 1);
    final_output[sizeof(final_output) - 1] = '\0';

    char *dot = strrchr(final_output, '.');
    if (dot != NULL) {
        *dot = '\0';
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "irc %s -o %s", ir_file, final_output);

    int result = system(cmd);
};
