typedef struct {
    char *name;
    AST_TypeInfo type;
    bool isArg;
    AST *value;
}TC_Variable;

typedef struct {
    int argn;
    AST_TypeInfo type;
}TC_Arg;

typedef struct {
    TC_Variable *variables;
    int variablelen;
}TC_Scope;

typedef struct {
    char *name;
    AST_TypeInfo ret;
    AST_TypeInfo *args;
    int arglen;
    TC_Scope scope;
}TC_Func;

typedef struct {
    char *name;
    AST *asts;
    int astlen;
    int cur;
    TC_Func *functions;
    int functionlen;
}Typechecker;

typedef struct {
    bool variable_alias;
    bool constant_folding;
    bool inline_ifs;
}Mode;

TC_Scope *current_scope = NULL;
TC_Scope *global_scope;
Mode *current_mode;

typedef struct {
    char *name;
    AST_TypeInfo type;
    int offset;
} TC_Field;

typedef struct {
    char *name;
    TC_Field *fields;
    int field_count;
    int size;
} TC_Struct;

TC_Struct structs[100];
int struct_count = 0;

TC_Struct find_struct(char *name){
    if (strncmp(name, "struct", strlen("struct")) == 0){
        name += strlen("struct");
    };
    for (int i=0; i<struct_count; i++){
        if (strcmp(structs[i].name, name) == 0){
            return structs[i];
        }
    }
    return (TC_Struct){0};
};

TC_Field find_field(TC_Struct struct1, char *name){
    for (int i=0; i<struct1.field_count; i++){
        if (strcmp(struct1.fields[i].name, name) == 0){
            return struct1.fields[i];
        };
    };
    return (TC_Field){0};
};

int is_expression(AST ast){
    switch (ast.type){
        case AST_PLUS: return 1;
        case AST_SUB: return 1;
        case AST_MUL: return 1;
        case AST_DIV: return 1;
        case AST_MODULO: return 1;
        case AST_EQ: return 1;
        case AST_NEQ: return 1;
        case AST_LT: return 1;
        case AST_LTE: return 1;
        case AST_GT: return 1;
        case AST_GTE: return 1;
        default: return 0;
    };
    return 0;
}

void optimize_speed(){
    if (current_mode == NULL){
        current_mode = malloc(sizeof(Mode));
    }
    current_mode->variable_alias = true;
    current_mode->constant_folding = true;
    current_mode->inline_ifs = true;
};


TC_Scope initialize_scope(){
    TC_Scope scope;
    scope.variablelen = 0;
    scope.variables = malloc(sizeof(TC_Variable) * 100);
    return scope;
};

void add_variable_in_current_scope(TC_Variable variable){
   current_scope->variables[current_scope->variablelen++] = variable;
}

void add_variable_in_global_scope(TC_Variable variable){
   global_scope->variables[global_scope->variablelen++] = variable;
}

AST_TypeInfo fetch_type(Typechecker *typechecker, AST *_ast);
TC_Variable *find_variable_in_scopes(Typechecker *typechecker, AST *_ast, int *p){
    AST ast = *_ast;
    if (ast.type == AST_DEREF || ast.type == AST_INDEX){
        TC_Variable *var = find_variable_in_scopes(typechecker, ast.data.expr.left, NULL);
        if (var != NULL){
            if (strcmp(var->name, "")){
                TC_Variable *copy = malloc(sizeof(TC_Variable));
                *copy = *var;
                copy->type = fetch_type(typechecker, _ast);
                return copy;
            };
        };
        var = find_variable_in_scopes(typechecker, ast.data.expr.right, NULL);
        if (var != NULL){
            if (strcmp(var->name, "")){
                TC_Variable *copy = malloc(sizeof(TC_Variable));
                *copy = *var;
                copy->type = fetch_type(typechecker, _ast);
                return copy;
            };
        }
        TC_Variable *variable = malloc(sizeof(TC_Variable));
        variable->name = "";
        variable->type = fetch_type(typechecker, _ast);
        return variable;
    };
    for (int i=0; i<current_scope->variablelen; i++){
        if (strcmp(current_scope->variables[i].name, ast.data.arg.value) == 0){
            if (p != NULL){
                *p = 1;
            }
            return current_scope->variables + i;
        };
    };
    for (int i=0; i<global_scope->variablelen; i++){
        if (strcmp(global_scope->variables[i].name, ast.data.arg.value) == 0){
            if (p != NULL){
                *p = 2;
            }
            return global_scope->variables + i;
        };
    };
    if (ast.type != AST_VAR) return NULL;
    char string[100];
    snprintf(string, 100, "Variable \"%s\" doesn't exist", ast.data.arg.value);
    error_generate_parser("VarError", string, ast.row, ast.col, ast.filename);
    return NULL;
}


