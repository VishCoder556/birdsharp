// Incomplete WASM support


typedef struct Wasm {
    char *name;
    FILE *f;
    AST *asts;
    int astlen;
    int cur;
    void *global;
    char *_wat;
    char *seen_locals[256];
    int seen_count;
} Wasm;
