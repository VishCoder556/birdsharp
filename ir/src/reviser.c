typedef struct {
    AST *value;
    AST *lastmention;
    bool known;
}Register;
Register registers[REGISTERS];


Reviser *reviser_init(Parser *parser){
    Reviser *reviser = malloc(sizeof(Reviser));
    reviser->name = parser->name;
    reviser->asts = parser->asts;
    reviser->astlen = parser->astlen;
    reviser->cur = 0;
    reviser->global = malloc(sizeof(Reviser_Global));
    reviser->global->functions = malloc(sizeof(Reviser_Function));
    reviser->global->functionlen = 0;
    reviser->global->scope.variables = malloc(sizeof(Reviser_Variable));
    reviser->global->scope.variablelen = 0;
    reviser->code = parser->code;

    for (int i=0; i<REGISTERS; i++){
        registers[i].known = 1;
        registers[i].value = NULL;
        registers[i].lastmention = NULL;
    }
    return reviser;
}

AST *load_ast(Reviser *reviser){
    if (reviser->cur == 0){
        return reviser->asts;
    }else {
        AST *ast1 = reviser->asts;
        for (int i=0; i<reviser->cur; i++){
            ast1 = ast1->next;
        };
        return ast1;
    };
}


AST_TypeInfo reviser_get_lhs_size(AST ast){
    AST *lhs = ast.data.opexpr.left;
    if (lhs != NULL){
        if (lhs->type == AST_REF){
            return 8;
        }else if(lhs->type == AST_REG){
            return ast.data.operation.reg.size;
        };
    }
    if (ast.data.opexpr.reg.size != 0){
        return ast.data.opexpr.reg.size;
    }else {
        AST *left = ast.data.opexpr.left;
        return left->typeinfo;
    };
    return (AST_TypeInfo)-1;
}


char *reviser_get_expr(Reviser *reviser, AST *ast){
    if (ast->type == AST_VAR){
        return ast->data.var.name;
    }else if (ast->type == AST_STRING){
        return ast->data.text.value;
    };
    return "";
}

Reviser_Variable *reviser_get_variable(Reviser_Variable *variable, int number){
    Reviser_Variable *initial = variable;
    for (int i=0; i<number; i++){
        initial = initial->next;
    }
    return initial;
};
Reviser_Variable *reviser_set_variable(Reviser_Variable *variable, int number, Reviser_Variable var){
    Reviser_Variable *initial = variable;
    if (number == 0){
        *initial = var;
        return initial;
    }
    for (int i=0; i<number-1; i++){
        initial = initial->next;
    };
    initial->next = malloc(sizeof(Reviser_Variable));
    *initial->next = var;
    return initial;
};



Reviser_Function *reviser_get_function(Reviser_Function *function, int number){
    Reviser_Function *initial = function;
    for (int i=0; i<number; i++){
        initial = initial->next;
    }
    return initial;
};
Reviser_Function *reviser_set_function(Reviser_Function *function, int number, Reviser_Function fun){
    Reviser_Function *initial = function;
    if (number == 0){
        *initial = fun;
        return initial;
    }
    for (int i=0; i<number-1; i++){
        initial = initial->next;
    };
    initial->next = malloc(sizeof(Reviser_Function));
    *initial->next = fun;
    return initial;
};



void revise_var(Reviser *reviser, AST *ast, char *name){
    for (int i=0; i<reviser->global->scope.variablelen; i++){
        Reviser_Variable *var = reviser_get_variable(reviser->global->scope.variables, i);
        char *str = var->name;
        ast->data.var.definition = var->definition;
        if (str == NULL){
            error_generate_reviser(reviser, "VariableError", "Variable has no value", ast);
        }
        if (strcmp(str, name) == 0){
            if (var->definition->type == AST_CONST){
                *ast = *var->definition->data.var.value;
            }
            ast->data.var.offset = -1;
            return;
        };
    };
    Reviser_Scope functionscope = reviser_get_function(reviser->global->functions, reviser->global->functionlen-1)->scope;
    int off = 0;
    switch (target){
        case TARGET_ARM64: break;
        case TARGET_X86_64: off = 150; break;
        case TARGET_LINUX: break;
        default: break;
    }
    for (int i=0; i<functionscope.variablelen; i++){
        Reviser_Variable *var = reviser_get_variable(functionscope.variables, i);
        char *str = var->name;
        ast->data.var.definition = var->definition;
        int name1 = strlen(name);
        int str1 = strlen(str);
        int len = name1 > str1 ? name1 : str1;
        if (strcmp(str, name) == 0){
            ast->data.var.offset = off + var->definition->typeinfo;
            return;
        };
        off += var->definition->typeinfo;
    };
    char string[100];
    snprintf(string, 100, "Variable '%s' does not exist", ast->data.var.name);
    error_generate_reviser(reviser, "VariableError", string, ast);
};

