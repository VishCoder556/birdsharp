#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *token_to_string(int tokentype) {
    switch (tokentype) {
        case TOKEN_EOF:      return "end of file";
        case TOKEN_STRING:   return "string";
        case TOKEN_CHAR:     return "character";
        case TOKEN_ID:       return "identifier";
        case TOKEN_LP:       return "'('";
        case TOKEN_RP:       return "')'";
        case TOKEN_LB:       return "'{'";
        case TOKEN_RB:       return "'}'";
        case TOKEN_LBRACKET: return "'['";
        case TOKEN_RBRACKET: return "']'";
        case TOKEN_HASH:     return "'#'";
        case TOKEN_EXC:      return "'!'";
        case TOKEN_COMMA:    return "','";
        case TOKEN_INT:      return "integer";
        case TOKEN_EQ:       return "'='";
        case TOKEN_DOLLAR:   return "'$'";
        case TOKEN_GT:       return "'>'";
        case TOKEN_LT:       return "'<'";
        case TOKEN_PLUS:     return "'+'";
        case TOKEN_SUB:      return "'-'";
        case TOKEN_MUL:      return "'*'";
        case TOKEN_DIV:      return "'/'";
        case TOKEN_COLON:    return "':'";
        case TOKEN_FLOAT:    return "floating point";
        case TOKEN_DOT:      return "'.'";
        case TOKEN_PIPE:     return "'|'";
        case TOKEN_AMP:      return "'&'";
        case TOKEN_MODULO:   return "'%'";
        default:             return "unknown";
    }
}

void tokenizer_ensure_capacity(Tokenizer *tokenizer) {
    if (tokenizer->tokenlen >= tokenizer->tokencap) {
        tokenizer->tokencap = (tokenizer->tokencap == 0) ? 9000 : tokenizer->tokencap * 2;
        Token *new_tokens = realloc(tokenizer->tokens, sizeof(Token) * tokenizer->tokencap);
        if (!new_tokens) exit(1);
        tokenizer->tokens = new_tokens;
    }
}

Tokenizer *tokenizer_init(char *file) {
    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        error_generate("FileError", "File not found");
    }
    tokenizer->line = 1;
    tokenizer->name = file;
    tokenizer->col = 0;
    tokenizer->cur = 0;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    tokenizer->code = malloc(fsize + 1);
    size_t size = fread(tokenizer->code, 1, fsize, f);
    tokenizer->code[size] = '\0';
    fclose(f);
    
    tokenizer->tokenlen = 0;
    tokenizer->tokencap = 900;
    tokenizer->tokens = malloc(sizeof(Token) * tokenizer->tokencap);
    return tokenizer;
}

