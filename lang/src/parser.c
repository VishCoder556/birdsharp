#include "include/parser.h"


void parser_peek(Parser *parser){
    parser->cur++;
    if (parser->cur > parser->tokenlen){
        error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->tokenlen-1].row, parser->tokens[parser->tokenlen-1].col, parser->tokens[parser->cur].name);
    };
};
void parser_expect_next(Parser *parser, int type){
    if (parser->cur+1 != parser->tokenlen){
        parser_peek(parser);
        if (parser->tokens[parser->cur].type != type){
            char error[100];
            snprintf(error, 100, "Expected %s got %s", token_to_string(type), token_to_string(parser->tokens[parser->cur].type));
            error_generate_parser("ExpectError", error, parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
        };
        if (parser->cur+2 != parser->tokenlen){
            parser_peek(parser);
        }else {
        }
    }else {
        error_generate_parser("ExpectError", "Didn't expect out of bounds", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col-1, parser->tokens[parser->cur].name);
    };
};
void parser_expect(Parser *parser, int type){
    if (parser->tokens[parser->cur].type != type){
        char error[100];
        snprintf(error, 100, "Expected %s got %s", token_to_string(type), token_to_string(parser->tokens[parser->cur].type));
        error_generate_parser("ExpectError", error, parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
    };
    if (parser->cur != parser->tokenlen){
        parser_peek(parser);
    }else {
    }
};
bool parser_is(Parser *parser, int type){
    if (parser->tokens[parser->cur].type == type){
        return true;
    };
    return false;
};

char parse_type(Parser *parser, AST_TypeInfo *typeinfo){
    typeinfo->type = parser->tokens[parser->cur].value;
    typeinfo->kind = KIND_UNKNOWN;
    for (int i = 0; i < typesLen; i++) {
        if (strcmp(types[i].name, parser->tokens[parser->cur].value) == 0) {
            typeinfo->kind = types[i].kind;
            break;
        }
    }
    parser_peek(parser);
    typeinfo->ptrnum = 0;
    while (parser->tokens[parser->cur].type == TOKEN_MUL){
        typeinfo->ptrnum++;
        parser_peek(parser);
    };
    if (parser->tokens[parser->cur].type == TOKEN_LBRACKET){
        parser_expect(parser, TOKEN_LBRACKET);
        AST_TypeInfo *typeinfo1 = malloc(sizeof(AST_TypeInfo));
        Token t = parser->tokens[parser->cur];
        parser_expect(parser, TOKEN_INT);
        *typeinfo1 = *typeinfo;
        typeinfo->data.array.elem_type = typeinfo1;
        typeinfo->data.array.size = atoi(t.value);
        typeinfo->kind = KIND_ARRAY;
        parser_expect(parser, TOKEN_RBRACKET);
    }
    return 0;
}
char is_type(Parser *parser, Token tok){
    for (int v=0; v<typesLen; v++){
        if (strcmp(types[v].name, tok.value) == 0){
            return 0;
        };
    };
    return -1;
};


AST *parser_eat_expr(Parser *parser);

AST *mode_parse_expr(Parser *parser){
    AST *ast = malloc(sizeof(AST));
    *ast = (AST){0};
    ast->type = AST_MODE;
    ast->data.mode.name = malloc(500);
    strcpy(ast->data.mode.name, "");
    while (parser->tokens[parser->cur].type == TOKEN_ID || parser->tokens[parser->cur].type == TOKEN_DOT){
        strncat(ast->data.mode.name, parser->tokens[parser->cur].value, 100);
        parser_peek(parser);
    };
    if (parser->tokens[parser->cur].type == TOKEN_EOF){
        error_generate("ModeError", "Abrupt end to expression of if macro");
    };
    return ast;
};

AST **top_level = NULL;
int top_level_len = 0;
bool is_flat = 0;
AST *_main = NULL;


AST *parser_eat_ast(Parser *parser);
char try_parse_mode(Parser *parser, AST *ast){
    if (parser->tokens[parser->cur].type == TOKEN_HASH){
        if (parser->cur+1 == parser->tokenlen){
            error_generate("HashError", "# not allowed this late into code");
            exit(0);
        }
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_EXC) {
            ast->type = AST_MODE;
            parser_peek(parser);
            if (strcmp(parser->tokens[parser->cur].value, "if") == 0){
                ast->type = AST_MODE_IF;
                parser_peek(parser);
                ast->data.if1 = (AST_If){};
                ast->data.if1.block.condition = mode_parse_expr(parser);
                ast->data.if1.block.statements = malloc(sizeof(AST*) * 100);
                ast->data.if1.block.statementlen = 0;
                parser_expect(parser, TOKEN_LB);
                while (parser->tokens[parser->cur].type != TOKEN_RB){
                    ast->data.if1.block.statements[ast->data.if1.block.statementlen++] = parser_eat_ast(parser);
                };
                parser_peek(parser);
                return 0;
            }else if(strcmp(parser->tokens[parser->cur].value, "flat") == 0){
                is_flat = 1;
                if (top_level == NULL)
                    top_level = malloc(sizeof(AST*) * 20);
                parser_peek(parser);
                return -1; // ignore as mode
            }else if(strcmp(parser->tokens[parser->cur].value, "round") == 0){
                is_flat = 0;
                parser_peek(parser);
                return -1; // ignore as mode
            };
            ast->data.mode.name = parser->tokens[parser->cur].value;
            if (parser->cur+1 == parser->tokenlen){
                error_generate("HashError", "# not allowed this late into code");
                exit(0);
            }
            parser_peek(parser);
            ast->data.mode.res = parser->tokens[parser->cur].value;
            parser_peek(parser);
            return 0;
        }else {
            parser->cur-=2;
        };
    };
    return -1;
};


int get_precedence(int type){
    switch (type){
        case AST_EQ: case AST_NEQ: 
            return 1;

        case AST_LT: case AST_LTE: case AST_GT: case AST_GTE: 
            return 2;

        case AST_PLUS: case AST_SUB: 
            return 3;

        case AST_MUL: case AST_DIV: case AST_MODULO: 
            return 4;

        default: return -1;
    };
}


AST* rotate_to_left(AST* parent) {
    // 1. If this isn't an expression or has no right child, do nothing
    if (parent == NULL || parent->data.expr.right == NULL) {
        return parent;
    }

    parent->data.expr.right = rotate_to_left(parent->data.expr.right);

    AST* child = parent->data.expr.right;

    if (get_precedence(parent->type) != -1 && 
        get_precedence(child->type) != -1 &&
        get_precedence(parent->type) >= get_precedence(child->type)) {
        
        parent->data.expr.right = child->data.expr.left;
        child->data.expr.left = rotate_to_left(parent);
        
        return child; 
    }

    return parent;
}

AST *parser_eat_expr(Parser *parser){
    Token token_1 = parser->tokens[parser->cur];
    int a = parser->cur;
    AST *ast = malloc(sizeof(AST));
    *ast = (AST){0};
    ast->row = parser->tokens[a].row;
    ast->col = parser->tokens[a].col;
    ast->filename = parser->tokens[a].name;
    ast->next = NULL;
    if (try_parse_mode(parser, ast) == 0){
        return ast;
    };
    ast->type = AST_UNKNOWN;
    if (parser->tokens[parser->cur].type == TOKEN_LP){
        parser_peek(parser);
        ast->type = AST_EXPR;
        ast->data.expr.left = parser_eat_expr(parser);
        parser_expect(parser, TOKEN_RP);
    }else if (parser->tokens[parser->cur].type == TOKEN_EXC){
        parser_peek(parser);
        ast->type = AST_NOT;
        ast->data.expr.left = parser_eat_expr(parser);
    }else if(parser->tokens[parser->cur].type == TOKEN_AMP){
        parser_peek(parser);
        ast->type = AST_REF;
        ast->data.expr.left = parser_eat_expr(parser);
        ast->typeinfo = ast->data.expr.left->typeinfo;
        ast->typeinfo.ptrnum++;
        return ast;
    }else if (parser->tokens[parser->cur].type == TOKEN_STRING){
        ast->type = AST_STRING;
        ast->typeinfo.ptrnum = 1;
        ast->typeinfo.kind = KIND_CHAR;
        ast->data.arg.value = parser->tokens[parser->cur].value;
        parser_peek(parser);
    }else if(parser->tokens[parser->cur].type == TOKEN_SUB){
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_INT){
            ast->type = AST_INT;
            ast->typeinfo.kind = KIND_CONST;
            char a[500];
            snprintf(a, 500, "-%s", parser->tokens[parser->cur].value);
            ast->data.arg.value = strdup(a);
            parser->cur++;
        }else {
            error_generate_parser("AbruptEndError", "Found negative sign alone", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->name);
        }
    }else if (parser->tokens[parser->cur].type == TOKEN_INT){
        ast->type = AST_INT;
        ast->typeinfo.kind = KIND_CONST;
        ast->data.arg.value = parser->tokens[parser->cur].value;
        if (strcmp(parser->tokens[parser->cur].value, "true") == 0){
            ast->data.arg.value = "1";
            ast->typeinfo.kind = KIND_BOOL;
        }else if (strcmp(parser->tokens[parser->cur].value, "false") == 0){
            ast->data.arg.value = "0";
            ast->typeinfo.kind = KIND_BOOL;
        }
        parser_peek(parser);
    }else if (parser->tokens[parser->cur].type == TOKEN_FLOAT){
        ast->type = AST_FLOAT;
        ast->typeinfo.kind = KIND_FLOAT;
        ast->data.arg.value = parser->tokens[parser->cur].value;
        parser_peek(parser);
    }else if (parser->tokens[parser->cur].type == TOKEN_CHAR){
        ast->type = AST_CHAR;
        ast->typeinfo.kind = KIND_CHAR;
        ast->data.arg.value = parser->tokens[parser->cur].value;
        parser_peek(parser);
    }else if (parser->tokens[parser->cur].type == TOKEN_ID){
        if (parser->tokens[parser->cur+1].type == TOKEN_LP){
            ast->type = AST_FUNCALL;
            ast->data.funcall = (AST_FuncCall){};
            ast->data.funcall.args = malloc(sizeof(AST*) * 100);
            parser_expect(parser, TOKEN_ID);
            ast->data.funcall.name = parser->tokens[parser->cur-1].value;
            parser_expect(parser, TOKEN_LP);
            if (strcmp(ast->data.funcall.name, "cast") == 0){
                ast->type = AST_CAST;
                parse_type(parser, &ast->typeinfo);
                parser_expect(parser, TOKEN_COMMA);
                ast->data.expr.left = parser_eat_expr(parser);
                parser_expect(parser, TOKEN_RP);
                goto skip;
            };
            while (parser->tokens[parser->cur].type != TOKEN_RP && parser->tokens[parser->cur].type != TOKEN_EOF){
                ast->data.funcall.args[ast->data.funcall.argslen++] = parser_eat_expr(parser);
                if (parser->tokens[parser->cur].type != TOKEN_RP){
                    parser_expect(parser, TOKEN_COMMA);
                };
            };
            parser_expect(parser, TOKEN_RP);
            if (strcmp(ast->data.funcall.name, "deref") == 0){
                ast->type = AST_DEREF;
                if (ast->data.funcall.argslen != 1){
                    error_generate("DerefError", "Too less or too many arguments to deref function");
                };
                ast->data.expr.left = ast->data.funcall.args[0];
            }else if(strncmp(ast->data.funcall.name, "syscall", strlen("syscall")) == 0){
                ast->type = AST_SYSCALL;
                ast->data.funcall.name = ast->data.funcall.name;
            };
        }else {
            ast->type = AST_VAR;
            ast->data.arg.value = parser->tokens[parser->cur].value;
            parser_peek(parser);
        }
    };
skip:

    if (parser->tokens[parser->cur].type == TOKEN_ID){
        if (parser->tokens[parser->cur].value[0] == 'x' && strcmp(ast->data.arg.value, "0") == 0){
            char decimal_string[20];
            strncpy(decimal_string, "0", 20);
            strncat(decimal_string, parser->tokens[parser->cur].value, 19);
            ast->data.arg.value = strdup(decimal_string);
            parser_peek(parser);
        };
    }

    if (parser->tokens[parser->cur].type == TOKEN_LBRACKET){
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_INDEX;
        ast2->row = parser->tokens[parser->cur].row;
        ast2->col = parser->tokens[parser->cur].col;
        ast2->filename = parser->tokens[parser->cur].name;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        if (parser->tokens[parser->cur].type != TOKEN_RBRACKET){
            error_generate_parser("AbruptEndError", "Abrupt end to array subscript", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->name);
        }
        parser_peek(parser);
        ast = ast2;
    }
    if (parser->tokens[parser->cur].type == TOKEN_PLUS){
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_PLUS;
        ast2->row = parser->tokens[parser->cur].row;
        ast2->col = parser->tokens[parser->cur].col;
        ast2->filename = parser->tokens[parser->cur].name;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        ast = rotate_to_left(ast2);
    }
    if (parser->tokens[parser->cur].type == TOKEN_SUB){
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_SUB;
        ast2->row = parser->tokens[parser->cur].row;
        ast2->col = parser->tokens[parser->cur].col;
        ast2->filename = parser->tokens[parser->cur].name;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        ast = rotate_to_left(ast2);
    }
    if (parser->tokens[parser->cur].type == TOKEN_MUL){
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_MUL;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end in multiplication");
        };
        parser_peek(parser);
        if (parser->tokens[parser->cur+1].type == TOKEN_EQ){
            parser->cur--;
            free(ast2);
            return ast;
        }
        ast2->data.expr.right = parser_eat_expr(parser);
        ast = rotate_to_left(ast2);
    }
    if (parser->tokens[parser->cur].type == TOKEN_DIV){
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_DIV;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        ast = rotate_to_left(ast2);
    }
    if (parser->tokens[parser->cur].type == TOKEN_MODULO){
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_MODULO;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        ast = rotate_to_left(ast2);
    }
    if (parser->tokens[parser->cur].type == TOKEN_GT){
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_GT;
        if (parser->tokens[parser->cur].type == TOKEN_EQ){
            ast2->type = AST_GTE;
        }else {
            parser->cur--;
        };
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        }
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        return ast2;
    }
    if (parser->tokens[parser->cur].type == TOKEN_LT){
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        AST *ast2 = malloc(sizeof(AST));
        *ast2 = (AST){0};
        ast2->type = AST_LT;
        if (parser->tokens[parser->cur].type == TOKEN_EQ){
            ast2->type = AST_LTE;
        }else {
            parser->cur--;
        };
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        ast2->data.expr.right = parser_eat_expr(parser);
        return ast2;
    }
    if (parser->tokens[parser->cur].type == TOKEN_EQ){
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_EQ){
            AST *ast2 = malloc(sizeof(AST));
            *ast2 = (AST){0};
            ast2->type = AST_EQ;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast = ast2;
        }else {
            parser->cur--;
        };
    }
    if (parser->tokens[parser->cur].type == TOKEN_EXC){
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_EQ){
            AST *ast2 = malloc(sizeof(AST));
            *ast2 = (AST){0};
            ast2->type = AST_NEQ;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast2->typeinfo =  (AST_TypeInfo){"", KIND_UNKNOWN, 0};
            ast = ast2;
        };
    }
    if (parser->tokens[parser->cur].type == TOKEN_AMP) {
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_AMP){
            AST *ast2 = malloc(sizeof(AST));
            *ast2 = (AST){0};
            ast2->type = AST_AND;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast2->typeinfo =  (AST_TypeInfo){"", KIND_BOOL, 0};
            ast = ast2;
        }else {
            error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->name);
        }
    };
    if (parser->tokens[parser->cur].type == TOKEN_PIPE){
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end");
        };
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_PIPE){
            AST *ast2 = malloc(sizeof(AST));
            *ast2 = (AST){0};
            ast2->type = AST_OR;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast2->typeinfo =  (AST_TypeInfo){"", KIND_BOOL, 0};
            ast = ast2;
        }else {
            error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->name);
        }
    }
    return ast;
};

