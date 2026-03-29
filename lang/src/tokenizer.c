#include "include/tokenizer.h"

void error_generate_parser(char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;31m%s\x1b[0m: %s\n", f, l, c, type, name);
    for (int i=0; i<num_source_files; i++){
        if (strcmp(source_files[i].file_path, f) == 0){
            print_error_line(source_files[i].code, l, c);
        };
    };
    exit(-1);
};

void tokenizer_ensure_capacity(Tokenizer *tokenizer, int required_len) {
    if (required_len < tokenizer->tokencap) return;

    int new_cap = tokenizer->tokencap == 0 ? 32 : tokenizer->tokencap;
    while (new_cap <= required_len) {
        new_cap *= 2;
    }

    Token *new_tokens = realloc(tokenizer->tokens, new_cap * sizeof(Token));
    if (new_tokens) {
        tokenizer->tokens = new_tokens;
        tokenizer->tokencap = new_cap;
    }
}

void tokenizer_insert_at(Tokenizer *tokenizer, int index, Token *new_tokens, int count, int tokens_to_remove) {
    if (index < 0 || index > tokenizer->tokenlen || count < 0 || tokens_to_remove < 0) {
        return;
    }
    
    int new_total_len = tokenizer->tokenlen + count - tokens_to_remove;
    if (new_total_len < 0) {
        return;
    }
    
    tokenizer_ensure_capacity(tokenizer, new_total_len);

    int tail_start = index + tokens_to_remove;
    int tail_len = tokenizer->tokenlen - tail_start;
    
    if (tail_len > 0) {
        memmove(&tokenizer->tokens[index + count], 
                &tokenizer->tokens[tail_start], 
                tail_len * sizeof(Token));
    }

    if (count > 0) {
        memcpy(&tokenizer->tokens[index], new_tokens, count * sizeof(Token));
    }

    tokenizer->tokenlen = new_total_len;
}

void tokenizer_remove_at(Tokenizer *tokenizer, int index, int count) {
    if (!tokenizer || index < 0 || count <= 0 || index + count > tokenizer->tokenlen) {
        return;
    }

    int tail_start = index + count;
    int tail_len = tokenizer->tokenlen - tail_start;

    if (tail_len > 0) {
        memmove(&tokenizer->tokens[index], 
                &tokenizer->tokens[tail_start], 
                tail_len * sizeof(Token));
    }

    tokenizer->tokenlen -= count;
}

void __tokenizer_append(Tokenizer *tokenizer, Token tok) {
    tokenizer_ensure_capacity(tokenizer, tokenizer->tokenlen);
    tokenizer->tokens[tokenizer->tokenlen++] = tok;
}

void _tokenizer_append(Tokenizer *tokenizer, int type, char *name, int row, int col) {
    Token tok = (Token){0};
    tok.type = type;
    strncpy(tok.value, name, 89);
    tok.value[89] = '\0';
    tok.row = row;
    tok.col = col;
    tok.name = tokenizer->name;

    __tokenizer_append(tokenizer, tok);
}

#define tokenizer_append(tokenizer, type, name) _tokenizer_append(tokenizer, type, name, prevRow, prevCol)


char tokenizer_token(Tokenizer *tokenizer){
    char c = tokenizer->code[tokenizer->cur];
    int prevCol = tokenizer->col;
    int prevRow = tokenizer->line;
    if (c == '\0'){
        tokenizer_append(tokenizer, TOKEN_EOF, "\0");
        return -1;
    }
    if (c == '('){
        tokenizer_append(tokenizer, TOKEN_LP, "(");
    }else if (c == ')'){
        tokenizer_append(tokenizer, TOKEN_RP, ")");
    }else if (c == '|'){
        tokenizer_append(tokenizer, TOKEN_PIPE, "|");
    }else if (c == '{'){
        tokenizer_append(tokenizer, TOKEN_LB, "{");
    }else if (c == ','){
        tokenizer_append(tokenizer, TOKEN_COMMA, ",");
    }else if (c == '}'){
        tokenizer_append(tokenizer, TOKEN_RB, "}");
    }else if (c == '='){
        tokenizer_append(tokenizer, TOKEN_EQ, "=");
    }else if (c == ';'){
        tokenizer_append(tokenizer, TOKEN_SEMICOLON, ";");
    }else if (c == '#'){
        tokenizer_append(tokenizer, TOKEN_HASH, "#");
    }else if (c == '$'){
        tokenizer_append(tokenizer, TOKEN_DOLLAR, "$");
    }else if (c == '<'){
        tokenizer_append(tokenizer, TOKEN_LT, "<");
    }else if (c == '>'){
        tokenizer_append(tokenizer, TOKEN_GT, ">");
    }else if (c == '+'){
        tokenizer_append(tokenizer, TOKEN_PLUS, "+");
    }else if (c == '['){
        tokenizer_append(tokenizer, TOKEN_LBRACKET, "[");
    }else if (c == ']'){
        tokenizer_append(tokenizer, TOKEN_RBRACKET, "]");
    }else if (c == '*'){
        tokenizer_append(tokenizer, TOKEN_MUL, "*");
    }else if (c == '/'){
        if (tokenizer->code[tokenizer->cur+1] == '/'){
            tokenizer->cur += 2;
            while (tokenizer->code[tokenizer->cur] != '\n' && tokenizer->code[tokenizer->cur] != '\0'){
                tokenizer->cur++;
            };
            tokenizer->line++;
            tokenizer->cur++;
            return tokenizer_token(tokenizer);
        }else if (tokenizer->code[tokenizer->cur+1] == '*'){
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
        tokenizer_append(tokenizer, TOKEN_DIV, "/");
    }else if (c == '%'){
        tokenizer_append(tokenizer, TOKEN_MODULO, "%");
    }else if(c == '.'){
        tokenizer_append(tokenizer, TOKEN_DOT, ".");
    }else if(c == '&'){
        tokenizer_append(tokenizer, TOKEN_AMP, "&");
    }else if (c == '-'){
        tokenizer_append(tokenizer, TOKEN_SUB, "-");
    }else if (c == '!'){
        tokenizer_append(tokenizer, TOKEN_EXC, "!");
    } else if (c == '\'') {
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
        tokenizer_append(tokenizer, TOKEN_CHAR, buf);
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
        tokenizer_append(tokenizer, TOKEN_STRING, string);
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
        if (strcmp(res, "true") == 0){
            tokenizer_append(tokenizer, TOKEN_INT, "true");
        }else if (strcmp(res, "false") == 0){
            tokenizer_append(tokenizer, TOKEN_INT, "true");
        }else if(strcmp(res, "__ir__") == 0 && tokenizer->tokenlen >= 2){
            if (tokenizer->tokens[tokenizer->tokenlen-1].type == TOKEN_EXC && tokenizer->tokens[tokenizer->tokenlen-2].type == TOKEN_HASH){
                tokenizer->cur++;
                tokenizer->col++;
                tokenizer_token(tokenizer);
                if (tokenizer->tokens[tokenizer->tokenlen-1].type != TOKEN_LP){
                    error_generate_parser("SyntaxError", "Unknown IR block found", tokenizer->line, prevCol, tokenizer->name);
                }else {
                    tokenizer->tokenlen -= 3; // Remove them
                    int stringcap = 200;
                    fflush(stdout);
                    char *string = malloc(stringcap);
                    int stringlen = 0;
                    while (tokenizer->code[tokenizer->cur] != ')'){
                        if (stringlen > stringcap){
                            stringcap += 50;
                            string = realloc(string, stringcap);
                        };
                        string[stringlen++] = tokenizer->code[tokenizer->cur];
                        tokenizer->cur++;
                        tokenizer->col++;
                    };
                    tokenizer_append(tokenizer, TOKEN_IR, string);
                };
            };
        }else {
            tokenizer_append(tokenizer, TOKEN_ID, res);
        }
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
            tokenizer_append(tokenizer, TOKEN_FLOAT, res);
        }else {
            tokenizer->cur--;
            tokenizer->col--;
            tokenizer_append(tokenizer, TOKEN_INT, res);
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
    tokenizer->cur++;
    tokenizer->col++;
    return 0;
};



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


Tokenizer *tokenizer_init(char *input_file, int is_input_file){
    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));
    FILE *f = fopen(input_file, "r");
    if (f == NULL){
        if (is_input_file){
            printf("\x1b[1;31merror\x1b[0m: Invalid input file '%s' provided\n", input_file);
        }else {
            printf("\x1b[1;31merror\x1b[0m: Included file '%s' could not be found\n", input_file);
        }
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    tokenizer->line = 1;
    tokenizer->name = input_file;
    tokenizer->col = 0;
    tokenizer->code = malloc(fsize + 1);
    size_t size = fread(tokenizer->code, 1, fsize, f);
    tokenizer->code[size] = '\0';
    source_files[num_source_files++] = (SourceFile){tokenizer->name, tokenizer->code, size};
    fclose(f);
    tokenizer->cur = 0;
    tokenizer->tokencap = 900;
    tokenizer->tokens = malloc(sizeof(Token) * tokenizer->tokencap);
    tokenizer->tokenlen = 0;
    return tokenizer;
}

void tokenizer_free_tokens(Tokenizer *tokenizer) {
    if (tokenizer && tokenizer->tokens) {
        free(tokenizer->tokens);
        tokenizer->tokens = NULL;
        tokenizer->tokenlen = 0;
        tokenizer->tokencap = 0;
    }
}
void tokenizer_free_code(Tokenizer *tokenizer) {
    if (tokenizer && tokenizer->code) {
        free(tokenizer->code);
    }
};
void tokenizer_free(Tokenizer *tokenizer) {
    if (tokenizer) {
        free(tokenizer);
    }
};