Typechecker *typechecker_init(Parser *parser){
    Typechecker *typechecker = malloc(sizeof(Typechecker));
    typechecker->asts = parser->asts;
    typechecker->astlen = parser->astlen;
    typechecker->name = parser->name;
    typechecker->cur = 0;
    typechecker->functions = malloc(sizeof(TC_Func) * 100);
    typechecker->functions[0].name = "print";
    typechecker->functions[0].ret = (AST_TypeInfo){.ptrnum=0, .type="int"};
    typechecker->functions[0].args = malloc(sizeof(AST_TypeInfo) * 100);
    typechecker->functions[0].args[0] = (AST_TypeInfo){.ptrnum=1, .type="char"};
    typechecker->functions[0].arglen = 1;
    typechecker->functions[0].scope = initialize_scope();
    typechecker->functionlen = 1;
    TC_Scope scope = initialize_scope();
    global_scope = malloc(sizeof(TC_Scope));
    *global_scope = scope;
    current_mode = malloc(sizeof(Mode));
    // optimize_speed();
    return typechecker;
};

char *externs[100];
int externlen = 0;


bool are_equal(AST_TypeInfo *left, AST_TypeInfo *right){
    char *l = typeinfo_to_string(*left);
    char *r = typeinfo_to_string(*right);
    if (strcmp(l, r)) {
        if (strcmp(l, "const") == 0){
            *left = *right;
        }else if (strcmp(r, "const") == 0){
            *right = *left;
        }else {
            return false;
        }
    };
    return true;
};


