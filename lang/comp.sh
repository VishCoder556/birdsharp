gcc  src/main.c -o ../bs -Wunused-function
# nasm -f macho64 res/main.asm -o res/main.o
# ld -macosx_version_min 10.13 -o res/main.out res/main.o -static
#


#include "tokenizer.c"
#include "parser.c"
#include "typechecker.c"
#include "generator.c"
#include "ir.c"
#include "preprocessor.c"
