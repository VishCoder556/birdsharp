typedef enum{
    AST_UNKNOWN,
    AST_STRING,
    AST_INT,
    AST_FUNCALL,
    AST_FUNCDEF,
    AST_SYSCALL,
    AST_MOV,
    AST_VAR,
    AST_REG,
    AST_RET,
    AST_PUSH,
    AST_POP,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_METADATA,
    AST_LT,
    AST_LTE,
    AST_GT,
    AST_GTE,
    AST_JMP,
    AST_JMPIF,
    AST_JMPIFN,
    AST_LABEL,
    AST_CALL,
    AST_CALLIF,
    AST_CALLIFN,
    AST_ARRAY,
    AST_CHAR,
    AST_EQ,
    AST_NEQ,
    AST_REF,
    AST_DEREF,
    AST_PLUS,
    AST_LOCAL,
    AST_CONST
}AST_Type;

typedef struct AST AST;
typedef int AST_TypeInfo;
typedef struct {
    char *name;
    AST *value;
    AST *definition;
    int offset;
    AST_Reg reg;
}AST_Var;
typedef struct {
    AST *block;
    int blocklen;
}AST_AstArray;
typedef struct {
    char *name;
    AST *block;
    int blocklen;
} AST_FuncDef;
typedef struct {
    AST *left;
    AST *right;
} AST_Expr;
typedef struct{
    AST_Reg reg;
    AST *right;
}AST_Operation;
typedef struct{
    AST_Reg reg;
    AST *left;
    AST *right;
}AST_OpExpr;
typedef struct{
    AST *reg;
    char *label;
    int line;
}AST_JmpIf;
typedef struct {
    char *value;
}AST_Text;

typedef union {
    AST_FuncDef funcdef;
    AST_AstArray astarray;
    AST_Operation operation;
    AST_OpExpr opexpr;
    AST_Text text;
    AST_Expr expr;
    AST_Var var;
    AST_JmpIf jmpif;
} AST_Data;

struct AST {
    AST_Type type;
    AST_Data data;
    AST_TypeInfo typeinfo;
    int row;
    int col;
    char *file;

    AST *next;
};

typedef struct Parser {
    char *code;
    AST *asts;
    int astlen;
    AST prevAst;
    char *name;
    int cur;
    Token *tokens;
    int tokenlen;
}Parser;
