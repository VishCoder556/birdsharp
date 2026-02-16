/*
 * BirdSharp Compiler
 * BirdSharp is a programming language that aims to be similar to C, while having many high level features.
 *
 * Why Use BirdSharp:
 *   1. BirdSharp, as stated earlier, is similar to C, but has nearly no bloat.
 *   2. BirdSharp is extremely fast (although dynamic libraries seem to slow the compile time a lot)
 *   3. BirdSharp supports bytecode and compilation directly into Assembly that can be compiled to create executables.
 *   4. BirdSharp has many features unique to it, such as modes, that allow you to change some stuff about the compiler during runtime, instead of having to rely on compiler flags for such tasks.
 *
 * NOTE: BirdSharp is not finished yet, and hence has a ton of unclean code, and places to improve upon. Do not rely on BirdSharp for complex projects.
 *
 * How to try out BirdSharp:
 *  There are two ways:
 *      1. Executables (The main version of BirdSharp):
 *          Run "./exes/bs.out main.bsh" to create the .s file
 *              (This will automatically assemble it into a .o file and link it into a .out)
 *          Run "./main.out" to run your file!
 *
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <wchar.h>
#include <unistd.h>
#include <time.h>

char *HELP = 
"Help for BirdSharp\n\n"
"-o: Specify the output file\n"
"-h: Print this menu\n"
"-dynamic: Specify dynamic linking (linking with the stdlibrary + a lot more, but at the cost of the program being slow)\n"
"-static: Specify static linking (done by default, no linking with stdlib and other libraries, but the program is fast)\n"
"-...: Anything starting with '-' will be treated as a flag that will be sent to the assembler and linker\n";



enum BirdSharpTypes {
   TYPE_STATIC,
   TYPE_DYNAMIC
};
typedef enum BirdSharpTypes BirdSharpTypes;
BirdSharpTypes _BirdSharpTypes;


enum TokenType {
    TOKEN_EOF,
    TOKEN_STRING,
    TOKEN_LP,
    TOKEN_RP,
    TOKEN_LB,
    TOKEN_RB,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_HASH,
    TOKEN_EXC,
    TOKEN_COMMA,
    TOKEN_INT,
    TOKEN_EQ,
    TOKEN_DOLLAR,
    TOKEN_CHAR,
    TOKEN_ID,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_PLUS,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MODULO,
    TOKEN_AMP,
    TOKEN_DOT,
    TOKEN_FLOAT,
    TOKEN_PIPE
};



char *token_to_string(int tokentype){
    switch (tokentype){
        case TOKEN_EOF: return "end of file";
        case TOKEN_STRING: return "string";
        case TOKEN_CHAR: return "character";
        case TOKEN_ID: return "identifier";
        case TOKEN_LP: return "'('";
        case TOKEN_RP: return "')'";
        case TOKEN_LB: return "'{'";
        case TOKEN_RB: return "'}'";
        case TOKEN_LBRACKET: return "'['";
        case TOKEN_RBRACKET: return "']'";
        case TOKEN_HASH: return "'#'";
        case TOKEN_EXC: return "'!'";
        case TOKEN_COMMA: return "','";
        case TOKEN_INT: return "integer";
        case TOKEN_EQ: return "'='";
        case TOKEN_DOLLAR: return "'$'";
        case TOKEN_GT: return "'<'";
        case TOKEN_LT: return "'>'";
        case TOKEN_PLUS: return "'+'";
        case TOKEN_SUB: return "'-'";
        case TOKEN_MUL: return "'*'";
    };
    return "unknown";
};

typedef enum{
    AST_UNKNOWN,
    AST_STRING,
    AST_CHAR,
    AST_FUNCALL,
    AST_FUNCDEF,
    AST_FLOAT,
    AST_INT,
    AST_RET,
    AST_ASSIGN,
    AST_VAR,
    AST_DOLLAR,
    AST_GT,
    AST_LT,
    AST_GTE,
    AST_LTE,
    AST_PLUS,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MODULO,
    AST_EQ,
    AST_NEQ,
    AST_IF,
    AST_WHILE,
    AST_DEREF,
    AST_REF,
    AST_MODE,
    AST_EXPR,
    AST_CAST,
    AST_SYSCALL,
    AST_BREAK,
    AST_MODE_IF,
    AST_TYPEDEF,
    AST_NOT,
    AST_AND,
    AST_OR,
    AST_INDEX,
    AST_STRUCT,
    AST_ACCESS
}AST_Type;

typedef struct AST AST;
typedef struct AST_T AST_T;

typedef struct Arg {
    char *value;
    int num;
}AST_Arg;


typedef struct ASTs {
    struct AST *ast;
    struct ASTs *next;
}ASTs;

typedef struct {
    char *name;
    AST **args;
    int argslen;
} AST_FuncCall;
typedef struct {
    AST *statements;
    int statementlen;
    AST *condition;
}Block;
typedef struct {
    Block block;
    AST *else1;
    int elselen;
    Block *elseif;
    int elseiflen;
} AST_If;
typedef struct {
    char *value;
    AST *opt;
} AST_OptVar;
typedef struct {
    AST *condition;
    AST **block;
    int blocklen;
} AST_While;

typedef struct {
    AST *from;
    AST *assignto;
    bool new;
    bool alias;
} AST_Assign;

typedef struct {
    char *type;
    char ptrnum;
} AST_TypeInfo;

typedef struct {
    AST *ret;
} AST_Return;

typedef struct Argument Argument;

typedef struct {
    char *name;
    struct Argument **args;
    int argslen;
    AST **block;
    int blocklen;
} AST_FuncDef;

typedef struct {
    AST_TypeInfo type;
    char *name;
}Field;

typedef struct {
    char *name;
    Field **fields;
    int fieldlen;
}AST_Struct;

typedef struct {
    AST *left;
    AST *right;
} AST_Expr;

typedef struct {
    char *name;
    char *res;
}AST_Mode;

typedef union {
    AST_FuncDef funcdef;
    AST_FuncCall funcall;
    AST_Assign assign;
    AST_Return ret;
    AST_Arg arg;
    AST_Expr expr;
    AST_If if1;
    AST_While while1;
    AST_Mode mode;
    AST_Struct struct1;
    AST_OptVar optvar;
} AST_Data;

struct AST {
    AST_Type type;
    AST_Data data;
    AST_TypeInfo typeinfo;
    int row;
    int col;
    char *filename;
    AST *next;
};


typedef struct Token{
    int type;
    char value[90];
    int row;
    int col;
    char *name;
}Token;
typedef struct Tokenizer {
    char *code;
    char *name;
    int cur;
    int line;
    int col;
    Token *tokens;
    int tokenlen;
}Tokenizer;
typedef struct Parser {
    AST *asts;
    int astlen;
    AST prevAst;
    char *name;
    int cur;
    Token *tokens;
    int tokenlen;
}Parser;

typedef struct Argument {
    char *arg;
    AST_TypeInfo type;
}Argument;

struct Pair {
    char *name;
    int length;
    char *longname;
};

struct Pair types[100] = {
    (struct Pair){"int", 4, "integer"},
    (struct Pair){"char", 1, "character"},
    (struct Pair){"long", 8, "long integer"},
    (struct Pair){"float", 4, "float"},
    (struct Pair){"i8", 1, "8-bit integer"},
    (struct Pair){"i16", 2, "16-bit integer"},
    (struct Pair){"i32", 4, "32-bit integer"},
    (struct Pair){"i64", 8, "64-bit integer"},
    (struct Pair){"bool", 1, "boolean (only 0 or 1)"},
};
int typesLen = 9;

typedef struct {
    char *file_path;
    char *code;
    size_t size;
} SourceFile;
#define MAX_FILES 16
SourceFile source_files[MAX_FILES];
int num_source_files = 0;

// Utilities:

void error_generate(char *type, char *name){
    printf("\x1b[1;31m%s\x1b[0m: %s\n", type, name);
    exit(-1);
};

void print_error_line(const char *code, int line, int col) {
    int curr_line = 1;
    const char *start = code;
    const char *end = code;

    while (curr_line < line && *end) {
        if (*end == '\n') {
            curr_line++;
            start = end + 1;
        }
        end++;
    }

    while (*end && *end != '\n') end++;

    printf("%.*s\n", (int)(end - start), start);

    int pointer_col = col;
    if (pointer_col < 1) pointer_col = 1;
    for (int i = 1; i < pointer_col; ++i) putchar(' ');
    puts("\x1b[1;31m^\x1b[0m\n");
}


void error_generate_parser(char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;31m%s\x1b[0m: %s\n", f, l, c, type, name);
    for (int i=0; i<num_source_files; i++){
        if (strcmp(source_files[i].file_path, f) == 0){
            print_error_line(source_files[i].code, l, c);
        };
    };
    exit(-1);
};

void warning_generate(char *type, char *name){
    printf("\x1b[1;35m%s\x1b[0m: %s\n", type, name);
};
void warning_generate_parser(char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;35m%s\x1b[0m: %s\n", f, l, c, type, name);
};


char* get_argument_register(int n) {
    switch (n) {
        case 0: return "rdi";
        case 1: return "rsi";
        case 2: return "rdx";
        case 3: return "rcx";
        case 4: return "r8";
        case 5: return "r9";
        default: return "stack";
    }
}


char *typeinfo_to_string(AST_TypeInfo typeinfo){
    char a[1000];
    int len = 0;
    strncpy(a, typeinfo.type, strlen(typeinfo.type));
    len = strlen(typeinfo.type);
    a[len+1] = '\0';
    for (int v=0; v<typeinfo.ptrnum; v++){
        strncat(a, "*", 1);
        len++;
    };
    a[len] = '\0';
    return strdup(a);
}


void parser_peek(Parser *parser){
    parser->cur++;
    if (parser->cur > parser->tokenlen){
        error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->tokenlen-1].row, parser->tokens[parser->tokenlen-1].col, parser->tokens[parser->cur].name);
    };
};

char parse_type(Parser *parser, AST_TypeInfo *typeinfo){ // Assumes parser->cur points to the idx of the type 
    /* char c = -1;
    for (int v=0; v<typesLen; v++){
        if (strcmp(types[v].name, parser->tokens[parser->cur].value) == 0){
            c = v;
            break;
        };
    }; */
    char res[100];
    strncpy(res, "", 100);
    if (strcmp(parser->tokens[parser->cur].value, "struct") == 0){
        strcat(res, "struct");
        parser_peek(parser);
    };
    strcat(res, parser->tokens[parser->cur].value);
    typeinfo->type = strdup(res);
    parser->cur++;
    typeinfo->ptrnum = 0;
    while (parser->tokens[parser->cur].type == TOKEN_MUL){
        typeinfo->ptrnum++;
        parser_peek(parser);
    };
    return 0;
}
char is_type(Parser *parser, Token tok){
    char c = -1;
    if (strcmp(tok.value, "struct") == 0){
        return 0;
    }
    for (int v=0; v<typesLen; v++){
        if (strcmp(types[v].name, tok.value) == 0){
            c = v;
            return 0;
        };
    };
    return -1;
};














