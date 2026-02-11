typedef struct {
    char *name;
    char *_o;
    char *_asm;
    AST *asts;
    int astlen;
    int cur;

    FILE *f;
    char *data;
    int templabel;
    Reviser_Global *global;
}Compiler;
