typedef enum {
    LOC_REG,
    LOC_STACK,
    LOC_CONST,
    LOC_UNKNOWN
} LocType;

typedef struct {
    LocType type;
    int offset;
    int size;
} Location;


typedef struct {
    void *stack;
    void *regs;
    int sp;
    int cur;
    int astlen;
    AST *asts;
    char *name;
} Simulator;