void typechecker_eat(Typechecker *typechecker, AST *ast);
AST_TypeInfo fetch_type(Typechecker *typechecker, AST *_ast){
    AST ast = *_ast;
    
    if (ast.type == AST_RET){
        return fetch_type(typechecker, (ast.data.ret.ret));
    }else if(ast.type == AST_VAR){
        return find_variable_in_scopes(typechecker, _ast, NULL)->type;
    }else if(ast.type == AST_FUNCALL){
        TC_Func func = (TC_Func){};
        _Bool a = 0;
        for (int i=0; i<typechecker->functionlen; i++){
            if (strcmp(ast.data.funcall.name, typechecker->functions[i].name) == 0){
                func = typechecker->functions[i];
                a = 1;
            };
        };
        for (int i=0; i<externlen; i++){
            if (strcmp(externs[i], ast.data.funcall.name) == 0){
                return (AST_TypeInfo){"int", 0};
            };
        }

        if (a == 0){
            char string[100];
            snprintf(string, 100, "Function \"%s\" doesn't exist", ast.data.funcall.name);
            error_generate_parser("FuncError", string, ast.row, ast.col, ast.filename);
        };
        return func.ret;
    }else if(ast.type == AST_PLUS || ast.type == AST_SUB || ast.type == AST_MUL || ast.type == AST_DIV || ast.type == AST_GT || ast.type == AST_GTE || ast.type == AST_LT || ast.type == AST_LTE || ast.type == AST_EQ || ast.type == AST_NEQ || ast.type == AST_MODULO){

        char *a = "";
        switch (ast.type){
            case AST_PLUS: a = "addition operation"; break;
            case AST_SUB: a = "subtraction operation"; break;
            case AST_MUL: a = "multiplication operation"; break;
            case AST_DIV: a = "division operation"; break;
            case AST_LT: a = "less than comparison"; break;
            case AST_GT: a = "greater than comparison"; break;
            case AST_LTE: a = "less than or equal to comparison"; break;
            case AST_GTE: a = "greater than or equal to comparison"; break;
            case AST_EQ: a = "equal to comparison"; break;
            case AST_NEQ: a = "is not equal to comparison"; break;
            case AST_MODULO: a = "modulus operation"; break;


            default: a = "unknown expression";
        }


        AST_TypeInfo left = fetch_type(typechecker, (ast.data.expr.left));
        AST_TypeInfo right = fetch_type(typechecker, (ast.data.expr.right));
        if (!are_equal(&left, &right)) {
            if (!((left.ptrnum > 0 && right.ptrnum == 0) || (right.ptrnum > 0 && left.ptrnum == 0) && (ast.type == AST_PLUS || ast.type == AST_SUB))){ // Allow ptr+num and ptr-num
                char string[100];
                snprintf(string, 100, "Inequal types of \"%s\" and \"%s\" inside of %s", typeinfo_to_string(left), typeinfo_to_string(right), a);
                error_generate_parser("VarError", string, ast.data.expr.left->row, ast.data.expr.left->col, ast.data.expr.left->filename);
            }
        };
        if (ast.type == AST_GT || ast.type == AST_GTE || ast.type == AST_LT || ast.type == AST_LTE || ast.type == AST_EQ || ast.type == AST_NEQ){
            ast.typeinfo = (AST_TypeInfo){"bool", 0};
            return ast.typeinfo;
        };
        ast.typeinfo = left;
        return left;
    }else if(ast.type == AST_EXPR){
        ast.typeinfo = fetch_type(typechecker, ast.data.expr.left);
        return ast.typeinfo;
    }else if(ast.type == AST_DEREF){
        AST_TypeInfo type = fetch_type(typechecker, ast.data.expr.left);
        if (type.ptrnum == 0){
            error_generate_parser("DerefError", "Cannot deref something that's not a pointer", ast.row, ast.col, ast.filename);
        }else {
            type.ptrnum--;
        };
        return type;
    }else if(ast.type == AST_ACCESS){
        AST_TypeInfo base_type = fetch_type(typechecker, ast.data.expr.left);
        TC_Struct struct1 = find_struct(base_type.type);
        TC_Field field = find_field(struct1, ast.data.expr.right->data.arg.value);
        return field.type;
    }else if(ast.type == AST_IF || ast.type == AST_WHILE){
        return (AST_TypeInfo){"bool", 0};
    }else if(ast.type == AST_NOT){
        return (AST_TypeInfo){"bool", 0};
    }else if(ast.type == AST_AND){
        return (AST_TypeInfo){"bool", 0};
    }else if(ast.type == AST_OR){
        return (AST_TypeInfo){"bool", 0};
    }else if(ast.type == AST_SYSCALL){
        return (AST_TypeInfo){"int", 0};
    }else if(ast.type == AST_INDEX){
        typechecker_eat(typechecker, ast.data.expr.left);
        typechecker_eat(typechecker, ast.data.expr.right);
        AST_TypeInfo typeinfo = fetch_type(typechecker, ast.data.expr.left);
        if (typeinfo.ptrnum == 0){
            error_generate_parser("ArraySubscriptError", "Cannot get array susbscript of something that's not an array / pointer", ast.row, ast.col, ast.filename);
        };
        typeinfo.ptrnum--;
        return typeinfo;
    };

    if (ast.typeinfo.type){
        if (strcmp(ast.typeinfo.type, "") != 0){
            return ast.typeinfo;
        }
    };
    return (AST_TypeInfo){"", 0};
};


int typeinfo_to_len(AST_TypeInfo type){
    if (type.ptrnum > 0){
        return 8;
    }
    if (type.type == NULL){
        return 0;
    }
    for (int v=0; v<typesLen; v++){
        if (types[v].name && strcmp(types[v].name, type.type) == 0){
            return types[v].length;
        }
    }
    return 0;
}

