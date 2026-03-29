/* Registers
 * --- VALUES ---
 *  v0 = Return value
 * --- FUNCTION ARGUMENTS ---
 *  a0 = 1st Argument
 *  a1 = 2nd Argument
 *  a2 = 3rd Argument
 *  a3 = 4th Argument
 *  a4 = 5th Argument
 *  a5 = 6th Argument
 * --- TEMPORARIES ---
 *  t0 = Temporary Register
 *  t1 = Temporary Register
 *  t2 = Temporary Register
 *  t3 = Temporary Register
 *  t4 = Temporary Register
 *  t5 = Temporary Register
 *  t6 = Temporary Register
 *  t7 = Temporary Register
 *  t8 = Temporary Register
 *  t9 = Temporary Register
 *  t10 = Temporary Register
 *  t11 = Temporary Register
 *  t12 = Temporary Register
 *  t13 = Temporary Register
 *  t14 = Temporary Register
 *  t15 = Temporary Register
 * --- PRESERVED ---
 *  s0 = Saved Register
 *  s1 = Saved Register
 *  s2 = Saved Register
 *  s3 = Saved Register
 *  s4 = Saved Register
 *  s5 = Saved Register
 *  s6 = Saved Register
*/


#define REGISTERS 32
char *regs[REGISTERS] = {
    "v0", "s0", "a3", "a2", "a1", "a0", "a4", "a5", "t0", "__", "t1", "__", "s1", "t2",
    "t3", "t4", "t5", "t6",
    "t7", "t8", "t9", "t10", "t11", "t12", "t13", "t14", "t15",
    "s2", "s3", "s4", "s5", "s6"
};
static_assert((sizeof(regs) / sizeof(regs[0])) == REGISTERS, "Too many registers");


int is_reg(char *reg){
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, regs[i]) == 0) {
            return 0;
        };
    }
    return -1;
}

int name_to_size(char *name){
    int len = strlen(name);
    if (string_compare(name, "i8", len > 2 ? len : 2) == 0){
        return 1;
    }else if (string_compare(name, "i16", len > 3 ? len : 3) == 0){
        return 2;
    }else if (string_compare(name, "i32", len > 3 ? len : 3) == 0){
        return 4;
    }else if (string_compare(name, "i64", len > 3 ? len : 3) == 0){
        return 8;
    };
    return 0;
};

char *size_to_name(int size){
    switch (size){
        case 1: return "byte";
        case 2: return "word";
        case 4: return "dword";
        case 8: return "qword";
    };
    return 0;
};
AST_Reg _reg_to_num(char *reg, int size){
    AST_Reg reg1 = (AST_Reg){0};
    reg1.reg = -1;
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, regs[i]) == 0){
            reg1.reg = i;
        };
    }
    if (size != 0){
        reg1.size = size;
    }else {
        reg1.size = 8;
    };
    return reg1;
};


void append_ast_to_list(AST **head, AST *new_node, int *count) {
    new_node->next = NULL;
    if (*head == NULL) {
        *head = new_node;
        *count = 1;
    } else {
        AST *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        (*count)++;
    }
}

void store_ast(Parser *parser, AST *ast){
    Token token = parser->tokens[parser->cur];
    ast->row = token.row;
    ast->col = token.col;
    ast->next = NULL;
    
    if (parser->astlen == 0){
        parser->asts = ast;
    }else {
        AST *ast1 = parser->asts;
        for (int i=0; i<parser->astlen-1; i++){
            ast1 = ast1->next;
        };
        ast1->next = ast;
    };
    parser->astlen++;
}


Parser *parser_init(Tokenizer *tokenizer){
    Parser *parser = malloc(sizeof(Parser));
    parser->name = tokenizer->name;
    parser->asts = malloc(sizeof(AST));
    parser->astlen = 0;
    parser->cur = 0;
    parser->tokens = tokenizer->tokens;
    parser->tokens[parser->cur].name = tokenizer->name;
    parser->tokenlen = tokenizer->tokenlen;
    parser->code = tokenizer->code;
    return parser;
};


