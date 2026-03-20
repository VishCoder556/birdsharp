#include <libgen.h>

void preprocess(char *main_file, Tokenizer *tokenizer) {
    for (int i = 0; i < tokenizer->tokenlen; i++) {
        if (tokenizer->tokens[i].type == TOKEN_HASH && i + 3 < tokenizer->tokenlen) {
            if (tokenizer->tokens[i + 1].type == TOKEN_EXC && 
                tokenizer->tokens[i + 2].type == TOKEN_ID && 
                strcmp(tokenizer->tokens[i + 2].value, "include") == 0 && 
                tokenizer->tokens[i + 3].type == TOKEN_STRING) {
                
                char path[512];
                strncpy(path, main_file, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
                
                char *dir = dirname(path);
                char input_file[512];
                snprintf(input_file, sizeof(input_file), "%s/%s", dir, tokenizer->tokens[i + 3].value);

                Tokenizer *tokenizer2 = tokenizer_init(input_file);
                if (tokenizer2 == NULL) {
                    char *err_msg = arena_sprintf(&arena, "File '%s' does not exist", input_file);
                    error_generate_parser("IncludeError", err_msg, tokenizer->tokens[i].row, tokenizer->tokens[i].col, tokenizer->tokens[i].name);
                    continue;
                }

                while(tokenizer_token(tokenizer2) != -1);

                if (tokenizer2->tokenlen > 0 && tokenizer2->tokens[tokenizer2->tokenlen - 1].type == TOKEN_EOF) {
                    tokenizer2->tokenlen--;
                }

                preprocess(input_file, tokenizer2);

                tokenizer_insert_at(tokenizer, i, tokenizer2->tokens, tokenizer2->tokenlen, 4);

                i += (tokenizer2->tokenlen - 1);
            }
        }
    }
}
