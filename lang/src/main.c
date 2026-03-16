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
 *          Run "bs main.bsh" to create the .s file
 *              (This will automatically assemble it into a .o file and link it into a .out)
 *          Run "./main" to run your file!
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
#include <time.h>
#include <unistd.h>
#include <fcntl.h>


// From https://github.com/tsoding/arena/

#define ARENA_IMPLEMENTATION
#include "include/arena.h"
typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;
Arena arena = {0};

char *HELP = 
"Help for AprilScript\n\n"
"-o: Specify the output file\n"
"-h: Print this menu\n"
"-dynamic: Specify dynamic linking (linking with the stdlibrary + a lot more, but at the cost of the program being slow)\n"
"-static: Specify static linking (done by default, no linking with stdlib and other libraries, but the program is fast)\n"
"-...: Anything starting with '-' will be treated as a flag that will be sent to the assembler and linker\n";

enum BirdSharpTypes {
   TYPE_STATIC,
   TYPE_DYNAMIC
};



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




char* find_ir() {
    static char filename[256];
    int i = 0;

    while (true) {
        snprintf(filename, sizeof(filename), "tmp_%d.ir", i);

        if (access(filename, F_OK) != 0) {
            return strdup(filename);
        }

        i++;
        
        if (i > 1000) return NULL; 
    }
}


#include "tokenizer.c"
#include "parser.c"
#include "typechecker.c"
#include "generator.c"
#include "ir.c"
#include "preprocessor.c"


#define clockstart() clock_gettime(CLOCK_MONOTONIC, &__start);

#define clockend(name) \
    clock_gettime(CLOCK_MONOTONIC, &__end); \
    fprintf(stdout, "[INFO] "name" process took %.4f milliseconds\n", (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6); \

void optimize_top_level(Parser *parser){
    if (_main == NULL){
        _main = arena_alloc(&arena, sizeof(AST));
        if (top_level_len >= 1 && top_level != NULL){
            _main->row = top_level[0]->row;
            _main->col = top_level[0]->col;
            _main->filename = top_level[0]->filename;
        }
        _main->type = AST_FUNCDEF;
        _main->data.funcdef.name = "main";
        _main->data.funcdef.block = top_level;
        _main->data.funcdef.blocklen = top_level_len;
        _main->next = NULL;

        if (parser->ast_tail) {
            parser->ast_tail->next = _main;
            parser->ast_tail = _main; 
            parser->astlen++;
        }
    } else {
        int old_len = _main->data.funcdef.blocklen;
        int new_len = old_len + top_level_len;

        AST **new_block = arena_realloc(&arena, _main->data.funcdef.block, sizeof(AST*) * old_len, sizeof(AST*) * new_len);
        
        for (int i = 0; i < top_level_len; i++) {
            new_block[old_len + i] = top_level[i];
        }
        _main->data.funcdef.block = new_block;
        _main->data.funcdef.blocklen = new_len;
    }
}


int main(int argc, char **argv){
    struct timespec __begin;
    clock_gettime(CLOCK_MONOTONIC, &__begin);
    argv++;
    char *input_file = "";
    char *output_file = "";
    while (*argv){
        if (*argv[0] == '-'){
            if(strcmp(*argv, "-o") == 0){
                argv++;
                if (*argv){
                output_file = *argv;
                }else {
                    break;
                }
            }else if(strcmp(*argv, "-h") == 0){
                printf("%s", HELP);
            }
        } else {
            input_file = *argv;
        };
        argv++;
    };

    if (strcmp(output_file, "") == 0){
        output_file = "main";
    };

    if (!input_file || strcmp(input_file, "") == 0){
        printf("\x1b[1;31merror\x1b[0m: No input file provided");
        return -1;
    };

    struct timespec __start, __end;
    arena_reset(&arena);


    clockstart();


    Tokenizer *tokenizer = tokenizer_init(input_file);
    while(tokenizer_token(tokenizer) != -1){};


    clockend("Tokenizer");
    clockstart();
    

    preprocess(input_file, tokenizer);

    clockend("Preprocessor");
    clockstart();

    Parser *parser = parser_init(tokenizer);

    while (parser_eat(parser) != -1){};

    optimize_top_level(parser);


    clockend("Parsing");
    clockstart();

    Typechecker *typechecker = typechecker_init(parser);
    while (typechecker_eat_ast(typechecker) != -1){};

    clockend("Typechecking");
    clockstart();

    char *_ir = find_ir();
    clock_gettime(CLOCK_MONOTONIC, &__start);
    Generator *generator = generator_init(typechecker, _ir, ir);
    while (generator_eat_ast(generator) != -1){};
    
    clockend("Transpiling");
    clockstart();

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "irc %s -target arm64 -o %s", _ir, output_file);
    system(cmd);
    remove(_ir);

    clockend("Compiling");

    fprintf(stdout, "[INFO] Program took %.4f milliseconds\n", (__end.tv_sec - __begin.tv_sec) * 1000.0 + (__end.tv_nsec - __begin.tv_nsec) / 1e6);
    
    arena_free(&arena);
    
    return 0;
}
