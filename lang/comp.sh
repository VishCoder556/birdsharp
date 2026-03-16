gcc -g src/main.c -o ../bs -Wunused-function
# nasm -f macho64 res/main.asm -o res/main.o
# ld -macosx_version_min 10.13 -o res/main.out res/main.o -static