void typechecker_eat(Typechecker *typechecker, AST *ast);
void typechecker_access(Typechecker *typechecker, AST *ast){
    if (ast->type == AST_CAST && ast->data.expr.left && ast->data.expr.left->type == AST_INDEX) {
        return; 
    }

    typechecker_eat(typechecker, ast->data.expr.left);
    
    TC_Struct struct1 = find_struct(ast->data.expr.left->typeinfo.type);
    TC_Field field = find_field(struct1, ast->data.expr.right->data.arg.value);

    AST *original_base = ast->data.expr.left; 

    #define NEW_NODE() calloc(1, sizeof(AST))

    AST *cast_ptr = NEW_NODE();
    cast_ptr->type = AST_CAST;
    cast_ptr->typeinfo = (AST_TypeInfo){"char", 1};
    cast_ptr->data.expr.left = original_base;

    AST *off_node = NEW_NODE();
    off_node->type = AST_INT;
    off_node->typeinfo = (AST_TypeInfo){"int", 0};
    char off_str[10];
    snprintf(off_str, 10, "%d", field.offset);
    off_node->data.arg.value = strdup(off_str);

    AST *plus_node = NEW_NODE();
    plus_node->type = AST_PLUS;
    plus_node->typeinfo = (AST_TypeInfo){"char", 1};
    plus_node->data.expr.left = cast_ptr;
    plus_node->data.expr.right = off_node;
    AST *index_node = NEW_NODE();
    index_node->type = AST_INDEX;
    index_node->typeinfo = field.type;
    index_node->data.expr.left = plus_node;
    index_node->data.expr.right = NEW_NODE();
    index_node->data.expr.right->type = AST_INT;
    index_node->data.expr.right->data.arg.value = strdup("0");

    ast->type = AST_CAST;
    ast->typeinfo = field.type;
    ast->data.expr.left = index_node;
}

void typechecker_eat_lhs(Typechecker *typechecker, AST *ast){
    if (ast->type == AST_VAR){
        TC_Variable *var = find_variable_in_scopes(typechecker, ast, NULL);
        if (var->value != NULL){
            var->value->data.assign.alias = false;
        }
    }else if(ast->type == AST_ACCESS){
        typechecker_access(typechecker, ast);
    }
};

AST *aliases(AST *var){
    if (var == NULL) return NULL;
    if (var->type == AST_VAR){
        AST *assign = var->data.optvar.opt;
        if (assign != NULL){
            if (assign->data.assign.alias){
                return aliases(assign->data.assign.assignto);
            }
        }
    }else if(var->type == AST_EXPR){
        return aliases(var->data.expr.left);
    }else if(var->type == AST_CAST){
        return aliases(var->data.expr.left);
    }
    return var;
}




