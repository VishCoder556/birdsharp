/*
 * Assembly Compiler
 *
*/

#define NO_SSIZE_T
#define INCLUDE_UNISTD
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../../../libs/inout.c"
#include "../../../libs/string.c"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "include/tokenizer.h"

typedef struct {
    int reg;
    short size;
}AST_Reg;

#include "include/parser.h"

typedef enum {
    TARGET_MACOS,
    TARGET_LINUX,
    TARGET_WINDOWS,
    TARGET_WASM,
    TARGET_UNKNOWN
}TargetFormat;

#ifdef _WIN32
    TargetFormat target = TARGET_WINDOWS;
    target = TARGET_WINDOWS;
#elif __APPLE__
    TargetFormat target = TARGET_MACOS;
#elif __linux__
    TargetFormat target = TARGET_LINUX;
#else
    TargetFormat target = TARGET_UNKNOWN;
#endif

#include "include/reviser.h"
#include "include/compiler.h"
#include "include/wasm.h"
#include "include/simulator.h"

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

void error_generate_tokenizer(Tokenizer *tokenizer, char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;31m%s\x1b[0m: %s\n", f, l, c, type, name);
    print_error_line(tokenizer->code, l, c);
    exit(-1);
};
void error_generate_parser(Parser *parser, char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;31m%s\x1b[0m: %s\n", f, l, c, type, name);
    print_error_line(parser->code, l, c);
    exit(-1);
};
void error_generate_reviser(Reviser *reviser, char *type, char *name, AST *ast){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;31m%s\x1b[0m: %s\n", reviser->name, ast->row, ast->col, type, name);
    print_error_line(reviser->code, ast->row, ast->col);
    exit(-1);
};

void warning_generate(char *type, char *name){
    printf("\x1b[1;35m%s\x1b[0m: %s\n", type, name);
};
void warning_generate_parser(char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;35m%s\x1b[0m: %s\n", f, l, c, type, name);
};

#include "tokenizer.c"
#include "parser.c"
#include "reviser.c"
#include "compiler.c"
#include "simulator.c"
#include "wasm.c"


typedef enum {
    MODE_ASM,
    MODE_WASM,
    MODE_SIMULATE
}Mode;

int main(int argc, char **argv){
    argv++;
    char *input_file = "";
    char *output_file = "";
    Mode mode = MODE_ASM;
    while (*argv){
        if (string_compare(*argv, "-o", strlen(*argv)) == 0){
            argv++;
            output_file = *argv;
        }
        if (string_compare(*argv, "--simulate", strlen(*argv)) == 0){
            mode = MODE_SIMULATE;
        }
        if (string_compare(*argv, "-target", strlen(*argv)) == 0){
            argv++;
            if (string_compare(*argv, "macos-x64", string_len(*argv)) == 0){
                target = TARGET_MACOS;
            }else if (string_compare(*argv, "linux-x64", string_len(*argv)) == 0){
                target = TARGET_LINUX;
            }else if (string_compare(*argv, "windows-x64", string_len(*argv)) == 0){
                target = TARGET_WINDOWS;
            }else if (string_compare(*argv, "wasm", string_len(*argv)) == 0){
                mode = MODE_WASM;
            }else {
                target = TARGET_UNKNOWN;
            };
        }
        if (string_compare(input_file, "", string_len(input_file)) == 0){
            input_file = *argv;
        }
        argv++;
    }
    if (string_compare(output_file, "", string_len(output_file)) == 0){
        output_file = "res/main";
    }

    Tokenizer *tokenizer = tokenizer_init(input_file);

    while(tokenizer_token(tokenizer) != -1){
    };

    Parser *parser = parser_init(tokenizer);
    while (parser_eat(parser) != -1){
    };

    Reviser *reviser = reviser_init(parser);
    while (reviser_spy(reviser) != -1){
    };
    current_ast = NULL;
    reviser->cur = 0;
    reviser->global->functionlen = 0;
    while (reviser_eat(reviser) != -1){
    };


    if (mode == MODE_ASM){
        Compiler *compiler = compiler_init(reviser, output_file);
        while (compiler_eat(compiler) != -1){
        };
        compiler_close(compiler);
    }else if(mode == MODE_WASM){
        Wasm *wasm = wasm_init(reviser, output_file);
        while (wasm_eat(wasm) != -1){
        };
        wasm_close(wasm);
    }else if(mode == MODE_SIMULATE){
        Simulator *simulator = simulator_init(reviser, output_file);
        while (simulator_eat(simulator) != -1){
        };
        for (int i=0; i<14; i++){
            printf("Reg %d: %d\n", i, (((int*)simulator->regs)[i]));
        };
    }

    return 0;
};
