typedef struct Reviser_Variable Reviser_Variable;
struct Reviser_Variable{
    char *name;
    AST *definition;
    int size;
    struct Reviser_Variable *next;
};

typedef struct {
    struct Reviser_Variable *variables;
    int variablelen;
}Reviser_Scope;

typedef struct Reviser_Function Reviser_Function;
struct Reviser_Function{
    Reviser_Scope scope;
    char *name;
    struct Reviser_Function *next;
};

typedef struct {
    Reviser_Scope scope;
    Reviser_Function *functions;
    int functionlen;
}Reviser_Global;

#define REVISER_REGISTERS 14

typedef struct {
    AST_Reg reg;
    int credits; // Virtual Registers are given a credit score as a priority index
    int firstmention; // As a line
    int lastmention; // As a line
}Reviser_VirtualReg;



typedef struct {
    char *name;
    char *code;
    AST *asts;
    int astlen;
    int cur;

    Reviser_Global *global;
    Reviser_VirtualReg *regs;
}Reviser;
