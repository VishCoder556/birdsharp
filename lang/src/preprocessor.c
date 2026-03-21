#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    Token in[10];
    int inlen;
    Token out[10];
    int outlen;
} Preprocessor_Define;

typedef struct {
    Preprocessor_Define defines[10];
    int definelen;
} Preprocessor_SymbolTable;

Preprocessor_SymbolTable preprocess_symtab;

char token_eq(Token token1, Token token2) {
    if (token1.type == token2.type) {
        return strcmp(token1.value, token2.value) == 0;
    }
    return 0;
}

void preprocess_substitute(Tokenizer *tokenizer, int *i) {
    for (int d = 0; d < preprocess_symtab.definelen; d++) {
        Preprocessor_Define *def = &preprocess_symtab.defines[d];
        if (def->inlen == 0) continue;
        if (*i + def->inlen > tokenizer->tokenlen) continue;

        char match = 1;
        for (int k = 0; k < def->inlen; k++) {
            if (!token_eq(tokenizer->tokens[*i + k], def->in[k])) {
                match = 0;
                break;
            }
        }

        if (match) {
            tokenizer_insert_at(tokenizer, *i, def->out, def->outlen, def->inlen);
            if (def->outlen > 0) {
                *i += (def->outlen - 1);
            } else {
                *i -= 1; 
            }
            return;
        }
    }
}

void preprocess_define(Tokenizer *tokenizer, int *i) {
    if (preprocess_symtab.definelen >= 10) {
        tokenizer_remove_at(tokenizer, *i, 1);
        return;
    }

    tokenizer_remove_at(tokenizer, *i, 3);

    Preprocessor_Define *def = &preprocess_symtab.defines[preprocess_symtab.definelen];
    def->inlen = 0;
    def->outlen = 0;

    while (*i < tokenizer->tokenlen && tokenizer->tokens[*i].type != TOKEN_EQ) {
        if (def->inlen < 10) {
            def->in[def->inlen++] = tokenizer->tokens[*i];
        }
        tokenizer_remove_at(tokenizer, *i, 1);
    }

    if (*i < tokenizer->tokenlen && tokenizer->tokens[*i].type == TOKEN_EQ) {
        tokenizer_remove_at(tokenizer, *i, 1);
    }

    while (*i < tokenizer->tokenlen) {
        if (tokenizer->tokens[*i].type == TOKEN_SEMICOLON) {
            tokenizer_remove_at(tokenizer, *i, 1);
            break;
        }

        int start_val = *i;
        preprocess_substitute(tokenizer, i);

        if (start_val == *i) {
            if (def->outlen < 10) {
                def->out[def->outlen++] = tokenizer->tokens[*i];
            }
            tokenizer_remove_at(tokenizer, *i, 1);
        }
    }

    preprocess_symtab.definelen++;
    *i -= 1;
}

void preprocess(char *main_file, Tokenizer *tokenizer);

char preprocess_include(Tokenizer *tokenizer, char *main_file, int i) {
    char path[512];
    strncpy(path, main_file, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    char *dir = dirname(path);
    char input_file[512];
    snprintf(input_file, sizeof(input_file), "%s/%s", dir, tokenizer->tokens[i + 3].value);

    Tokenizer *tokenizer2 = tokenizer_init(input_file);
    if (tokenizer2 == NULL) return 0;

    while (tokenizer_token(tokenizer2) != -1);
    // tokenizer_free_code(tokenizer2);

    if (tokenizer2->tokenlen > 0 && tokenizer2->tokens[tokenizer2->tokenlen - 1].type == TOKEN_EOF) {
        tokenizer2->tokenlen--;
    }

    preprocess(input_file, tokenizer2);

    if (tokenizer2->tokenlen > 0) {
        tokenizer_insert_at(tokenizer, i, tokenizer2->tokens, tokenizer2->tokenlen, 4);
    } else {
        tokenizer_remove_at(tokenizer, i, 4);
    }
    // tokenizer_free_tokens(tokenizer2);
    
    return 1;
}

void preprocess(char *main_file, Tokenizer *tokenizer) {
    if (!tokenizer || !tokenizer->tokens) return;

    int i = 0;
    while (i < tokenizer->tokenlen) {
        if (i + 2 < tokenizer->tokenlen &&
            tokenizer->tokens[i].type == TOKEN_HASH &&
            tokenizer->tokens[i + 1].type == TOKEN_EXC &&
            tokenizer->tokens[i + 2].type == TOKEN_ID) {

            if (strcmp(tokenizer->tokens[i + 2].value, "include") == 0) {
                if (i + 3 < tokenizer->tokenlen && tokenizer->tokens[i + 3].type == TOKEN_STRING) {
                    preprocess_include(tokenizer, main_file, i);
                } else {
                    tokenizer_remove_at(tokenizer, i, 3);
                }
                continue;
            } else if (strcmp(tokenizer->tokens[i + 2].value, "define") == 0) {
                preprocess_define(tokenizer, &i);
                continue;
            }
        }

        int start_i = i;
        preprocess_substitute(tokenizer, &i);
        
        if (start_i == i) {
            i++;
        } else if (i < 0) {
            i = 0;
        }
    }
}