// Tokenizer
char tokenizer_token(Tokenizer *tokenizer){
    char c = tokenizer->code[tokenizer->cur];
    int prevCol = tokenizer->col;
    int prevRow = tokenizer->line;
    if (c == '\0'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_EOF, "\0", tokenizer->line, tokenizer->col, tokenizer->name};
        return -1;
    }
    if (c == '('){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LP, "(", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == ')'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_RP, ")", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '|'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_PIPE, "|", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '{'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LB, "{", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == ','){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_COMMA, ",", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '}'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_RB, "}", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '='){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_EQ, "=", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == ';'){
    }else if (c == '#'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_HASH, "#", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '$'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_DOLLAR, "$", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '<'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_GT, "<", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '>'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LT, ">", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '+'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_PLUS, "+", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '['){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LBRACKET, "[", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == ']'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_RBRACKET, "]", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '*'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_MUL, "*", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '/'){
        if (tokenizer->code[tokenizer->cur+1] == '/'){ // Comment
            tokenizer->cur += 2;
            while (tokenizer->code[tokenizer->cur] != '\n' && tokenizer->code[tokenizer->cur] != '\0'){
                tokenizer->cur++;
            };
            tokenizer->line++;
            tokenizer->cur++;
            return tokenizer_token(tokenizer);
        }else if (tokenizer->code[tokenizer->cur+1] == '*'){ // Comment
            tokenizer->cur += 2;
            char c = 0;
            while (tokenizer->code[tokenizer->cur] != '\0'){
                if (tokenizer->code[tokenizer->cur] != '/' && c == 1){
                    c = 0;
                }else if (c == 1){
                    break;
                }
                if (tokenizer->code[tokenizer->cur] == '\n'){
                    tokenizer->line++;
                }

                if (tokenizer->code[tokenizer->cur] == '*'){
                    c = 1;
                };
                tokenizer->cur++;
            };
            tokenizer->cur++;
            return tokenizer_token(tokenizer);
        };
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_DIV, "/", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '%'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_MODULO, "%", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if(c == '.'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_DOT, ".", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if(c == '&'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_AMP, "&", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '-'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_SUB, "-", tokenizer->line, tokenizer->col, tokenizer->name};
    }else if (c == '!'){
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_EXC, "!", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '\''){
        tokenizer->cur+=2;
        tokenizer->col += 2;
        char buf[3];
        if (tokenizer->code[tokenizer->cur] != '\0'){
            if (tokenizer->code[tokenizer->cur] != '\''){
                if (tokenizer->code[tokenizer->cur-1] == '\\'){
                    buf[0] = tokenizer->code[tokenizer->cur-1];
                    buf[1] = tokenizer->code[tokenizer->cur];
                    buf[2] = '\0';
                }else {
                    error_generate_parser("AbruptEndError", "Character is too long", tokenizer->line, prevCol, tokenizer->name);
                }
            };
        }else {
            error_generate_parser("AbruptEndError", "Abrupt end", tokenizer->line, prevCol, tokenizer->name);
        }
        if (tokenizer->code[tokenizer->cur] == '\''){
            buf[0] = tokenizer->code[tokenizer->cur-1];
            buf[1] = '\0';
        }else {
            if (strcmp(buf, "\\n") == 0){
                buf[0] = '\n';
                buf[1] = '\0';
            }
            tokenizer->cur++;
            tokenizer->col++;
        }
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_CHAR;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, buf);
        tokenizer->tokens[tokenizer->tokenlen].row = tokenizer->line;
        tokenizer->tokens[tokenizer->tokenlen].col = tokenizer->col;
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
    }else if(c == '\"'){
        tokenizer->cur++;
        tokenizer->col++;
        c = tokenizer->code[tokenizer->cur];
        char string[1000];int stringlen = 0;
        while (c != '\"' && c != '\0'){
            if (c == '\n'){
                error_generate_parser("SyntaxError", "Newlines not allowed inside of strings. Use \\n instead", tokenizer->line, prevCol, tokenizer->name);
            };
            string[stringlen++] = c;
            tokenizer->cur++;
            tokenizer->col++;
            if (c == '\\'){
                if (strlen(tokenizer->code) == tokenizer->cur+1){
                    error_generate_parser("SyntaxError", "Found end of string with \\", tokenizer->line, prevCol, tokenizer->name);
                };
                if (tokenizer->code[tokenizer->cur] == '\"'){
                    string[stringlen++] = '\"';
                    tokenizer->cur++;
                    tokenizer->col++;
                    c = tokenizer->code[tokenizer->cur];
                } else if (tokenizer->code[tokenizer->cur] == 'n') {
                    string[stringlen - 1] = '\n';
                    tokenizer->cur++;
                    tokenizer->col++;
                };
            };
            c = tokenizer->code[tokenizer->cur];
        };
        if (c == '\0'){
            error_generate_parser("SyntaxError", "Abrupt end", tokenizer->line, prevCol, tokenizer->name);
        };
        string[stringlen++] = '\0';
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_STRING;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, string);
        tokenizer->tokens[tokenizer->tokenlen].row = tokenizer->line;
        tokenizer->tokens[tokenizer->tokenlen].col = tokenizer->col;
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;
        tokenizer->tokenlen++;
    }else if(isalpha(c) || c == '_'){
        char res[200];int reslen=0;
        int _isSyscall = 0;

        while(true){
            if (strncmp(res, "syscall", strlen("syscall")) == 0){
                _isSyscall = 1;
            }
            bool cond = !isalnum(c) && c != '_';
            if (_isSyscall){
                cond = cond && c != '.';
            }
            if (cond){
                break;
            };
            res[reslen++] = c;
            tokenizer->cur++;
            tokenizer->col++;
            c = tokenizer->code[tokenizer->cur];
        };
        res[reslen] = '\0';
        tokenizer->cur--;
        tokenizer->col--;
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_ID;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
        tokenizer->tokens[tokenizer->tokenlen].row = tokenizer->line;
        tokenizer->tokens[tokenizer->tokenlen].col = tokenizer->col;
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
        if (strcmp(tokenizer->tokens[tokenizer->tokenlen-1].value, "true") == 0){
            tokenizer->tokens[tokenizer->tokenlen-1].type = TOKEN_INT;
            strcpy(tokenizer->tokens[tokenizer->tokenlen-1].value, "true");
        }else if (strcmp(tokenizer->tokens[tokenizer->tokenlen-1].value, "false") == 0){
            tokenizer->tokens[tokenizer->tokenlen-1].type = TOKEN_INT;
            strcpy(tokenizer->tokens[tokenizer->tokenlen-1].value, "false");
        };
    }else if(isnumber(c)){
        char res[200];int reslen=0;
        while (isnumber(c)){
            res[reslen++] = c;
            tokenizer->cur++;
            tokenizer->col++;
            c = tokenizer->code[tokenizer->cur];
        };
        res[reslen] = '\0';
        if (tokenizer->code[tokenizer->cur] == 'f'){
            tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_FLOAT;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
            tokenizer->tokens[tokenizer->tokenlen].row = tokenizer->line;
            tokenizer->tokens[tokenizer->tokenlen].col = tokenizer->col;
            tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
        }else {
            tokenizer->cur--;
            tokenizer->col--;
            tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_INT;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
            tokenizer->tokens[tokenizer->tokenlen].row = tokenizer->line;
            tokenizer->tokens[tokenizer->tokenlen].col = tokenizer->col;
            tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
        }
    }else if (c == '\n') {
        tokenizer->col = 0;
        tokenizer->line++;
        tokenizer->cur++;
        tokenizer->col++;
        return 0;
    }else if(c == ' ' || c == '\t'){
        tokenizer->cur++;
        tokenizer->col++;
        return 0;
    } else {
        error_generate_parser("SyntaxError", "Found unknown token", tokenizer->line, prevCol, tokenizer->name);
    };
    tokenizer->tokens[tokenizer->tokenlen-1].col = prevCol;
    tokenizer->tokens[tokenizer->tokenlen-1].row = prevRow;
    tokenizer->cur++;
    tokenizer->col++;
    return 0;
};
void tokenizer_free(Tokenizer *tok) {
    free(tok->code);
    free(tok->tokens);
    free(tok);
}
void rem_idx(Token arr[], int *len, int index) {
    if (index < 0 || index >= *len) {
        printf("Invalid index\n");
        return;
    }
    
    for (int i = index; i < *len - 1; i++) {
        arr[i] = arr[i + 1];
    }
    (*len)--;
}