AST *parser_eat_ast(Parser *parser){
    int a = parser->cur;
    AST *ast = malloc(sizeof(AST));
    *ast = (AST){0};
    ast->next = NULL;
    if (try_parse_mode(parser, ast) == 0){
        return ast;
    };
    ast->type = AST_UNKNOWN;
    if (parser->tokens[parser->cur].type == TOKEN_ID){
        if(strcmp(parser->tokens[parser->cur].value, "if") == 0){
            ast->type = AST_IF;
            if (parser->cur + 1 >= parser->tokenlen){
                error_generate_parser("AbruptEnd", "Sudden end to if statement", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
            }
            
            parser_peek(parser);
            ast->data.if1.block.condition = parser_eat_expr(parser);
            parser_expect(parser, TOKEN_LB);

            ast->data.if1.block.statements = malloc(sizeof(AST*) * 100);
            ast->data.if1.block.statementlen = 0;
            ast->data.if1.else1 = malloc(sizeof(AST*) * 100);
            ast->data.if1.elseif = malloc(sizeof(Block) * 100);
            ast->data.if1.elselen = 0;
            ast->data.if1.elseiflen = 0;

            Token prev = (Token){0};
            int c = 0;

            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                if (parser->tokens[parser->cur].type == prev.type && strcmp(parser->tokens[parser->cur].value, prev.value) == 0){
                    if (++c > 10000) error_generate_parser("SyntaxError", "Infinite loop detected", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
                } else {
                    c = 0;
                }
                ast->data.if1.block.statements[ast->data.if1.block.statementlen++] = parser_eat_ast(parser);
                prev = parser->tokens[parser->cur];
            }
            parser_expect(parser, TOKEN_RB);

            while (parser->cur < parser->tokenlen && strcmp(parser->tokens[parser->cur].value, "else") == 0){
                parser_peek(parser);
                
                if (strcmp(parser->tokens[parser->cur].value, "if") == 0){
                    parser_peek(parser);
                    
                    int ei = ast->data.if1.elseiflen;
                    ast->data.if1.elseif[ei].condition = parser_eat_expr(parser);
                    ast->data.if1.elseif[ei].statements = malloc(sizeof(AST*) * 100);
                    ast->data.if1.elseif[ei].statementlen = 0;
                    
                    parser_expect(parser, TOKEN_LB);

                    prev = (Token){0};
                    c = 0;
                    while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                        ast->data.if1.elseif[ei].statements[ast->data.if1.elseif[ei].statementlen++] = parser_eat_ast(parser);
                    }
                    ast->data.if1.elseiflen++;
                    parser_expect(parser, TOKEN_RB);
                } else {
                    parser_expect(parser, TOKEN_LB);
                    while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                        ast->data.if1.else1[ast->data.if1.elselen++] = parser_eat_ast(parser);
                    }
                    parser_expect(parser, TOKEN_RB);
                    break; 
                }
            }
        }else if(strcmp(parser->tokens[parser->cur].value, "while") == 0){
                ast->type = AST_WHILE;
                ast->data.while1.block = malloc(sizeof(AST*) * 100);
                if (parser->cur + 1 == parser->tokenlen){
                    error_generate_parser("AbruptEnd", "Sudden end to while statement", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
                }
                parser_peek(parser);
                ast->data.while1.condition = parser_eat_expr(parser);
                parser_expect(parser, TOKEN_LB);
                Token prev = (Token){0};
                char c = 0;
                ast->data.while1.blocklen = 0;
                while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                    if (parser->tokens[parser->cur].type == prev.type && strcmp(parser->tokens[parser->cur].value, prev.value) == 0){
                        c++;
                        if (c > 5){
                            error_generate_parser("SyntaxError", "Something went wrong in while", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
                        }
                    }else {
                        c = 0;
                    };
                    prev = parser->tokens[parser->cur];
                    ast->data.while1.block[ast->data.while1.blocklen++] = parser_eat_ast(parser);
                };
                parser_expect(parser, TOKEN_RB);
            }else if (strcmp(parser->tokens[parser->cur].value, "return") == 0) {
                parser_peek(parser);
                ast->type = AST_RET;
                ast->data.ret.ret = parser_eat_expr(parser);
            }else if((is_type(parser, parser->tokens[parser->cur]) == 0)){
                parse_type(parser, &ast->typeinfo);
                ast->type = AST_ASSIGN;
                parser_expect(parser, TOKEN_ID);
                parser->cur--;
                ast->data.assign.new = true;
                ast->data.assign.from = parser_eat_expr(parser);
                ast->data.assign.from->typeinfo = ast->typeinfo;
                if (parser->tokens[parser->cur].type == TOKEN_EQ){
                    parser_peek(parser);
                    ast->data.assign.assignto = parser_eat_expr(parser);
                }else {
                    ast->data.assign.assignto = malloc(sizeof(*(ast->data.assign.assignto)));
                    *(ast->data.assign.assignto) = (AST){0};
                    ast->data.assign.assignto->type = AST_UNKNOWN;
                }
            }else if (strcmp(parser->tokens[parser->cur].value, "return") == 0) {
                parser_peek(parser);
                ast->type = AST_RET;
                ast->data.ret.ret = parser_eat_expr(parser);
            }else if(strcmp(parser->tokens[parser->cur].value, "break") == 0){
                ast->type = AST_BREAK;
                parser->cur++;
            }else {
                AST *ast_expr = parser_eat_expr(parser);
                if(parser->tokens[parser->cur].type == TOKEN_EQ){
                    ast->type = AST_ASSIGN;
                    ast->row = parser->tokens[parser->cur].row;
                    ast->col = parser->tokens[parser->cur].col;
                    ast->filename = parser->tokens[parser->cur].name;
                    ast->typeinfo.type = "unknown";
                    ast->typeinfo.kind = KIND_UNKNOWN;
                    ast->data.assign.from = ast_expr;
                    parser_peek(parser);
                    ast->data.assign.assignto = parser_eat_expr(parser);
                    return ast;
                }else if (parser->tokens[parser->cur].type == TOKEN_LP){
                    ast->type = AST_FUNCALL;
                    ast->data.funcall = (AST_FuncCall){0};
                    parser_expect(parser, TOKEN_ID);
                    ast->data.funcall.name = parser->tokens[parser->cur-1].value;
                    parser_expect(parser, TOKEN_LP);
                    while (parser->tokens[parser->cur].type != TOKEN_RP && parser->tokens[parser->cur].type != TOKEN_EOF){
                        ast->data.funcall.args[ast->data.funcall.argslen++] = parser_eat_expr(parser);
                        if (parser->tokens[parser->cur].type != TOKEN_COMMA && parser->tokens[parser->cur].type != TOKEN_RP){
                            error_generate("CommaError", "Expected Comma");
                        }else if(parser->tokens[parser->cur].type == TOKEN_COMMA)
                            parser->cur++;
                    };
                    parser_expect(parser, TOKEN_RP);
            }else {
                return ast_expr;
            }
        }
    }else{
        AST *ast_expr = parser_eat_expr(parser);
        if(parser->tokens[parser->cur].type == TOKEN_EQ){
            ast->type = AST_ASSIGN;
            ast->row = parser->tokens[parser->cur].row;
            ast->col = parser->tokens[parser->cur].col;
            ast->filename = parser->tokens[parser->cur].name;
            ast->typeinfo.kind = KIND_UNKNOWN;
            ast->data.assign.new = false;
            ast->data.assign.from = ast_expr;
            parser_peek(parser);
            ast->data.assign.assignto = parser_eat_expr(parser);
            return ast;
        }else if(parser->tokens[parser->cur].type == TOKEN_LP){

        }else if(is_flat == 1){
            free(ast);
            return NULL;
        }else {
            error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->cur+2].row, parser->tokens[parser->cur+2].col, parser->tokens[parser->cur].name);
       }
    }

    return ast;
}