char tokenizer_token(Tokenizer *tokenizer) {
    char c = tokenizer->code[tokenizer->cur];
    int prevCol = tokenizer->col;
    int prevRow = tokenizer->line;

    if (c == '\0') {
        tokenizer_ensure_capacity(tokenizer);
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_EOF, "\0", tokenizer->line, tokenizer->col, tokenizer->name};
        return -1;
    }

    tokenizer_ensure_capacity(tokenizer);

    if (c == '(') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LP, "(", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == ')') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_RP, ")", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '|') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_PIPE, "|", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == ':') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_COLON, ":", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '{') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LB, "{", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == ',') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_COMMA, ",", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '}') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_RB, "}", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '=') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_EQ, "=", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == ';') {
    } else if (c == '#') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_HASH, "#", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '$') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_DOLLAR, "$", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '<') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_GT, "<", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '>') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LT, ">", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '+') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_PLUS, "+", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '[') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_LBRACKET, "[", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == ']') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_RBRACKET, "]", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '*') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_MUL, "*", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '/') {
        if (tokenizer->code[tokenizer->cur + 1] == '/') {
            tokenizer->cur += 2;
            while (tokenizer->code[tokenizer->cur] != '\n' && tokenizer->code[tokenizer->cur] != '\0') {
                tokenizer->cur++;
            }
            tokenizer->line++;
            tokenizer->cur++;
            tokenizer->col = 0;
            return tokenizer_token(tokenizer);
        } else if (tokenizer->code[tokenizer->cur + 1] == '*') {
            tokenizer->cur += 2;
            char found = 0;
            while (tokenizer->code[tokenizer->cur] != '\0') {
                if (tokenizer->code[tokenizer->cur] == '*' && tokenizer->code[tokenizer->cur+1] == '/') {
                    tokenizer->cur += 2;
                    found = 1;
                    break;
                }
                if (tokenizer->code[tokenizer->cur] == '\n') tokenizer->line++;
                tokenizer->cur++;
            }
            return tokenizer_token(tokenizer);
        }
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_DIV, "/", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '%') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_MODULO, "%", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '.') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_DOT, ".", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '&') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_AMP, "&", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '-') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_SUB, "-", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '!') {
        tokenizer->tokens[tokenizer->tokenlen++] = (Token){TOKEN_EXC, "!", tokenizer->line, tokenizer->col, tokenizer->name};
    } else if (c == '\'') {
        tokenizer->cur++;
        char buf[3];
        if (tokenizer->code[tokenizer->cur] == '\\') {
            buf[0] = '\\';
            buf[1] = tokenizer->code[++tokenizer->cur];
            buf[2] = '\0';
        } else {
            buf[0] = tokenizer->code[tokenizer->cur];
            buf[1] = '\0';
        }
        tokenizer->cur++;
        if (tokenizer->code[tokenizer->cur] != '\'') {
            error_generate_tokenizer(tokenizer, "AbruptEndError", "Character is too long", tokenizer->line, prevCol, tokenizer->name);
        }
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_CHAR;
        strcpy(tokenizer->tokens[tokenizer->tokenlen].value, buf);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;
        tokenizer->tokenlen++;
    } else if (c == '\"') {
        tokenizer->cur++;
        char string[1000]; int stringlen = 0;
        while (tokenizer->code[tokenizer->cur] != '\"' && tokenizer->code[tokenizer->cur] != '\0') {
            if (tokenizer->code[tokenizer->cur] == '\n') {
                error_generate_tokenizer(tokenizer, "SyntaxError", "Newlines not allowed inside strings", tokenizer->line, prevCol, tokenizer->name);
            }
            if (tokenizer->code[tokenizer->cur] == '\\') {
                tokenizer->cur++;
                if (tokenizer->code[tokenizer->cur] == 'n') string[stringlen++] = '\n';
                else if (tokenizer->code[tokenizer->cur] == '\"') string[stringlen++] = '\"';
            } else {
                string[stringlen++] = tokenizer->code[tokenizer->cur];
            }
            tokenizer->cur++;
        }
        string[stringlen] = '\0';
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_STRING;
        strcpy(tokenizer->tokens[tokenizer->tokenlen].value, string);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;
        
        if (tokenizer->tokenlen >= 2) {
            if (tokenizer->tokens[tokenizer->tokenlen-2].type == TOKEN_HASH && 
                strcmp(tokenizer->tokens[tokenizer->tokenlen-1].value, "include") == 0) {
                tokenizer->tokenlen -= 2;
                Tokenizer *tokenizer2 = tokenizer_init(string);
                while(tokenizer_token(tokenizer2) != -1);
                for (int n = 0; n < tokenizer2->tokenlen; n++) {
                    if (tokenizer2->tokens[n].type == TOKEN_EOF) continue;
                    tokenizer_ensure_capacity(tokenizer);
                    tokenizer->tokens[tokenizer->tokenlen++] = tokenizer2->tokens[n];
                }
                free(tokenizer2->code); free(tokenizer2->tokens); free(tokenizer2);
                tokenizer->cur++;
                return 0;
            }
        }
        tokenizer->tokenlen++;
    } else if (isalpha(c) || c == '_') {
        char res[200]; int reslen = 0;
        while (isalnum(tokenizer->code[tokenizer->cur]) || tokenizer->code[tokenizer->cur] == '_') {
            res[reslen++] = tokenizer->code[tokenizer->cur++];
        }
        res[reslen] = '\0';
        tokenizer->cur--;
        tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_ID;
        strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;
        if (strcmp(res, "true") == 0 || strcmp(res, "false") == 0) {
            tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_INT;
        }
        tokenizer->tokenlen++;
    } else if (isdigit(c)) {
        char res[200]; int reslen = 0;
        while (isdigit(tokenizer->code[tokenizer->cur])) {
            res[reslen++] = tokenizer->code[tokenizer->cur++];
        }
        res[reslen] = '\0';
        if (tokenizer->code[tokenizer->cur] == 'f') {
            tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_FLOAT;
        } else {
            tokenizer->cur--;
            tokenizer->tokens[tokenizer->tokenlen].type = TOKEN_INT;
        }
        strcpy(tokenizer->tokens[tokenizer->tokenlen].value, res);
        tokenizer->tokens[tokenizer->tokenlen].name = tokenizer->name;
        tokenizer->tokenlen++;
    } else if (c == '\n') {
        tokenizer->line++;
        tokenizer->col = 0;
        tokenizer->cur++;
        return 0;
    } else if (isspace(c)) {
        tokenizer->cur++;
        tokenizer->col++;
        return 0;
    } else {
        error_generate_tokenizer(tokenizer, "SyntaxError", "Unknown token", tokenizer->line, prevCol, tokenizer->name);
    }

    tokenizer->tokens[tokenizer->tokenlen - 1].col = prevCol;
    tokenizer->tokens[tokenizer->tokenlen - 1].row = prevRow;
    tokenizer->cur++;
    tokenizer->col++;
    return 0;
}