void reviser_eat_expr(Reviser *reviser, AST *ast);
void reviser_eat_expr(Reviser *reviser, AST *ast){
    if (ast->type == AST_INT){
        ast->typeinfo = 4; // Integer
    }else if (ast->type == AST_STRING){
    }else if (ast->type == AST_CHAR){
        ast->typeinfo = 1; // Character
    }else if(ast->type == AST_GT){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_GTE){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_LT){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_LTE){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_ARRAY){
    }else if (ast->type == AST_VAR){
        revise_var(reviser, ast, strdup(ast->data.text.value));
    }else if (ast->type == AST_METADATA){
        if (string_compare(ast->data.var.name, "len", strlen(ast->data.var.name)) == 0){
            for (int i=0; i<reviser->global->scope.variablelen; i++){
                Reviser_Variable *variable1 = reviser_get_variable(reviser->global->scope.variables, i);
                if (string_compare(variable1->name, ast->data.var.value->data.text.value, strlen(variable1->name)) == 0){
                    ast->type = AST_INT;
                    Reviser_Variable variable = variable;
                    char *text = variable.definition->data.var.value->data.text.value;
                    ast->data.text.value = string_int_to_string(strlen(text));
                };
            };
        }else if(ast->data.var.value->type == AST_VAR){

        };

    }else if (ast->type == AST_EQ){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
        if (ast->typeinfo == 0){
            if (ast->data.expr.left->typeinfo != 0){
                ast->typeinfo = ast->data.expr.left->typeinfo;
            }else if (ast->data.expr.right->typeinfo != 0){
                ast->typeinfo = ast->data.expr.right->typeinfo;
            };
        };
    }else if (ast->type == AST_NEQ){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
        if (ast->typeinfo == 0){
            if (ast->data.expr.left->typeinfo != 0){
                ast->typeinfo = ast->data.expr.left->typeinfo;
            }else if (ast->data.expr.right->typeinfo != 0){
                ast->typeinfo = ast->data.expr.right->typeinfo;
            };
        };
    }else if (ast->type == AST_DEREF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if (ast->type == AST_REF){
        reviser_eat_expr(reviser, ast->data.expr.left);
        ast->typeinfo = 8;
    }else if (ast->type == AST_PLUS){
        reviser_eat_expr(reviser, ast->data.expr.right);
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_REG){
        ast->typeinfo = ast->data.operation.reg.size;
    }
};

Register check_register(Reviser *reviser, AST_Reg reg){
    if (reg.reg <= REGISTERS){
        Register r = registers[reg.reg];
        if (r.value != NULL){
            if (r.value->type == AST_REG){
                return check_register(reviser, r.value->data.operation.reg);
            }
            return r;
        }
        // printf("Register: %s, Known: %d\n", regs[reg.reg], r.known);
    };
    return (Register){NULL, NULL, -1};
}

AST *reviser_expand(Reviser *reviser, AST *ast){
    if (ast->type == AST_REG){
        AST *value = check_register(reviser, ast->data.operation.reg).value;
        if (value != NULL){
            return reviser_expand(reviser, value);
        }
    }
    return ast;
};
Register find_register(Reviser *reviser, AST *ast){
    if (ast->type == AST_REG){
        return check_register(reviser, ast->data.operation.reg);
    }
    return (Register){NULL, NULL, -1};
}
void set_register(Reviser *reviser, AST_Reg reg, AST *value, bool known){
    if (reg.reg <= REGISTERS && reg.reg > 0){
        registers[reg.reg].lastmention = value;
        if (value != NULL){
            registers[reg.reg].value = reviser_expand(reviser, value->data.opexpr.right);
        }else {
            registers[reg.reg].value = NULL;
        }
    }
}


void reviser_eat_lhs(Reviser *reviser, AST *_ast){
    AST *ast = _ast->data.opexpr.left;
    if (ast->type == AST_VAR){
        revise_var(reviser, ast, ast->data.var.name);
    }else if (ast->type == AST_DEREF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if (ast->type == AST_REF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_REG){
        ast->typeinfo = ast->data.operation.reg.size;
        set_register(reviser, ast->data.operation.reg, _ast, 1);
    }
}

bool is_immediate_value(AST *ast){
    if (ast->type == AST_INT || ast->type == AST_CHAR){
        return 1;
    }else {
        return 0;
    };
}

bool reviser_is_immediate(Reviser *reviser, AST *ast){
    if (ast->type == AST_INT || ast->type == AST_CHAR){
        return 1;
    };
    if (ast->type == AST_REG){
        AST *value = check_register(reviser, ast->data.operation.reg).value;
        if (reviser_is_immediate(reviser, value)){
            return 1;
        }
    }
    return 0;
}


void reviser_eat_ast(Reviser *reviser, AST *ast);
void reviser_constant_fold(Reviser *reviser, AST *ast){
    Register reg = check_register(reviser, ast->data.operation.reg);
    AST *value = reg.value;
    AST *right = ast->data.operation.right;
    Register reg_right = find_register(reviser, right);
    if (value == NULL){
        return;
    }
    if (reviser_is_immediate(reviser, value) && reviser_is_immediate(reviser, right)){
        right = reviser_expand(reviser, right);
        int r = atoi(right->data.text.value);
        int l = atoi(value->data.text.value);
        char string[100];
        if (ast->type == AST_ADD){
            snprintf(string, 100, "%d", l + r);
        }else if (ast->type == AST_SUB){
            snprintf(string, 100, "%d", l - r);
        }else if (ast->type == AST_MUL){
            snprintf(string, 100, "%d", l * r);
        }else if (ast->type == AST_DIV){
            snprintf(string, 100, "%d", l / r);
        }
        AST *inner = malloc(sizeof(AST));
        inner->data.text.value = strdup(string);
        inner->type = AST_INT;
        AST oldast = *ast;
        ast->type = AST_MOV;
        ast->data.opexpr.left->type = AST_REG;
        ast->data.opexpr.left->data.operation.reg = oldast.data.operation.reg;
        ast->data.opexpr.right = inner;
        reg.lastmention->type = AST_NOP;
        if (reg_right.lastmention != NULL){
            reg_right.lastmention->type = AST_NOP;
        }
        set_register(reviser, ast->data.operation.reg, ast, 0);
        reviser_eat_ast(reviser, ast);
    }else {
        set_register(reviser, ast->data.operation.reg, NULL, 0);
    }
}

void reviser_equal_fold(Reviser *reviser, AST *ast){
    AST *left = reviser_expand(reviser, ast->data.opexpr.left);
    Register reg_left = find_register(reviser, left);
    AST *right = reviser_expand(reviser, ast->data.opexpr.right);
    Register reg_right = find_register(reviser, ast->data.opexpr.right);
    if (is_immediate_value(right)){
        ast->data.opexpr.right = right;
        if (reg_right.lastmention != NULL)
            reg_right.lastmention->type = AST_NOP;
        set_register(reviser, left->data.operation.reg, ast, 1);
    }else {
        set_register(reviser, left->data.operation.reg, NULL, 0);
    };
};

void reviser_eat_ast(Reviser *reviser, AST *ast){
    if (ast->type == AST_MOV){
        if (ast->data.opexpr.left != NULL){
            reviser_eat_lhs(reviser, ast);
        }
        ast->typeinfo = reviser_get_lhs_size(*ast);
        if (ast->typeinfo == 0){
            ast->typeinfo = ast->data.opexpr.right->typeinfo;
        }
        reviser_eat_expr(reviser, ast->data.opexpr.right);
        reviser_equal_fold(reviser, ast);
    }else if(ast->type == AST_SYSCALL){
        set_register(reviser, _reg_to_num("v0", 8), NULL, 0);
        for (int i = 0; i < REGISTERS; i++) {
            set_register(reviser, (AST_Reg){i, 8}, NULL, 0);
        }
        char *name = strdup(ast->data.text.value);
        char *res = "";
        if (string_compare(name, "write", strlen(name)) == 0){
            switch (target){
                case TARGET_ARM64: res = "33554436"; break;
                case TARGET_X86_64: res = "33554436"; break;
                case TARGET_LINUX: res = "1"; break;
                default: break;
            }
        }else if (string_compare(name, "exit", strlen(name)) == 0){
            switch (target){
                case TARGET_X86_64: res = "33554433"; break;
                case TARGET_ARM64: res = "33554433"; break;
                case TARGET_LINUX: res = "60"; break;
                default: break;
            }
        }else if (string_compare(name, "mmap", strlen(name)) == 0){
            switch (target){
                case TARGET_ARM64: res = "33554629"; break;
                case TARGET_X86_64: res = "33554629"; break;
                case TARGET_LINUX: res = "9"; break;
                default: break;
            }
        } else if (string_compare(name, "read", strlen(name)) == 0) {
            switch (target) {
                case TARGET_ARM64: res = "33554435"; break;
                case TARGET_X86_64: res = "33554435"; break;
                case TARGET_LINUX: res = "0"; break;
                default: break;
            }
        } else if (string_compare(name, "open", strlen(name)) == 0) {
            switch (target) {
                case TARGET_ARM64: res = "33554437"; break;
                case TARGET_X86_64: res = "33554437"; break;
                case TARGET_LINUX: res = "2"; break;
                default: break;
            }
        } else if (string_compare(name, "close", strlen(name)) == 0) {
            switch (target) {
                case TARGET_ARM64: res = "33554438"; break;
                case TARGET_X86_64: res = "33554438"; break;
                case TARGET_LINUX: res = "3"; break;
                default: break;
            }
        }
        ast->data.text.value = res;
    }else if(ast->type == AST_RET){
    }else if(ast->type == AST_PUSH){
        reviser_eat_expr(reviser, ast->data.operation.right);
    }else if(ast->type == AST_REF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_DEREF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_POP){
    }else if(ast->type == AST_CALL){
        set_register(reviser, _reg_to_num("v0", 8), NULL, 0);
        for (int i = 0; i < REGISTERS; i++) {
            set_register(reviser, (AST_Reg){i, 8}, NULL, 0);
        }
    }else if(ast->type == AST_CALLIF){
        set_register(reviser, _reg_to_num("v0", 8), NULL, 0);
        for (int i = 0; i < REGISTERS; i++) {
            set_register(reviser, (AST_Reg){i, 8}, NULL, 0);
        }
        reviser_eat_expr(reviser, ast->data.jmpif.reg);
    }else if(ast->type == AST_CALLIFN){
        set_register(reviser, _reg_to_num("v0", 8), NULL, 0);
        for (int i = 0; i < REGISTERS; i++) {
            set_register(reviser, (AST_Reg){i, 8}, NULL, 0);
        }
        reviser_eat_expr(reviser, ast->data.jmpif.reg);
    }else if(ast->type == AST_ADD){
        reviser_eat_expr(reviser, ast->data.operation.right);
        reviser_constant_fold(reviser, ast);
    }else if(ast->type == AST_SUB){
        reviser_eat_expr(reviser, ast->data.operation.right);
        reviser_constant_fold(reviser, ast);
    }else if(ast->type == AST_MUL){
        reviser_eat_expr(reviser, ast->data.operation.right);
        reviser_constant_fold(reviser, ast);
    }else if(ast->type == AST_DIV){
        reviser_eat_expr(reviser, ast->data.operation.right);
        reviser_constant_fold(reviser, ast);
    }else if(ast->type == AST_JMP){
    }else if(ast->type == AST_JMPIF){
        reviser_eat_expr(reviser, ast->data.jmpif.reg);
    }else if(ast->type == AST_JMPIFN){
        reviser_eat_expr(reviser, ast->data.jmpif.reg);
    }else if(ast->type == AST_LABEL){
        AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            reviser_eat_ast(reviser, current);
            current = current->next;
        }
    };
};

AST *current_ast = NULL;


void reviser_eat_body(Reviser *reviser){
    AST *ast = load_ast(reviser);
    if (ast->type == AST_FUNCDEF){
        AST *current = ast->data.funcdef.block;
        reviser->global->functionlen++;
        for (int i = 0; i < REGISTERS; i++) {
            set_register(reviser, (AST_Reg){i, 8}, NULL, 0);
        }
        while (current != NULL) {
            reviser_eat_ast(reviser, current);
            current = current->next;
        }
    }else if (ast->type == AST_LABEL){
        AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            reviser_eat_ast(reviser, current);
            current = current->next;
        }
    }else if(ast->type == AST_VAR){
    }else if(ast->type == AST_CONST){
    }
    reviser->cur++;
    current_ast = current_ast->next;
};


