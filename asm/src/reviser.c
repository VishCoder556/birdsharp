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
    reviser->regs = malloc(sizeof(Reviser_VirtualReg) * REVISER_REGISTERS);
    for (int i=0; i<REVISER_REGISTERS; i++){
        reviser->regs[i].reg = (AST_Reg){i, 8};
        reviser->regs[i].credits = 0;
        reviser->regs[i].firstmention = 0;
        reviser->regs[i].lastmention = 0;
    };
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
            AST_Reg reg = _reg_to_num(strdup(lhs->data.text.value), lhs->typeinfo);
            return (AST_TypeInfo)reg.size;
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
        int name1 = strlen(name);
        int str1 = strlen(str);
        int len = name1 > str1 ? name1 : str1;
        if (string_compare(str, name, len) == 0){
            if (var->definition->type == AST_CONST){
                *ast = *var->definition->data.var.value;
            }
            ast->data.var.offset = -1;
            return;
        };
    };
    Reviser_Scope functionscope = reviser_get_function(reviser->global->functions, reviser->global->functionlen-1)->scope;
    int off = 0;
    for (int i=0; i<functionscope.variablelen; i++){
        Reviser_Variable *var = reviser_get_variable(functionscope.variables, i);
        char *str = var->name;
        ast->data.var.definition = var->definition;
        int name1 = strlen(name);
        int str1 = strlen(str);
        int len = name1 > str1 ? name1 : str1;
        if (string_compare(str, name, strlen(name)) == 0){
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
                    Reviser_Variable variable = *variable1;
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
        AST_Reg reg = _reg_to_num(ast->data.text.value, ast->typeinfo);
        ast->typeinfo = reg.size;
    }
};


void reviser_eat_lhs(Reviser *reviser, AST *ast){
    if (ast->type == AST_VAR){
        revise_var(reviser, ast, ast->data.var.name);
    }else if (ast->type == AST_DEREF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if (ast->type == AST_REF){
        reviser_eat_expr(reviser, ast->data.expr.left);
    }else if(ast->type == AST_REG){
        AST_Reg reg = _reg_to_num(ast->data.text.value, ast->typeinfo);
        ast->typeinfo = reg.size;
    }
}

void reviser_eat_ast(Reviser *reviser, AST *ast);
void reviser_eat_ast(Reviser *reviser, AST *ast){
    if (ast->type == AST_MOV){
        if (ast->data.opexpr.left != NULL){
            reviser_eat_lhs(reviser, ast->data.opexpr.left);
        }
        ast->typeinfo = reviser_get_lhs_size(*ast);
        if (ast->typeinfo == 0){
            ast->typeinfo = ast->data.opexpr.right->typeinfo;
        }
        reviser_eat_expr(reviser, ast->data.opexpr.right);
    }else if(ast->type == AST_SYSCALL){
        char *name = strdup(ast->data.text.value);
        char *res = "";
        if (string_compare(name, "write", strlen(name)) == 0){
            switch (target){
                case TARGET_MACOS: res = "33554436"; break;
                case TARGET_LINUX: res = "1"; break;
                default: break;
            }
        }else if (string_compare(name, "exit", strlen(name)) == 0){
            switch (target){
                case TARGET_MACOS: res = "33554433"; break;
                case TARGET_LINUX: res = "60"; break;
                default: break;
            }
        }else if (string_compare(name, "mmap", strlen(name)) == 0){
            switch (target){
                case TARGET_MACOS: res = "33554629"; break;
                case TARGET_LINUX: res = "9"; break;
                default: break;
            }
        } else if (string_compare(name, "read", strlen(name)) == 0) {
            switch (target) {
                case TARGET_MACOS: res = "33554435"; break;
                case TARGET_LINUX: res = "0"; break;
                default: break;
            }
        } else if (string_compare(name, "open", strlen(name)) == 0) {
            switch (target) {
                case TARGET_MACOS: res = "33554437"; break;
                case TARGET_LINUX: res = "2"; break;
                default: break;
            }
        } else if (string_compare(name, "close", strlen(name)) == 0) {
            switch (target) {
                case TARGET_MACOS: res = "33554438"; break;
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
    }else if(ast->type == AST_CALLIF){
        reviser_eat_expr(reviser, ast->data.jmpif.reg);
    }else if(ast->type == AST_CALLIFN){
        reviser_eat_expr(reviser, ast->data.jmpif.reg);
    }else if(ast->type == AST_ADD){
        reviser_eat_expr(reviser, ast->data.operation.right);
    }else if(ast->type == AST_SUB){
        reviser_eat_expr(reviser, ast->data.operation.right);
    }else if(ast->type == AST_MUL){
        reviser_eat_expr(reviser, ast->data.operation.right);
    }else if(ast->type == AST_DIV){
        reviser_eat_expr(reviser, ast->data.operation.right);
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
