typedef enum{
    AST_UNKNOWN,
    AST_STRING,
    AST_CHAR,
    AST_FUNCALL,
    AST_FUNCDEF,
    AST_FLOAT,
    AST_INT,
    AST_RET,
    AST_ASSIGN,
    AST_VAR,
    AST_DOLLAR,
    AST_GT,
    AST_LT,
    AST_GTE,
    AST_LTE,
    AST_PLUS,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MODULO,
    AST_EQ,
    AST_NEQ,
    AST_IF,
    AST_WHILE,
    AST_DEREF,
    AST_REF,
    AST_MODE,
    AST_EXPR,
    AST_CAST,
    AST_SYSCALL,
    AST_BREAK,
    AST_MODE_IF,
    AST_TYPEDEF,
    AST_NOT,
    AST_AND,
    AST_OR,
    AST_INDEX,
    AST_STRUCT
}AST_Type;

typedef struct AST AST;
typedef struct AST_T AST_T;

typedef struct Arg {
    char *value;
    int num;
}AST_Arg;

typedef struct ASTs {
    struct AST *ast;
    struct ASTs *next;
}ASTs;

typedef struct {
    char *name;
    AST *args;
    int argslen;
} AST_FuncCall;
typedef struct {
    AST **statements;
    int statementlen;
    AST *condition;
}Block;
typedef struct {
    Block block;
    AST **else1;
    int elselen;
    Block *elseif;
    int elseiflen;
} AST_If;
typedef struct {
    AST *condition;
    AST **block;
    int blocklen;
} AST_While;

typedef struct {
    AST *from;
    AST *assignto;
    bool new;
} AST_Assign;

typedef enum {
    KIND_VOID,
    KIND_CHAR,
    KIND_I8,
    KIND_I16,
    KIND_I32,
    KIND_I64,
    KIND_INT,
    KIND_FLOAT,
    KIND_DOUBLE,
    KIND_STRUCT,
    KIND_CONST,
    KIND_BOOL,
    KIND_UNKNOWN,
    KIND_ARRAY,
    KIND_AUTO
} TypeKind;

typedef struct AST_TypeInfo {
    char *type;
    TypeKind kind;
    char ptrnum;
    
    union {
        struct {
            int size;
            struct AST_TypeInfo *elem_type;
        } array;
    } data;
} AST_TypeInfo;

typedef struct {
    AST *ret;
} AST_Return;

typedef struct Argument Argument;

typedef struct {
    char *name;
    struct Argument *args;
    int argslen;
    AST *block;
    int blocklen;
} AST_FuncDef;

typedef struct {
    AST *left;
    AST *right;
} AST_Expr;

typedef struct {
    char *name;
    char *res;
}AST_Mode;

typedef struct {
    AST_TypeInfo type;
    char *name;
}AST_StructMember;

typedef struct {
    char* name;
    AST_StructMember *members;
    int memberlen;
} AST_Struct;

typedef union {
    AST_FuncDef funcdef;
    AST_FuncCall funcall;
    AST_Assign assign;
    AST_Return ret;
    AST_Arg arg;
    AST_Expr expr;
    AST_If if1;
    AST_While while1;
    AST_Mode mode;
    AST_Struct struct1;
} AST_Data;

struct AST {
    AST_Type type;
    AST_Data data;
    AST_TypeInfo typeinfo;
    int row;
    int col;
    char *filename;
    struct AST *next;
};
typedef struct Parser {
    AST *asts;
    AST *ast_tail;
    int astlen;
    AST prevAst;
    char *name;
    int cur;
    Token *tokens;
    int tokenlen;
}Parser;



typedef struct Argument {
    char *arg;
    AST_TypeInfo type;
}Argument;

struct Pair {
    TypeKind kind;
    char *name;
    int length;
    char *longname;
};

struct Pair types[] = {
    (struct Pair){KIND_I32, "int", 4, "integer"},
    (struct Pair){KIND_CHAR, "char", 1, "character"},
    (struct Pair){KIND_I64, "long", 8, "long integer"},
    (struct Pair){KIND_FLOAT, "float", 4, "float"},
    (struct Pair){KIND_I8, "i8", 1, "8-bit integer"},
    (struct Pair){KIND_I16, "i16", 2, "16-bit integer"},
    (struct Pair){KIND_I32, "i32", 4, "32-bit integer"},
    (struct Pair){KIND_I64, "i64", 8, "64-bit integer"},
    (struct Pair){KIND_BOOL, "bool", 1, "boolean (only 0 or 1)"},
    (struct Pair){KIND_CONST, "const", 8, "constant (placeholder type for integer constants)"},
    (struct Pair){KIND_ARRAY, "array", -1, "array of values"},
    (struct Pair){KIND_AUTO, "var", 8, "automatic value"},
};
int typesLen = sizeof(types) / sizeof(types[0]);