void reviser_spy_ast(Reviser *reviser, AST *ast){
    if(ast->type == AST_LABEL){
        AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            reviser_spy_ast(reviser, current);
            current = current->next;
        }
    }else if(ast->type == AST_LOCAL){
        Reviser_Function *function = reviser_get_function(reviser->global->functions, reviser->global->functionlen-1);
        reviser_set_variable(function->scope.variables, function->scope.variablelen++, (Reviser_Variable){strdup(ast->data.text.value), ast, ast->typeinfo, NULL});
    };
};



void reviser_spy_body(Reviser *reviser){
    if (current_ast->type == AST_FUNCDEF){
        reviser_set_function(reviser->global->functions, reviser->global->functionlen++, (Reviser_Function){(Reviser_Scope){0}, current_ast->data.funcdef.name, NULL});
        reviser_get_function(reviser->global->functions, reviser->global->functionlen-1)->scope.variablelen = 0;
        reviser_get_function(reviser->global->functions, reviser->global->functionlen-1)->scope.variables = malloc(sizeof(Reviser_Variable));
        AST *current = current_ast->data.funcdef.block;
        while (current != NULL) {
            reviser_spy_ast(reviser, current);
            current = current->next;
        }
    }else if (current_ast->type == AST_LABEL){
        reviser_set_function(reviser->global->functions, reviser->global->functionlen++, (Reviser_Function){(Reviser_Scope){0}, current_ast->data.funcdef.name, NULL});
        reviser_get_function(reviser->global->functions, reviser->global->functionlen-1)->scope.variablelen = 0;
        reviser_get_function(reviser->global->functions, reviser->global->functionlen-1)->scope.variables = malloc(sizeof(Reviser_Variable));
        AST *current = current_ast->data.funcdef.block;
        while (current_ast != NULL) {
            // reviser_spy_current_ast(reviser, current);
            current_ast = current_ast->next;
        }
    }else if(current_ast->type == AST_VAR){
        reviser_set_variable(reviser->global->scope.variables, reviser->global->scope.variablelen++, (Reviser_Variable){current_ast->data.var.name, current_ast, current_ast->typeinfo, NULL});
    }else if(current_ast->type == AST_CONST){
        reviser_set_variable(reviser->global->scope.variables, reviser->global->scope.variablelen++, (Reviser_Variable){current_ast->data.var.name, current_ast, -1, NULL});
    }
    reviser->cur++;
    current_ast = current_ast->next;
};


char reviser_spy(Reviser *reviser){
    if(reviser->cur >= reviser->astlen)
        return -1;
    if (current_ast == NULL){
        current_ast = reviser->asts;
    }
    reviser_spy_body(reviser);
    return 0;
};
char reviser_eat(Reviser *reviser){
    if(reviser->cur >= reviser->astlen)
        return -1;
    if (current_ast == NULL){
        current_ast = reviser->asts;
    }
    reviser_eat_body(reviser);
    return 0;
}