AST *parser_eat_body(Parser *parser){
    int av = parser->cur;
    AST *ast = malloc(sizeof(AST));
    *ast = (AST){0};
    ast->row = parser->tokens[av].row;
    ast->col = parser->tokens[av].col;
    ast->filename = parser->tokens[av].name;
    ast->next = NULL;
    int orig_cur = parser->cur;
    if (try_parse_mode(parser, ast) == 0){
        return ast;
    };

parse_flat:
    parser->cur = orig_cur;
    if (is_flat == 1){
        AST *ast_new = parser_eat_ast(parser);
        if (ast_new == NULL){
            ast->type = AST_UNKNOWN;
            return ast;
        }else {
            free(ast);
            top_level[top_level_len++] = ast_new;
            return parser_eat_body(parser);
        }
    }
    ast->type = AST_UNKNOWN;
    if (parser->tokens[parser->cur].type == TOKEN_ID){
        if(is_type(parser, parser->tokens[parser->cur]) == 0){
            ast->type = AST_ASSIGN;
            parse_type(parser, &ast->typeinfo);
            ast->data.assign.from = malloc(sizeof(struct AST));
            *(ast->data.assign.from) = (AST){0};
            ast->data.assign.from->type = AST_VAR;
            ast->data.assign.from->data.arg.value = parser->tokens[parser->cur].value;
            parser_peek(parser);
            if (parser->tokens[parser->cur].type == TOKEN_EQ){
                parser_peek(parser);
                ast->data.assign.assignto = parser_eat_expr(parser);
            }else if(parser->tokens[parser->cur].type == TOKEN_LP){
                ast->type = AST_FUNCDEF;
                ast->data.funcdef.args = malloc(sizeof(Argument*) * 100);
                ast->data.funcdef.block = malloc(sizeof(AST*) * 200);
                ast->data.funcdef.name = ast->data.assign.from->data.arg.value;
                if (strcmp(ast->data.funcdef.name, "main") == 0){
                    _main = ast;
                }
                parser_peek(parser);
            while (parser->tokens[parser->cur].type != TOKEN_RP && parser->tokens[parser->cur].type != TOKEN_EOF){
                ast->data.funcdef.args[ast->data.funcdef.argslen] = malloc(sizeof(Argument));
                parse_type(parser, &(ast->data.funcdef.args[ast->data.funcdef.argslen]->type));
                ast->data.funcdef.args[ast->data.funcdef.argslen]->arg = parser->tokens[parser->cur].value;
                ast->data.funcdef.argslen++;
                parser_peek(parser);
                if (parser->tokens[parser->cur].type != TOKEN_RP){
                    if(parser_is(parser, TOKEN_COMMA)){
                        parser_peek(parser);
                    }else {
                        goto parse_flat;
                    }
                };
            };
            if(parser_is(parser, TOKEN_RP)){
                parser_peek(parser);
            }else {
                goto parse_flat;
            }
            if(parser_is(parser, TOKEN_LB)){
                parser_peek(parser);
            }else {
                goto parse_flat;
            }
            Token prev = (Token){0};
            char c = 0;
            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                if (parser->tokens[parser->cur].type == prev.type && strcmp(parser->tokens[parser->cur].value, prev.value) == 0){
                    c++;
                    if (c > 5){
                        error_generate_parser("SyntaxError", "Something went wrong in function", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
                    }
                }else {
                    c = 0;
                };
                ast->data.funcdef.block[ast->data.funcdef.blocklen++] = parser_eat_ast(parser);
                prev = parser->tokens[parser->cur-1];
            };
            parser_expect(parser, TOKEN_RB);
            }else {
                goto parse_flat;
            }
        }else {
            error_generate_parser("SyntaxError", "Unknown identifier", parser->tokens[av].row, parser->tokens[av].col, parser->name);
        };
    };
    return ast;

}