void parser_peek(Parser *parser){
    parser->cur++;
    if (parser->cur > parser->tokenlen){
        error_generate_parser(parser, "AbruptEndError", "Abrupt end", parser->tokens[parser->tokenlen-1].row, parser->tokens[parser->tokenlen-1].col, parser->tokens[parser->cur].name);
    };
};

void parser_back(Parser *parser){
    parser->cur--;
    if (parser->cur < 0){
        error_generate_parser(parser, "AbruptEndError", "Abrupt end", parser->tokens[parser->tokenlen-1].row, parser->tokens[parser->tokenlen-1].col, parser->tokens[parser->cur].name);
    };
};

void parser_expect(Parser *parser, int type){
    if (parser->tokens[parser->cur].type != type){
        char error[100];
        snprintf(error, 100, "Expected %s, got %s", token_to_string(type), token_to_string(parser->tokens[parser->cur].type));
        error_generate_parser(parser, "ExpectError", error, parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
    };
    if (parser->cur != parser->tokenlen){
        parser_peek(parser);
    }else {
    }
};
AST_Reg reg_to_num(Parser *parser){
    Token cur = parser->tokens[parser->cur];
    char *reg = cur.value;
    AST_Reg reg1 = _reg_to_num(reg, 0);
    parser_peek(parser);
    cur = parser->tokens[parser->cur];
    if (cur.type == TOKEN_COLON){
        parser_peek(parser);
        cur = parser->tokens[parser->cur];
        parser_expect(parser, TOKEN_ID);
        reg1.size = name_to_size(cur.value);
    }else {
        reg1.size = 8;
    };

    if (reg1.reg == -1){
        char string[100];
        snprintf(string, 100, "Register '%s' doesn't exist", reg);
        error_generate("InvalidRegisterError", string);
    }
    return reg1;
};
#define ret(ast) ast->row = orig.row; ast->col = orig.col; return ast
char *parser_label(Parser *parser){
    Token token = parser->tokens[parser->cur];
    if (string_compare(token.value, "label", strlen(token.value)) == 0){
        parser_peek(parser);
        char string[100];
        token = parser->tokens[parser->cur];
        snprintf(string, 100, ".%s", token.value);
        parser_peek(parser);
        return strdup(string);
    }else if (string_compare(token.value, "labelend", strlen(token.value)) == 0){
        parser_peek(parser);
        char string[100];
        token = parser->tokens[parser->cur];
        snprintf(string, 100, ".%s_end", token.value);
        parser_peek(parser);
        return strdup(string);
    }else {
        token = parser->tokens[parser->cur];
        parser_peek(parser);
        return strdup(token.value);
    };
}
AST *parser_eat_expr(Parser *parser);
void parser_eat_call(Parser *parser, AST *ast){
    Token token = parser->tokens[parser->cur];
    Token orig = parser->tokens[parser->cur];
    ast->type = AST_CALL;
    parser_peek(parser);
    ast->data.jmpif.label = parser_label(parser);
    token = parser->tokens[parser->cur];
    if (string_compare(token.value, "if", 2) == 0){
        ast->type = AST_CALLIF;
        parser_peek(parser);
        ast->data.jmpif.reg = parser_eat_expr(parser);
    }else if (string_compare(token.value, "ifnot", 5) == 0){
        ast->type = AST_CALLIFN;
        parser_peek(parser);
        ast->data.jmpif.reg = parser_eat_expr(parser);
    }
}
void parser_eat_syscall(Parser *parser, AST *ast){
    Token token = parser->tokens[parser->cur];
    Token orig = parser->tokens[parser->cur];
    parser_peek(parser);
    parser_expect(parser, TOKEN_DOT);
    token = parser->tokens[parser->cur];
    parser_expect(parser, TOKEN_ID);
    ast->type = AST_SYSCALL;
    ast->data.text.value = strdup(token.value);
}

AST *parser_eat_expr(Parser *parser){
    Token token = parser->tokens[parser->cur];
    Token orig = parser->tokens[parser->cur];
    AST *ast = malloc(sizeof(AST));
    if (token.type == TOKEN_AMP) {
        ast->type = AST_REF;
        parser_peek(parser);
        ast->data.expr.left = parser_eat_expr(parser);
        ret(ast);
    }
    if (token.type == TOKEN_INT){
        ast->type = AST_INT;
        ast->data.text.value = strdup(token.value);
    };
    if (token.type == TOKEN_SUB){
        parser_peek(parser);
        token = parser->tokens[parser->cur];
        ast->type = AST_INT;
        char string[100];
        snprintf(string, 100, "-%s", token.value);
        ast->data.text.value = strdup(string);
    };
    if (token.type == TOKEN_STRING){
        ast->type = AST_STRING;
        ast->data.text.value = strdup(token.value);
    };
    if (token.type == TOKEN_CHAR){
        ast->type = AST_CHAR;
        ast->data.text.value = strdup(token.value);
    };
    if (token.type == TOKEN_ID){
        if (strcmp(token.value, "call") == 0){
            parser_eat_call(parser, ast);
        }else if (strcmp(token.value, "syscall") == 0){
            parser_eat_call(parser, ast);
        }

        if(name_to_size(token.value) != 0){
            parser_peek(parser);
            parser_expect(parser, TOKEN_LBRACKET);
            ast->typeinfo = (AST_TypeInfo){name_to_size(token.value)};
            ast->type = AST_DEREF;
            ast->data.expr.left = parser_eat_expr(parser);
            parser_expect(parser, TOKEN_RBRACKET);
            goto extr;
        };
        if (is_reg(token.value) == 0){
            ast->type = AST_REG;
            ast->data.operation.reg = reg_to_num(parser);
            goto extr;
        }else {
            ast->type = AST_VAR;
            ast->data.text.value = strdup(token.value);
        };
    };
    parser_peek(parser);
extr:
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_COLON){
        parser_peek(parser);
        Token token = parser->tokens[parser->cur];
        parser_expect(parser, TOKEN_ID);
        ast->typeinfo = name_to_size(token.value);
    };
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_EQ){
        parser_peek(parser);
        parser_expect(parser, TOKEN_EQ);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_EQ;
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
        ret(ast1);
    };
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_DOT){
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_METADATA;
        ast1->data.var.value = ast;
        parser_peek(parser);
        ast1->data.var.name = parser->tokens[parser->cur].value;
        parser_peek(parser);
    };
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_PLUS){
        parser_peek(parser);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_PLUS;
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
    }
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_GT){
        parser_peek(parser);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_LT;
        if (parser->tokens[parser->cur].type == TOKEN_EQ){
            ast1->type = AST_LTE;
            parser_peek(parser);
        };
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
        ret(ast1);
    };
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_LT){
        parser_peek(parser);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_GT;
        if (parser->tokens[parser->cur].type == TOKEN_EQ){
            ast1->type = AST_GTE;
            parser_peek(parser);
        };
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
        ret(ast1);
    };
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_AMP){
        parser_peek(parser);
        parser_expect(parser, TOKEN_AMP);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_AND;
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
        ret(ast1);
    };
    if (token.type == TOKEN_PIPE){
        parser_peek(parser);
        parser_expect(parser, TOKEN_PIPE);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_OR;
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
        ret(ast1);
    };
    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_COMMA){
        parser_peek(parser);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_ARRAY;
        ast1->data.astarray.blocklen = 0;
        ast1->data.astarray.block = NULL;
        
        // Add first element to the linked list
        append_ast_to_list(&ast1->data.astarray.block, ast, &ast1->data.astarray.blocklen);
        
        for (int i=0; ; i++){
            AST *next_expr = parser_eat_expr(parser);
            append_ast_to_list(&ast1->data.astarray.block, next_expr, &ast1->data.astarray.blocklen);
            token = parser->tokens[parser->cur];
            if (token.type != TOKEN_COMMA) break;
            parser_peek(parser);
        };
    }
    token = parser->tokens[parser->cur];

    if (token.type == TOKEN_EXC){
        parser_peek(parser);
        parser_expect(parser, TOKEN_EQ);
        AST *ast1 = malloc(sizeof(AST));
        ast1->type = AST_NEQ;
        ast1->data.expr.left = ast;
        ast1->data.expr.right = parser_eat_expr(parser);
        ret(ast1);
    };
    ret(ast);
};