void add_idx(Token arr[], int *len, int index) {
    if (index < 0 || index >= *len) {
        printf("Index out of bounds.\n");
        return;
    }

    for (int i = *len - 1; i > index; i--) {
        arr[i] = arr[i - 1];
    }

    arr[index] = (Token){};
    (*len)++;
}


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
    ast->next = NULL;
    
    if (parser->astlen == 0){
        parser->asts = ast;
    } else {
        AST *ast1 = parser->asts;
        for (int i=0; i<parser->astlen-1; i++){
            ast1 = ast1->next;
        };
        ast1->next = ast;
    };
    parser->astlen++;
}
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
AST *parser_eat_expr(Parser *parser);

AST *mode_parse_expr(Parser *parser){
    AST *ast = malloc(sizeof(AST));
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
            char name[500];
            strcpy(name, "");
            parser->cur++;
            while (parser->tokens[parser->cur].type != TOKEN_EOF && parser->tokens[parser->cur].type != TOKEN_EQ){
                strncat(name, parser->tokens[parser->cur].value, 100);
                if (strcmp(name, "extern") == 0){
                    break;
                }else if (strcmp(name, "optimize") == 0){
                    break;
                };
                parser->cur++;
            }
            ast->data.mode.name = strdup(name);
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
bool is_immediate(AST *ast){
    switch (ast->type){
        case AST_INT: return true;
        case AST_CAST: return is_immediate(ast->data.expr.left);
        default: return false;
    }

    return false;
};

AST *parser_eat_expr(Parser *parser){
    Token token_1 = parser->tokens[parser->cur];
    int a = parser->cur;
    AST *ast = malloc(sizeof(AST));
    ast->row = parser->tokens[a].row;
    ast->col = parser->tokens[a].col;
    ast->filename = parser->tokens[a].name;
    if (try_parse_mode(parser, ast) == 0){
        return ast;
    };
    ast->type = AST_UNKNOWN;
    if (parser->tokens[parser->cur].type == TOKEN_LP){ // Assume it is an expression
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
        ast->typeinfo.type = "char";
        ast->typeinfo.ptrnum = 1;
        ast->data.arg.value = parser->tokens[parser->cur].value;
        parser_peek(parser);
    }else if(parser->tokens[parser->cur].type == TOKEN_SUB){
        parser_peek(parser);
        if (parser->tokens[parser->cur].type == TOKEN_INT){
            ast->type = AST_INT;
            ast->typeinfo.type = "const";
            char a[500];
            snprintf(a, 500, "-%s", parser->tokens[parser->cur].value);
            ast->data.arg.value = strdup(a);
            parser->cur++;
        }else {
            error_generate_parser("AbruptEndError", "Found negative sign alone", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->name);
        }
    }else if (parser->tokens[parser->cur].type == TOKEN_INT){
        ast->type = AST_INT;
        ast->typeinfo.type = "const";
        ast->data.arg.value = parser->tokens[parser->cur].value;
        if (strcmp(parser->tokens[parser->cur].value, "true") == 0){
            ast->data.arg.value = "1";
            ast->typeinfo.type = "bool";
        }else if (strcmp(parser->tokens[parser->cur].value, "false") == 0){
            ast->data.arg.value = "0";
            ast->typeinfo.type = "bool";
        }
        parser_peek(parser);
    }else if (parser->tokens[parser->cur].type == TOKEN_FLOAT){
        ast->type = AST_FLOAT;
        ast->typeinfo.type = "float";
        ast->data.arg.value = parser->tokens[parser->cur].value;
        parser_peek(parser);
    }else if (parser->tokens[parser->cur].type == TOKEN_CHAR){
        ast->type = AST_CHAR;
        ast->typeinfo.type = "char";
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
                return ast;
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

    if (parser->tokens[parser->cur].type == TOKEN_ID){
        if (parser->tokens[parser->cur].value[0] == 'x' && strcmp(ast->data.arg.value, "0") == 0){
            char decimal_string[20];
            strncpy(decimal_string, "0", 20);
            strncat(decimal_string, parser->tokens[parser->cur].value, 19);
            ast->data.arg.value = strdup(decimal_string);
            parser_peek(parser);
        };
    }

    if (parser->tokens[parser->cur].type == TOKEN_DOT){
        AST *ast2 = malloc(sizeof(AST));
        ast2->type = AST_ACCESS;
        ast2->row = parser->tokens[parser->cur].row;
        ast2->col = parser->tokens[parser->cur].col;
        ast2->filename = parser->tokens[parser->cur].name;
        ast2->data.expr.left = ast;
        parser_peek(parser);
        
        if (parser->tokens[parser->cur].type != TOKEN_ID) {
            error_generate_parser("SyntaxError", "Expected identifier after '.'", 
                                parser->tokens[parser->cur].row, 
                                parser->tokens[parser->cur].col, 
                                parser->name);
        }
        
        ast2->data.expr.right = malloc(sizeof(AST));
        ast2->data.expr.right->type = AST_VAR;
        ast2->data.expr.right->data.arg.value = parser->tokens[parser->cur].value;
        parser_peek(parser);
        
        ast = ast2;
    }
    if (parser->tokens[parser->cur].type == TOKEN_LBRACKET){
        AST *ast2 = malloc(sizeof(AST));
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
        ast2->type = AST_MUL;
        ast2->data.expr.left = ast;
        if (parser->cur + 1 == parser->tokenlen){
            error_generate("AbruptEndError", "Abrupt end in multiplication");
        };
        parser_peek(parser);
        if (parser->tokens[parser->cur+1].type == TOKEN_EQ){ // Only types supported
            parser->cur--;
            free(ast2);
            return ast;
        }
        ast2->data.expr.right = parser_eat_expr(parser);
        ast = rotate_to_left(ast2);
    }
    if (parser->tokens[parser->cur].type == TOKEN_DIV){
        AST *ast2 = malloc(sizeof(AST));
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
            ast2->type = AST_NEQ;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast2->typeinfo = (AST_TypeInfo){"bool", 0};
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
            ast2->type = AST_AND;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast2->typeinfo = (AST_TypeInfo){"bool", 0};
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
            ast2->type = AST_OR;
            ast2->data.expr.left = ast;
            if (parser->cur + 1 == parser->tokenlen){
                error_generate("AbruptEndError", "Abrupt end");
            };
            parser_peek(parser);
            ast2->data.expr.right = parser_eat_expr(parser);
            ast2->typeinfo = (AST_TypeInfo){"bool", 0};
            ast = ast2;
        }else {
            error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->name);
        }
    }
    // Position and filename already set at function start
    return ast;
};

