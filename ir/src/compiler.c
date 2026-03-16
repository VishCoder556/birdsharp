AST *load_ast_compiler(Compiler *compiler){
    if (compiler->cur == 0){
        return compiler->asts;
    }else {
        AST *ast1 = compiler->asts;
        for (int i=0; i<compiler->cur; i++){
            ast1 = ast1->next;
        };
        return ast1;
    };
}

char* find_available_tmp_file(char *ext) {
    static char filename[256];
    int i = 0;

    while (true) {
        snprintf(filename, sizeof(filename), "tmp_%d.%s", i, ext);

        if (access(filename, F_OK) != 0) {
            return strdup(filename);
        }

        i++;
        
        if (i > 1000) return NULL; 
    }
}
void compiler_peek(Compiler *compiler){
    compiler->cur++;
}

void compiler_write_text(Compiler *compiler, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    fprintf(compiler->f, "%s", add);
};

void compiler_write_text_line(Compiler *compiler, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    fprintf(compiler->f, "\t%s\n", add);
};

void compiler_write(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    if (strlen(buf) + n >= 4096) {
        n = 4096 - strlen(buf) - 1;
        if (n <= 0) return;
        add[n] = '\0';
    }

    strcat(buf, add);
}

void compiler_write_line(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);

    if (n < 0) return;

    if (strlen(buf) + n >= 4096) {
        n = 4096 - strlen(buf) - 1;
        if (n <= 0) return;
        add[n] = '\0';
    }

    strcat(buf, "\t");
    strcat(buf, add);
    strcat(buf, "\n");
}