AST *parser_parse_lhs(Parser *parser){
    Token token = parser->tokens[parser->cur];
    Token orig = parser->tokens[parser->cur];

    AST *ast = malloc(sizeof(AST));
    
    if (token.type == TOKEN_ID) {
        if (name_to_size(token.value) != 0) {
            parser_peek(parser);
            parser_expect(parser, TOKEN_LBRACKET);
            ast->typeinfo = (AST_TypeInfo){name_to_size(token.value)};
            ast->type = AST_DEREF;
            ast->data.expr.left = parser_eat_expr(parser);
            parser_expect(parser, TOKEN_RBRACKET);
        } else {
            if (is_reg(token.value) == 0){
                ast->type = AST_REG;
                ast->data.operation.reg = reg_to_num(parser);
            }else {
                ast->type = AST_VAR;
                ast->data.text.value = strdup(token.value);
                parser_peek(parser);
            };
        }
    }else if(token.type == TOKEN_AMP){
        ast->typeinfo = (AST_TypeInfo)8;
        ast->type = AST_REF;
        parser_peek(parser);
        ast->data.expr.left = parser_eat_expr(parser);
    }

    token = parser->tokens[parser->cur];
    if (token.type == TOKEN_COLON){
        parser_peek(parser);
        token = parser->tokens[parser->cur];
        parser_expect(parser, TOKEN_ID);
        ast->typeinfo = name_to_size(token.value);
    };
    
    ret(ast);
}