AST *parser_eat_ast(Parser *parser){
    int a = parser->cur;
    AST *ast = malloc(sizeof(AST));
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

            ast->data.if1.block.statements = malloc(sizeof(AST));
            ast->data.if1.block.statementlen = 0;
            ast->data.if1.else1 = malloc(sizeof(AST));
            ast->data.if1.elseif = malloc(sizeof(Block) * 100);
            ast->data.if1.elselen = 0;
            ast->data.if1.elseiflen = 0;

            Token prev = {0};
            int c = 0;

            AST *start = NULL;
            AST *_current = NULL;
            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                if (parser->tokens[parser->cur].type == prev.type && strcmp(parser->tokens[parser->cur].value, prev.value) == 0){
                    if (++c > 10000) error_generate_parser("SyntaxError", "Infinite loop detected", parser->tokens[parser->cur].row, parser->tokens[parser->cur].col, parser->tokens[parser->cur].name);
                } else {
                    c = 0;
                }
                AST* new_node = malloc(sizeof(AST));
                *new_node = *(parser_eat_ast(parser));
                if (start == NULL) {
                    start = new_node;
                    _current = new_node;
                } else {
                    _current->next = new_node;
                    _current = new_node;
                }
                ast->data.if1.block.statementlen++;
                prev = parser->tokens[parser->cur];
            }
            ast->data.if1.block.statements = start;
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
                    start = NULL;
                    _current = NULL;
                    while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                        AST* new_node = malloc(sizeof(AST));
                        *new_node = *(parser_eat_ast(parser));
                        if (start == NULL) {
                            start = new_node;
                            _current = new_node;
                        } else {
                            _current->next = new_node;
                            _current = new_node;
                        }
                        ast->data.if1.elseif[ei].statementlen++;
                    }
                    ast->data.if1.elseif[ei].statements = start;
                    ast->data.if1.elseiflen++;
                    parser_expect(parser, TOKEN_RB);
                } else {
                    parser_expect(parser, TOKEN_LB);
                    prev = (Token){0};
                    start = NULL;
                    _current = NULL;
                    while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                        AST* new_node = malloc(sizeof(AST));
                        *new_node = *(parser_eat_ast(parser));
                        if (start == NULL) {
                            start = new_node;
                            _current = new_node;
                        } else {
                            _current->next = new_node;
                            _current = new_node;
                        }
                        ast->data.if1.elselen++;
                    }
                    ast->data.if1.else1 = start;
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
                Token prev;
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
                ast->data.assign.alias = false;
                ast->data.assign.from = parser_eat_expr(parser); // Even though we technically don't expect it to be expression, this is used for consistency
                ast->data.assign.from->typeinfo = ast->typeinfo;
                if (parser->tokens[parser->cur].type == TOKEN_EQ){
                    parser_peek(parser);
                    ast->data.assign.assignto = parser_eat_expr(parser);
                    if (is_immediate(ast->data.assign.assignto)){
                        ast->data.assign.alias = true;
                    }
                }else {
                    ast->data.assign.assignto = malloc(sizeof(*(ast->data.assign.assignto)));
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
                char *str = parser->tokens[parser->cur].value;
                AST *ast_expr = parser_eat_expr(parser);
                if(parser->tokens[parser->cur].type == TOKEN_EQ){
                    ast->type = AST_ASSIGN;
                    ast->row = parser->tokens[parser->cur].row;
                    ast->col = parser->tokens[parser->cur].col;
                    ast->filename = parser->tokens[parser->cur].name;
                    ast->typeinfo.type = "unknown";
                    ast->data.assign.from = ast_expr;
                    parser_peek(parser);
                    ast->data.assign.assignto = parser_eat_expr(parser);
                    return ast;
                }else if (parser->tokens[parser->cur].type == TOKEN_LP){
                    ast->type = AST_FUNCALL;
                    ast->data.funcall = (AST_FuncCall){};
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
            ast->typeinfo.type = "unknown";
            ast->data.assign.new = false;
            ast->data.assign.from = ast_expr;
            parser_peek(parser);
            ast->data.assign.assignto = parser_eat_expr(parser);
            return ast;
        }else if(parser->tokens[parser->cur].type == TOKEN_LP){

        }else {
            error_generate_parser("AbruptEndError", "Abrupt end", parser->tokens[parser->cur+2].row, parser->tokens[parser->cur+2].col, parser->tokens[parser->cur].name);
       }
    }

    return ast;
}

