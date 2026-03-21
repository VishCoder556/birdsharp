typedef enum BirdSharpTypes BirdSharpTypes;
BirdSharpTypes _BirdSharpTypes;

enum TokenType {
    TOKEN_EOF,
    TOKEN_STRING,
    TOKEN_LP,
    TOKEN_RP,
    TOKEN_LB,
    TOKEN_RB,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_HASH,
    TOKEN_EXC,
    TOKEN_COMMA,
    TOKEN_INT,
    TOKEN_EQ,
    TOKEN_DOLLAR,
    TOKEN_CHAR,
    TOKEN_ID,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_PLUS,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MODULO,
    TOKEN_AMP,
    TOKEN_DOT,
    TOKEN_FLOAT,
    TOKEN_PIPE,
    TOKEN_SEMICOLON
};
typedef struct Token{
    int type;
    char value[90];
    int row;
    int col;
    char *name;
}Token;
typedef struct Tokenizer {
    char *code;
    char *name;
    int cur;
    int line;
    int col;
    Token *tokens;
    int tokenlen;
    int tokencap;
}Tokenizer;


void tokenizer_free_tokens(Tokenizer *tokenizer);
char *token_to_string(int tokentype){
    switch (tokentype){
        case TOKEN_EOF: return "end of file";
        case TOKEN_STRING: return "string";
        case TOKEN_CHAR: return "character";
        case TOKEN_ID: return "identifier";
        case TOKEN_LP: return "'('";
        case TOKEN_RP: return "')'";
        case TOKEN_SEMICOLON: return "';'";
        case TOKEN_LB: return "'{'";
        case TOKEN_RB: return "'}'";
        case TOKEN_LBRACKET: return "'['";
        case TOKEN_RBRACKET: return "']'";
        case TOKEN_HASH: return "'#'";
        case TOKEN_EXC: return "'!'";
        case TOKEN_COMMA: return "','";
        case TOKEN_INT: return "integer";
        case TOKEN_EQ: return "'='";
        case TOKEN_DOLLAR: return "'$'";
        case TOKEN_GT: return "'<'";
        case TOKEN_LT: return "'>'";
        case TOKEN_PLUS: return "'+'";
        case TOKEN_SUB: return "'-'";
        case TOKEN_MUL: return "'*'";
    };
    return "unknown";
};


typedef struct {
    char *file_path;
    char *code;
    size_t size;
} SourceFile;
#define MAX_FILES 16
SourceFile source_files[MAX_FILES];
int num_source_files = 0;