AST *parser_eat_ast(Parser *parser){
    Token token = parser->tokens[parser->cur];
    Token orig = parser->tokens[parser->cur];
    AST *ast = malloc(sizeof(AST));
    if (token.type == TOKEN_ID){
        if(strcmp(token.value, "call") == 0){
            parser_eat_call(parser, ast);
        }else if (string_compare(token.value, "syscall", 7) == 0){
            parser_eat_syscall(parser, ast);
        }else if(string_compare(token.value, "jump", 4) == 0){
            ast->type = AST_JMP;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.jmpif.label = parser_label(parser);
            ast->data.jmpif.line = 0;
            token = parser->tokens[parser->cur];
            if (string_compare(token.value, "if", 2) == 0){
                ast->type = AST_JMPIF;
                parser_peek(parser);
                token = parser->tokens[parser->cur];
                ast->data.jmpif.reg = parser_eat_expr(parser);
            }else if (string_compare(token.value, "ifnot", 5) == 0){
                ast->type = AST_JMPIFN;
                parser_peek(parser);
                token = parser->tokens[parser->cur];
                ast->data.jmpif.reg = parser_eat_expr(parser);
            }
        }else if(string_compare(token.value, "add", 3) == 0){
            ast->type = AST_ADD;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.operation.reg = reg_to_num(parser);
            parser_expect(parser, TOKEN_COMMA);
            ast->data.operation.right = parser_eat_expr(parser);
        }else if(string_compare(token.value, "sub", 3) == 0){
            ast->type = AST_SUB;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.operation.reg = reg_to_num(parser);
            parser_expect(parser, TOKEN_COMMA);
            ast->data.operation.right = parser_eat_expr(parser);
        }else if(string_compare(token.value, "mul", 3) == 0){
            ast->type = AST_MUL;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.operation.reg = reg_to_num(parser);
            parser_expect(parser, TOKEN_COMMA);
            ast->data.operation.right = parser_eat_expr(parser);
        }else if(string_compare(token.value, "div", 3) == 0){
            ast->type = AST_DIV;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.operation.reg = reg_to_num(parser);
            parser_expect(parser, TOKEN_COMMA);
            ast->data.operation.right = parser_eat_expr(parser);
        }else if(string_compare(token.value, "mod", 3) == 0){
            ast->type = AST_MOD;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.operation.reg = reg_to_num(parser);
            parser_expect(parser, TOKEN_COMMA);
            ast->data.operation.right = parser_eat_expr(parser);
        }else if(string_compare(token.value, "push", 4) == 0){
            ast->type = AST_PUSH;
            parser_peek(parser);
            ast->data.operation.right = parser_eat_expr(parser);
        }else if(string_compare(token.value, "pop", 3) == 0){
            ast->type = AST_POP;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->data.operation.reg = reg_to_num(parser);
        }else if(string_compare(token.value, "ret", 3) == 0){
            ast->type = AST_RET;
            parser_peek(parser);
        }else {
            // Expect variable assignment
            AST *lhs = parser_parse_lhs(parser);
            parser_expect(parser, TOKEN_EQ);
            ast->type = AST_MOV;
            ast->data.opexpr.left = NULL;
            ast->data.opexpr.left = lhs;
            ast->data.opexpr.right = parser_eat_expr(parser);
        }
    }else if(token.type == TOKEN_MODULO){
        parser_peek(parser);
        token = parser->tokens[parser->cur];
        if(string_compare(token.value, "label", strlen(token.value)) == 0){
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->type = AST_LABEL;
            ast->data.text.value = strdup(token.value);
            parser_peek(parser);
            parser_expect(parser, TOKEN_LB);
            ast->data.funcdef.block = NULL;
            ast->data.funcdef.blocklen = 0;
            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                AST *ast1 = parser_eat_ast(parser);
                append_ast_to_list(&ast->data.funcdef.block, ast1, &ast->data.funcdef.blocklen);
            };
            parser_expect(parser, TOKEN_RB);
        }else if(string_compare(token.value, "local", strlen(token.value)) == 0){
            ast->type = AST_LOCAL;
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            parser_expect(parser, TOKEN_ID);
            ast->data.var.name = strdup(token.value);
            parser_expect(parser, TOKEN_COMMA);
            token = parser->tokens[parser->cur];
            parser_expect(parser, TOKEN_INT);
            ast->typeinfo = atoi(token.value);
        };
    };
    ret(ast);
};