void typechecker_eat(Typechecker *typechecker, AST *ast){
    ast->typeinfo = fetch_type(typechecker, ast);
    if (ast->type == AST_FUNCDEF){
        AST_TypeInfo expected = ast->typeinfo;
        typechecker->functions[typechecker->functionlen].name = ast->data.funcdef.name;
        typechecker->functions[typechecker->functionlen].args = malloc(sizeof(AST_TypeInfo) * 100);
        typechecker->functions[typechecker->functionlen].arglen = ast->data.funcdef.argslen;
        typechecker->functions[typechecker->functionlen].ret = ast->typeinfo;
        typechecker->functions[typechecker->functionlen].scope = initialize_scope();
        if (current_scope != NULL) free(current_scope);
        current_scope = malloc(sizeof(TC_Scope));
        *current_scope = typechecker->functions[typechecker->functionlen].scope;
        for (int i=0; i<ast->data.funcdef.argslen; i++){
            add_variable_in_current_scope((TC_Variable){ast->data.funcdef.args[i]->arg, ast->data.funcdef.args[i]->type, true, NULL});
            typechecker->functions[typechecker->functionlen].args[i] = ast->data.funcdef.args[i]->type;
        };
        typechecker->functionlen++;

        // Check if expected type is valid
        if (!(expected.type)){
            goto error;
        }else if(strcmp(expected.type, "") == 0){
            goto error;
        }


        for (int i=0; i<ast->data.funcdef.blocklen; i++){
            typechecker_eat(typechecker, ast->data.funcdef.block[i]);
        };


        AST_TypeInfo found = (AST_TypeInfo){"", 0};
        int a = 0;
        for (int i=0; i<ast->data.funcdef.blocklen; i++){
            if (ast->data.funcdef.block[i]->type == AST_RET){
                found = fetch_type(typechecker, (ast->data.funcdef.block[i]));
                a = 1;
                if(!(found.type)){
                    goto error;
                }else if(strcmp(found.type, "") == 0){
                    goto error;
                }else if(strcmp(typeinfo_to_string(expected), "const") == 0 || strcmp(typeinfo_to_string(found), "const") == 0){
                    ;
                }else if(strcmp(typeinfo_to_string(expected), typeinfo_to_string(found))){
                    char string[100];
                    snprintf(string, 100, "Function expects a return type of %s, but return is found returning %s", typeinfo_to_string(expected), typeinfo_to_string(found));
                    error_generate_parser("ReturnError", string, ast->row, ast->col, ast->filename);
                };
            };
        };
        if (a == 0 && ast->data.funcdef.blocklen != -1){
            char string[100];
            snprintf(string, 100, "No return found in function \"%s\" that doesn't return void", ast->data.funcdef.name);
            error_generate_parser("ReturnError", string, ast->row, ast->col, ast->filename);
        };
    }else if(ast->type == AST_INDEX){
        typechecker_eat_lhs(typechecker, ast->data.expr.left);
        if (strcmp(typeinfo_to_string(ast->data.expr.left->typeinfo), "char*") == 0){
            ast->type = AST_DEREF;
            AST *astleft = ast->data.expr.left;
            AST *astright = ast->data.expr.right;
            ast->data.expr.left = malloc(sizeof(AST));
            ast->data.expr.left->type = AST_PLUS;
            ast->data.expr.left->data.expr.left = astleft;
            ast->data.expr.left->data.expr.right = astright;
            typechecker_eat(typechecker, ast);
        }
        return;
    }else if(ast->type == AST_ASSIGN){
        bool is_block = typechecker->asts[typechecker->cur].type == AST_ASSIGN;
        if (!current_mode->variable_alias){
            ast->data.assign.alias = false;
        }
        AST_TypeInfo expected = ast->typeinfo;
        if (strcmp(typeinfo_to_string(expected), "unknown") == 0){
            char *name = ast->data.assign.from->data.arg.value;
            int *p = malloc(sizeof(int));
            TC_Variable *var = find_variable_in_scopes(typechecker, ast->data.assign.from, p);
            if (var != NULL){
                if (var->value != NULL){
                    var->value->data.assign.alias = false;
                }
            }
            expected = fetch_type(typechecker, ast->data.assign.from);
            if (*p == 2 && is_block){ // Global scope
                error_generate_parser("GlobalVarError", "Global variables cannot be re-assigned.", ast->data.assign.from->row, ast->data.assign.from->col, ast->data.assign.from->filename);
            }
            ast->typeinfo = expected;
        }else if(is_block){
            if (ast->data.assign.from->type != AST_VAR){
                error_generate_parser("GlobalVarError", "Global variables cannot be assined to anything other than a variable.", ast->data.assign.from->row, ast->data.assign.from->col, ast->data.assign.from->filename);
            };
            if (!is_immediate(ast->data.assign.assignto)){
                error_generate_parser("GlobalVarError", "Global variables cannot be set to a non-constant value.", ast->data.assign.assignto->row, ast->data.assign.assignto->col, ast->data.assign.assignto->filename);
            };
            add_variable_in_global_scope((TC_Variable){ast->data.assign.from->data.arg.value, expected, false, ast});
        }else {
            add_variable_in_current_scope((TC_Variable){ast->data.assign.from->data.arg.value, expected, false, ast});
        }
        typechecker_eat(typechecker, ast->data.assign.from);
        _Bool a = 1;

        if (!(expected.type)){
            a = 0;
        }else if(strcmp(expected.type, "") == 0){
            a = 0;
        }

        AST_TypeInfo found = fetch_type(typechecker, (ast->data.assign.assignto));
        typechecker_eat(typechecker, ast->data.assign.assignto);
        if (ast->data.assign.assignto->type != AST_UNKNOWN && strcmp(typeinfo_to_string(found), "const")){
            if (strcmp(typeinfo_to_string(expected), typeinfo_to_string(found)) != 0){
                char string[100];
                snprintf(string, 100, "Variable is supposed to be of type %s but is assigned to object of type %s", typeinfo_to_string(expected), typeinfo_to_string(found));
                error_generate_parser("VarError", string, ast->row, ast->col, ast->filename);
            };
        }
    }else if(ast->type == AST_FUNCALL){
        TC_Func func = (TC_Func){};
        int a = 0;
        for (int i=0; i<typechecker->functionlen; i++){
            if (strcmp(ast->data.funcall.name, typechecker->functions[i].name) == 0){
                func = typechecker->functions[i];
                a = 1;
            };
        };
        for (int i=0; i<externlen; i++){
            if (strcmp(externs[i], ast->data.funcall.name) == 0){
                char new[100];
                snprintf(new, 100, "_%s", ast->data.funcall.name);
                ast->data.funcall.name = strdup(new);
                a = 2;
            };
        }
        if (a == 0){
            char string[100];
            snprintf(string, 100, "Function %s doesn't exist", ast->data.funcall.name);
            error_generate_parser("FuncError", string, ast->row, ast->col, ast->filename);
        };
        if (a != 2){
            if (ast->data.funcall.argslen != func.arglen){
                char string[100];
                snprintf(string, 100, "Too many or too little arguments sent to function %s", ast->data.funcall.name);
                error_generate_parser("FuncError", string, ast->row, ast->col, ast->filename);
            };
            for (int i=0; i<ast->data.funcall.argslen; i++){
                AST *arg = ast->data.funcall.args[i];
                typechecker_eat(typechecker, arg);
                AST_TypeInfo left = fetch_type(typechecker, arg);
                AST_TypeInfo right = func.args[i];
                if (!are_equal(&left, &right)) {
                    char string[100];
                    snprintf(string, 100, "In function \"%s\", argument type \"%s\" is expected, but argument of type \"%s\" is given", ast->data.funcall.name, typeinfo_to_string(func.args[i]), typeinfo_to_string(fetch_type(typechecker, arg)));
                    error_generate_parser("ArgError", string, ast->row, ast->col, ast->filename);
                };
            };
        }
    }else if(ast->type == AST_SYSCALL){
        for (int i=0; i<ast->data.funcall.argslen; i++){
            AST *arg = ast->data.funcall.args[i];
            typechecker_eat(typechecker, arg);
        };
    }else if(ast->type == AST_RET){
        typechecker_eat(typechecker, ast->data.ret.ret);
    }else if(ast->type == AST_PLUS || ast->type == AST_SUB || ast->type == AST_MUL || ast->type == AST_DIV || ast->type == AST_MODULO){
        typechecker_eat(typechecker, ast->data.expr.left);
        typechecker_eat(typechecker, ast->data.expr.right);
        AST *left = aliases(ast->data.expr.left);
        AST *right = aliases(ast->data.expr.right);
        if (left->type == AST_INT && right->type == AST_INT && current_mode->constant_folding){
            char string[100];
            int res = atoi(left->data.arg.value);
            int rhs = atoi(right->data.arg.value);
            switch (ast->type){
                case AST_PLUS: res += rhs; break;
                case AST_SUB: res -= rhs; break;
                case AST_MUL: res *= rhs; break;
                case AST_DIV: res /= rhs; break;
                case AST_MODULO: res %= rhs; break;
                default: res = 0;
            };
            snprintf(string, 100, "%d", res);
            ast->type = AST_INT;
            ast->data.arg.value = strdup(string);
        };

        ast->typeinfo = fetch_type(typechecker, ast);
        return;
    }else if (ast->type == AST_GT || ast->type == AST_GTE || ast->type == AST_LT || ast->type == AST_LTE || ast->type == AST_EQ || ast->type == AST_NEQ){
        typechecker_eat(typechecker, ast->data.expr.left);
        typechecker_eat(typechecker, ast->data.expr.right);
        AST *left = aliases(ast->data.expr.left);
        AST *right = aliases(ast->data.expr.right);
        if (left->type == AST_INT && right->type == AST_INT && current_mode->constant_folding){
            char string[100];
            int res = atoi(left->data.arg.value);
            int rhs = atoi(right->data.arg.value);
            switch (ast->type){
                case AST_GT: res = res < rhs; break;
                case AST_GTE: res = res <= rhs; break;
                case AST_LT: res = res > rhs; break;
                case AST_LTE: res = res >= rhs; break;
                case AST_EQ: res = res == rhs; break;
                case AST_NEQ: res = res != rhs; break;
                default: res = 0;
            };
            snprintf(string, 100, "%d", res);
            ast->type = AST_INT;
            ast->data.arg.value = strdup(string);
            ast->typeinfo = (AST_TypeInfo){"int", 0};
        }else {
            ast->typeinfo = (AST_TypeInfo){"bool", 0};
        }
        return;
    }else if(ast->type == AST_CAST){
        typechecker_eat(typechecker, ast->data.expr.left);
    }else if(ast->type == AST_DEREF){
        typechecker_eat(typechecker, ast->data.expr.left);
    }else if(ast->type == AST_IF){
        AST_TypeInfo type = fetch_type(typechecker, ast->data.if1.block.condition);
        typechecker_eat(typechecker, ast->data.if1.block.condition);
        AST *cond = aliases(ast->data.if1.block.condition);
        if (cond != NULL && current_mode->inline_ifs){
            if (cond->type == AST_INT){
                int true1 = atoi(cond->data.arg.value) >= 1;
                if (true1) {
                    AST *block_start = ast->data.if1.block.statements;
                    AST *original_next = ast->next;

                    if (block_start != NULL) {
                        AST *last = block_start;
                        while (last->next != NULL) {
                            last = last->next;
                        }
                        last->next = original_next;
                        *ast = *block_start; 
                    } else {
                        ast->type = AST_UNKNOWN; 
                        ast->next = original_next;
                    }
                    
                    return;
                }
            }
        }
        AST *statement = ast->data.if1.block.statements;
        for (int i=0; i<ast->data.if1.block.statementlen; i++){
            typechecker_eat(typechecker, statement);
            statement = statement->next;
        };
        for (int j = 0; j < ast->data.if1.elseiflen; j++) {
            AST *statement = ast->data.if1.elseif[j].statements;
            for (int i=0; i < ast->data.if1.elseif[j].statementlen; i++){
                typechecker_eat(typechecker, statement);
                statement = statement->next;
            }
        };
        statement = ast->data.if1.else1;
        for (int i=0; i<ast->data.if1.elselen; i++){
            typechecker_eat(typechecker, statement);
            statement = statement->next;
        };
    }else if(ast->type == AST_WHILE){
        AST_TypeInfo type = fetch_type(typechecker, ast->data.while1.condition);
        typechecker_eat(typechecker, ast->data.while1.condition);
        if (strcmp(type.type, "bool") || type.ptrnum != 0){
        };
        for (int i=0; i<ast->data.while1.blocklen; i++){
            typechecker_eat(typechecker, ast->data.while1.block[i]);
        };
    }else if(ast->type == AST_EXPR){
        typechecker_eat(typechecker, ast->data.expr.left);
        ast->typeinfo = ast->data.expr.left->typeinfo;
    }else if(ast->type == AST_NOT){
        ast->typeinfo = (AST_TypeInfo){"bool", 0};
    }else if(ast->type == AST_VAR){
        TC_Variable *var = find_variable_in_scopes(typechecker, ast, NULL);
        if (var->value != NULL && current_mode->variable_alias){
            ast->data.optvar.opt = var->value;
            // *ast = *var->value->data.assign.assignto;
        };
        return;
    }else if(ast->type == AST_AND){
        ast->typeinfo = (AST_TypeInfo){"bool", 0};
        typechecker_eat(typechecker, ast->data.expr.left);
        typechecker_eat(typechecker, ast->data.expr.right);
        char *type1 = typeinfo_to_string(ast->data.expr.left->typeinfo);
        char *type2 = typeinfo_to_string(ast->data.expr.right->typeinfo);
        if (strcmp(type1, "bool") || strcmp(type2, "bool")) {
            char a[1000];
            snprintf(a, 1000, "Expected boolean types, found types \"%s\" and \"%s\"", type1, type2);
            error_generate_parser("AndError", a, ast->row, ast->col, ast->filename);
        }
    }else if(ast->type == AST_OR){
        ast->typeinfo = (AST_TypeInfo){"bool", 0};
        typechecker_eat(typechecker, ast->data.expr.left);
        typechecker_eat(typechecker, ast->data.expr.right);
        char *type1 = typeinfo_to_string(ast->data.expr.left->typeinfo);
        char *type2 = typeinfo_to_string(ast->data.expr.right->typeinfo);
        if (strcmp(type1, "bool") || strcmp(type2, "bool")) {
            char a[1000];
            snprintf(a, 1000, "Expected boolean types, found types \"%s\" and \"%s\"", type1, type2);
            error_generate_parser("AndError", a, ast->row, ast->col, ast->filename);
        }
    }else if(ast->type == AST_MODE){
        if (strcmp(ast->data.mode.name, "extern") == 0){
            externs[externlen++] = ast->data.mode.res;
        }else if (strcmp(ast->data.mode.name, "optimize") == 0){
            optimize_speed();
        };
    }else if(ast->type == AST_STRUCT){
        TC_Struct *s = &structs[struct_count++];
        s->name = ast->data.struct1.name;
        s->field_count = ast->data.struct1.fieldlen;
        s->fields = malloc(sizeof(TC_Field) * s->field_count);
        
        int offset = 0;
        for (int i = 0; i < s->field_count; i++) {
            Field *field = ast->data.struct1.fields[i];
            s->fields[i].name = field->name;
            s->fields[i].type = field->type;
            s->fields[i].offset = offset;
            
            int field_size = typeinfo_to_len(field->type);
            offset += field_size;
        }
        s->size = offset;
        char string[100];
        snprintf(string, 100, "struct%s", ast->data.struct1.name);
        types[typesLen++] = (struct Pair){strdup(string), s->size, "structure"};
    }else if(ast->type == AST_ACCESS){
        typechecker_eat(typechecker, ast->data.expr.left);
        if (ast->data.expr.left->type == AST_VAR){
            if (ast->data.expr.left->data.optvar.opt != NULL){
                ast->data.expr.left->data.optvar.opt->data.assign.alias = false;
            }
        }
        typechecker_access(typechecker, ast);
    }
    return;
error:
    error_generate_parser("TypeError", "Type doesn't exist", ast->row, ast->col, ast->filename);
    return;
};



AST *load_ast(Typechecker *typechecker){
    if (typechecker->cur == 0){
        return typechecker->asts;
    }else {
        AST *ast1 = typechecker->asts;
        for (int i=0; i<typechecker->cur; i++){
            ast1 = ast1->next;
        };
        return ast1;
    };
}

int typechecker_eat_ast(Typechecker *typechecker){
    if (typechecker->astlen == typechecker->cur){
        return -1;
    };
    typechecker_eat(typechecker, load_ast(typechecker));
    typechecker->cur++;
    return 0;
};

void typechecker_close(Typechecker *typechecker){
    free(current_mode);
    free(current_scope);
    free(global_scope);
};