Field *create_field(char *name, AST_TypeInfo type){
    Field *field = malloc(sizeof(Field));
    field->type = type;
    field->name = name;
    return field;
}

AST parser_eat_body(Parser *parser){
    int av = parser->cur;
    AST ast = (AST){};
    ast.row = parser->tokens[av].row;
    ast.col = parser->tokens[av].col;
    ast.filename = parser->tokens[av].name;
    if (try_parse_mode(parser, &ast) == 0){
        return ast;
    };
    ast.type = AST_UNKNOWN;
    if (parser->tokens[parser->cur].type == TOKEN_ID){
        if(strcmp(parser->tokens[parser->cur].value, "struct") == 0){
            ast.type = AST_STRUCT;
            parser_expect(parser, TOKEN_ID);
            ast.data.struct1.name = strdup(parser->tokens[parser->cur].value);
            parser_expect(parser, TOKEN_ID);
            parser_expect(parser, TOKEN_LB);
            ast.data.struct1.fields = malloc(sizeof(Field*) * 100);
            ast.data.struct1.fieldlen = 0;
            char strin[100];
            snprintf(strin, 100, "struct%s", ast.data.struct1.name);
            types[typesLen++] = (struct Pair){strin, 0, "Structure"};
            while (parser->tokens[parser->cur].type != TOKEN_RB && parser->tokens[parser->cur].type != TOKEN_EOF){
                AST_TypeInfo *type = malloc(sizeof(AST_TypeInfo));
                parse_type(parser, type);
                char *field_name = parser->tokens[parser->cur].value;
                parser_peek(parser);
                
                ast.data.struct1.fields[ast.data.struct1.fieldlen++] = create_field(field_name, *type);
            };
            parser_expect(parser, TOKEN_RB);
        }else if(is_type(parser, parser->tokens[parser->cur]) == 0){
            ast.type = AST_ASSIGN;
            parse_type(parser, &ast.typeinfo);
            ast.data.assign.from = malloc(sizeof(struct AST));
            ast.data.assign.from->type = AST_VAR;
            ast.data.assign.from->data.arg.value = parser->tokens[parser->cur].value;
            parser_peek(parser);
            if (parser->tokens[parser->cur].type == TOKEN_EQ){
                parser_peek(parser);
                ast.data.assign.assignto = parser_eat_expr(parser);
            }else if(parser->tokens[parser->cur].type == TOKEN_LP){
                ast.type = AST_FUNCDEF;
                ast.data.funcdef.args = malloc(sizeof(Argument*) * 100);
                ast.data.funcdef.block = malloc(sizeof(AST*) * 200);
                ast.data.funcdef.name = ast.data.assign.from->data.arg.value;
                parser_peek(parser);
            while (parser->tokens[parser->cur].type != TOKEN_RP && parser->tokens[parser->cur].type != TOKEN_EOF){
                ast.data.funcdef.args[ast.data.funcdef.argslen] = malloc(sizeof(Argument));
                parse_type(parser, &(ast.data.funcdef.args[ast.data.funcdef.argslen]->type));
                ast.data.funcdef.args[ast.data.funcdef.argslen]->arg = parser->tokens[parser->cur].value;
                ast.data.funcdef.argslen++;
                parser_peek(parser);
                if (parser->tokens[parser->cur].type != TOKEN_RP){
                    parser_expect(parser, TOKEN_COMMA);
                };
            };
            parser_expect(parser, TOKEN_RP);
            if (parser->tokens[parser->cur].type == TOKEN_LB){
                parser_expect(parser, TOKEN_LB);
                Token prev;
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
                    ast.data.funcdef.block[ast.data.funcdef.blocklen++] = parser_eat_ast(parser);
                    prev = parser->tokens[parser->cur-1];
                };
                parser_expect(parser, TOKEN_RB);
            }else {
                ast.data.funcdef.blocklen = -1;
            }
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
    Token token = parser->tokens[parser->cur];
    AST *ast = malloc(sizeof(AST));
    *ast = parser_eat_body(parser);
    ast->row = token.row;
    ast->col = token.col;
    store_ast(parser, ast);
    return 0;
};


#include "typechecker.c"
#include "generator.c"
#include "ir.c"
#include "preprocessor.c"


int main(int argc, char **argv){
    struct timespec __begin;
    clock_gettime(CLOCK_MONOTONIC, &__begin);
    argv++;
    char *input_file = "";
    char *output_file = "";
    while (*argv){
        if (*argv[0] == '-'){ // Settings command
            if(strcmp(*argv, "-o") == 0){
                argv++;
                output_file = *argv;
            }else if(strcmp(*argv, "-h") == 0){
                printf("%s", HELP);
            }
        } else {
            input_file = *argv;
        };
        argv++;
    };

    if (strcmp(output_file, "") == 0){
        output_file = "res/main";
    };


    if (!input_file){
        printf("\x1b[1;31merror\x1b[0m: No input file provided");
        return -1;
    };
    struct timespec __start, __end;

    clock_gettime(CLOCK_MONOTONIC, &__start);

    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));
    FILE *f = fopen(input_file, "r");
    if (f == NULL){
        printf("\x1b[1;31merror\x1b[0m: Invalid input file provided\n");
        return -1;
    }
    tokenizer->line = 1;
    tokenizer->name = input_file;
    tokenizer->col = 0;
    tokenizer->code = malloc(90000);
    size_t size = fread(tokenizer->code, 1, 90000, f);
    tokenizer->code[size] = '\0';
    source_files[num_source_files++] = (SourceFile){tokenizer->name, tokenizer->code, size};
    fclose(f);
    tokenizer->cur = 0;
    tokenizer->tokens = malloc(sizeof(Token) * 9000);
    tokenizer->tokenlen = 0;

    while(tokenizer_token(tokenizer) != -1){
    };

    clock_gettime(CLOCK_MONOTONIC, &__end);
    fprintf(stdout, "[INFO] Tokenizer process took %.4f milliseconds\n", (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6);
    clock_gettime(CLOCK_MONOTONIC, &__start);

    preprocess(input_file, tokenizer);

    clock_gettime(CLOCK_MONOTONIC, &__end);
    fprintf(stdout, "[INFO] Preprocessor process took %.4f milliseconds\n", (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6);
    Parser *parser = malloc(sizeof(Parser));
    parser->name = tokenizer->name;
    parser->asts = NULL;
    parser->astlen = 0;
    parser->cur = 0;
    parser->tokens = tokenizer->tokens;
    parser->tokens[parser->cur].name = tokenizer->name;
    parser->tokenlen = tokenizer->tokenlen;
    while (parser_eat(parser) != -1){};


    clock_gettime(CLOCK_MONOTONIC, &__end);
    fprintf(stdout, "[INFO] Parsing process took %.4f milliseconds\n", (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6);

    clock_gettime(CLOCK_MONOTONIC, &__start);

    Typechecker *typechecker = typechecker_init(parser);
    while (typechecker_eat_ast(typechecker) != -1){
    };
    typechecker_close(typechecker);
    
    clock_gettime(CLOCK_MONOTONIC, &__end);
    fprintf(stdout, "[INFO] Typechecking process took %.4f milliseconds\n", (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6);


    clock_gettime(CLOCK_MONOTONIC, &__start);
    Generator *generator = generator_init(typechecker, output_file, ir);
    while (generator_eat_ast(generator) != -1){};
    clock_gettime(CLOCK_MONOTONIC, &__end);
    fprintf(stdout, "[INFO] Transpiling process took %.4f milliseconds\n", (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6);


    tokenizer_free(tokenizer);
    free(generator);
    AST *current = parser->asts;
    while (current != NULL) {
        AST *next = current->next;
        free(current);
        current = next;
    }
    free(parser);
    free(typechecker);
    fprintf(stdout, "[INFO] Program took %.4f milliseconds\n", (__end.tv_sec - __begin.tv_sec) * 1000.0 +
                        (__end.tv_nsec - __begin.tv_nsec) / 1e6);
    
    return 0;
}
