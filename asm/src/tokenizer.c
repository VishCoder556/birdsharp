Tokenizer *tokenizer_init(char *file){
    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));
    FILE *f = fopen(file, "r");
    if (f == NULL){
        error_generate("FileError", "File not found");
    }
    tokenizer->line = 1;
    tokenizer->name = file;
    tokenizer->col = 0;
    tokenizer->code = malloc(90000);
    size_t size = fread(tokenizer->code, 1, 90000, f);
    tokenizer->code[size] = '\0';
    fclose(f);
    tokenizer->cur = 0;
    tokenizer->tokens = malloc(sizeof(Token) * 9000);
    tokenizer->tokenlen = 0;
    return tokenizer;
}


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
        case TOKEN_FLOAT: return "floating point";
        case TOKEN_DOT: return "'.'";
        case TOKEN_PIPE: return "'|'";
    };
    return "unknown";
};

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
                    error_generate_tokenizer(tokenizer, "AbruptEndError", "Character is too long", tokenizer->line, prevCol, tokenizer->name);
                }
            };
        }else {
            error_generate_tokenizer(tokenizer, "AbruptEndError", "Abrupt end", tokenizer->line, prevCol, tokenizer->name);
        }
        if (tokenizer->code[tokenizer->cur] == '\''){
            buf[0] = tokenizer->code[tokenizer->cur-1];
            buf[1] = '\0';
        }else {
            if (string_compare(buf, "\\n", string_len(buf)) == 0){
                buf[0] = '\n';
                buf[1] = '\0';
            }
            tokenizer->cur++;
            tokenizer->col++;
        }
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_CHAR;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, buf);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
    }else if(c == '\"'){
        tokenizer->cur++;
        tokenizer->col++;
        c = tokenizer->code[tokenizer->cur];
        char string[1000];int stringlen = 0;
        while (c != '\"' && c != '\0'){
            if (c == '\n'){
                error_generate_tokenizer(tokenizer, "SyntaxError", "Newlines not allowed inside of strings. Use \\n instead", tokenizer->line, prevCol, tokenizer->name);
            };
            string[stringlen++] = c;
            tokenizer->cur++;
            tokenizer->col++;
            if (c == '\\'){
                if (string_len(tokenizer->code) == tokenizer->cur+1){
                    error_generate_tokenizer(tokenizer, "SyntaxError", "Found end of string with \\", tokenizer->line, prevCol, tokenizer->name);
                };
                if (tokenizer->code[tokenizer->cur] == '\"'){
                    string[stringlen++] = '\"';
                    tokenizer->cur++;
                    tokenizer->col++;
                    c = tokenizer->code[tokenizer->cur];
                }else if (tokenizer->code[tokenizer->cur] == 'n') {
                    string[stringlen - 1] = '\n';
                    tokenizer->cur++;
                    tokenizer->col++;
                };
            };
            c = tokenizer->code[tokenizer->cur];
        };
        if (c == '\0'){
            error_generate_tokenizer(tokenizer, "SyntaxError", "Abrupt end", tokenizer->line, prevCol, tokenizer->name);
        };
        string[stringlen++] = '\0';
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_STRING;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, string);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;
        if (tokenizer->tokenlen <= 2){
            if (tokenizer->tokens[tokenizer->tokenlen-2].type == TOKEN_MODULO && tokenizer->tokens[tokenizer->tokenlen-1].type == TOKEN_ID && string_compare(tokenizer->tokens[tokenizer->tokenlen-1].value, "include", strlen("include")) == 0){
                tokenizer->tokenlen -= 2;
                Tokenizer *tokenizer2 = tokenizer_init(string);
                while(tokenizer_token(tokenizer2) != -1){};
                for (int n=0; n<tokenizer2->tokenlen; n++){
                    tokenizer->tokens[tokenizer->tokenlen++] = tokenizer2->tokens[n];
                };
                tokenizer->tokenlen -= 2;
                free(tokenizer2);
            };
        };
        tokenizer->tokenlen++;
    }else if(isalpha(c) || c == '_'){
        char res[200];int reslen=0;

        while (isalnum(c) || c == '_'){
            res[reslen++] = c;
            tokenizer->cur++;
            tokenizer->col++;
            c = tokenizer->code[tokenizer->cur];
        };
        res[reslen] = '\0';
        tokenizer->cur--;
        tokenizer->col--;
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_ID;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
        if (string_compare(tokenizer->tokens[tokenizer->tokenlen-1].value, "true", 4) == 0){
            tokenizer->tokens[tokenizer->tokenlen-1].type = TOKEN_INT;
            strcpy(tokenizer->tokens[tokenizer->tokenlen-1].value, "true");
        }else if (string_compare(tokenizer->tokens[tokenizer->tokenlen-1].value, "false", 5) == 0){
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
            tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;tokenizer->tokenlen++;
        }else {
            tokenizer->cur--;
            tokenizer->col--;
            tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_INT;strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
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
        error_generate_tokenizer(tokenizer, "SyntaxError", "Found unknown token", tokenizer->line, prevCol, tokenizer->name);
    };
    tokenizer->tokens[tokenizer->tokenlen-1].col = prevCol;
    tokenizer->tokens[tokenizer->tokenlen-1].row = prevRow;
    tokenizer->cur++;
    tokenizer->col++;
    return 0;
};