AST *parser_eat_body(Parser *parser){
    AST *ast = malloc(sizeof(AST));
    Token token = parser->tokens[parser->cur];
    Token orig = parser->tokens[parser->cur];
    if (token.type == TOKEN_MODULO){
        parser_peek(parser);
        token = parser->tokens[parser->cur];
        if (string_compare(token.value, "func", strlen(token.value)) == 0){
            parser_peek(parser);
            ast->type = AST_FUNCDEF;
            ast->data.funcdef.name = parser->tokens[parser->cur].value;
            parser_peek(parser);
            parser_expect(parser, TOKEN_LB);
            ast->data.funcdef.block = NULL;
            ast->data.funcdef.blocklen = 0;
            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                AST *ast1 = parser_eat_ast(parser);
                append_ast_to_list(&ast->data.funcdef.block, ast1, &ast->data.funcdef.blocklen);
            };
            parser_expect(parser, TOKEN_RB);
        }else if(name_to_size(token.value) != 0){
            ast->type = AST_VAR;
            token = parser->tokens[parser->cur];
            parser_peek(parser);
            ast->typeinfo = (AST_TypeInfo){name_to_size(token.value)};
            ast->data.var.name = parser->tokens[parser->cur].value;
            parser_peek(parser);
            parser_expect(parser, TOKEN_EQ);
            ast->data.var.value = parser_eat_expr(parser);
        }else if(string_compare(token.value, "const", strlen(token.value)) == 0){
            ast->type = AST_CONST;
            token = parser->tokens[parser->cur];
            parser_peek(parser);
            ast->typeinfo = (AST_TypeInfo)8;
            ast->data.var.name = parser->tokens[parser->cur].value;
            parser_peek(parser);
            parser_expect(parser, TOKEN_EQ);
            ast->data.var.value = parser_eat_expr(parser);
        }else if(string_compare(token.value, "label", strlen(token.value)) == 0){
            parser_peek(parser);
            token = parser->tokens[parser->cur];
            ast->type = AST_LABEL;
            ast->data.text.value = strdup(token.value);
            parser_peek(parser);
            parser_expect(parser, TOKEN_LB);
            ast->data.funcdef.block = NULL;
            ast->data.funcdef.blocklen = 0;
            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                AST *ast1 = parser_eat_ast(parser);
                append_ast_to_list(&ast->data.funcdef.block, ast1, &ast->data.funcdef.blocklen);
            };
            parser_expect(parser, TOKEN_RB);
        };
    };
    
    ret(ast);
}


char parser_eat(Parser *parser){
    if(parser->tokens[parser->cur].type == TOKEN_EOF)
        return -1;
    store_ast(parser, parser_eat_body(parser));
    return 0;
};