char parser_eat(Parser *parser){
    if(parser->tokens[parser->cur].type == TOKEN_EOF){
        return -1;
    };
    AST *node = parser_eat_body(parser);
    if (!parser->asts){
        parser->asts = node;
        parser->ast_tail = node;
    } else {
        parser->ast_tail->next = node;
        parser->ast_tail = node;
    }
    parser->astlen++;
    return 0;
};





char *typeinfo_to_string(AST_TypeInfo typeinfo){
    char a[1000];
    int len = 0;
    
    if (typeinfo.kind == KIND_UNKNOWN) {
        return strdup("unknown");
    }
    
    const char *canonical;

    switch (typeinfo.kind) {
        case KIND_VOID:    canonical = "void"; break;
        case KIND_BOOL:    canonical = "bool"; break;
        case KIND_CHAR:
        case KIND_I8:      canonical = "char"; break;
        case KIND_I16:     canonical = "short"; break;
        case KIND_I32:
        case KIND_INT:     canonical = "int"; break;
        case KIND_I64:     canonical = "long"; break;
        case KIND_FLOAT:   canonical = "float"; break;
        case KIND_DOUBLE:  canonical = "double"; break;
        case KIND_STRUCT:
            if (typeinfo.type && strncmp(typeinfo.type, "struct", 6) == 0) {
                canonical = typeinfo.type + 6;
            } else {
                canonical = typeinfo.type ? typeinfo.type : "struct";
            }
            break;
        case KIND_CONST:   canonical = "const"; break;
        case KIND_ARRAY:   canonical = typeinfo_to_string(*typeinfo.data.array.elem_type); break;
        default:           canonical = "unknown"; break;
    }
    for (int i=0; i<typesLen; i++){
        if (types[i].kind == typeinfo.kind){
            canonical = types[i].name;
        }
    }
    
    size_t type_len = strlen(canonical);
    if (type_len >= 1000) {
        error_generate("TypeError", "Type name too long");
        return strdup("");
    }

    strncpy(a, canonical, type_len);
    len = type_len;
    a[len] = '\0';
    
    for (int v=0; v<typeinfo.ptrnum; v++){
        if (len >= 999) {
            error_generate("TypeError", "Too many pointer indirections");
            break;
        }
        a[len++] = '*';
    }
    a[len] = '\0';
    return strdup(a);
}

Parser *parser_init(Tokenizer *tokenizer){
    Parser *parser = arena_alloc(&arena, sizeof(Parser));
    parser->name = tokenizer->name;
    parser->asts = NULL;
    parser->ast_tail = NULL;
    parser->astlen = 0;
    parser->cur = 0;
    parser->tokens = tokenizer->tokens;
    parser->tokens[parser->cur].name = tokenizer->name;
    parser->tokenlen = tokenizer->tokenlen;
    return parser;
}
