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

                FILE *f = fopen(input_file, "r");
                if (f == NULL) {
                    char *err_msg = arena_sprintf(&arena, "File '%s' does not exist", input_file);
                    error_generate_parser("IncludeError", err_msg, tokenizer->tokens[i].row, tokenizer->tokens[i].col, tokenizer->tokens[i].name);
                    continue;
                }

                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);

                Tokenizer *tokenizer2 = arena_alloc(&arena, sizeof(Tokenizer));
                tokenizer2->line = 1;
                tokenizer2->name = arena_strdup(&arena, input_file);
                tokenizer2->col = 0;
                tokenizer2->code = arena_alloc(&arena, fsize + 1);
                size_t size = fread(tokenizer2->code, 1, fsize, f);
                tokenizer2->code[size] = '\0';
                fclose(f);

                source_files[num_source_files++] = (SourceFile){tokenizer2->name, tokenizer2->code, size};
                
                tokenizer2->cur = 0;
                tokenizer2->tokens = arena_alloc(&arena, sizeof(Token) * 9000);
                tokenizer2->tokenlen = 0;

                while(tokenizer_token(tokenizer2) != -1);

                if (tokenizer2->tokenlen > 0) {
                    tokenizer2->tokenlen--;
                }

                preprocess(input_file, tokenizer2);

                int tokens_to_remove = 4;
                int new_tokens_count = tokenizer2->tokenlen;
                int current_tail_size = tokenizer->tokenlen - (i + tokens_to_remove);

                memmove(
                    tokenizer->tokens + i + new_tokens_count,
                    tokenizer->tokens + i + tokens_to_remove,
                    current_tail_size * sizeof(Token)
                );

                for (int v = 0; v < new_tokens_count; v++) {
                    tokenizer->tokens[i + v] = tokenizer2->tokens[v];
                }

                tokenizer->tokenlen += (new_tokens_count - tokens_to_remove);
                i += (new_tokens_count - 1);
            }
        }
    }
}
