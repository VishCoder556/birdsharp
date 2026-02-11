// Incomplete WASM support


void wasm_write_text(Wasm *wasm, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char add[256];
    int n = vsnprintf(add, sizeof(add), fmt, args);
    va_end(args);
    if (n < 0) return;
    fprintf(wasm->f, "%s", add);
}

char *len_to_type(int len) {
    switch (len) {
        case 8: return "i64";
        default: return "i32";
    }
}

struct AST *load_ast_wasm(Wasm *wasm) {
    if (wasm->cur == 0) return wasm->asts;
    struct AST *ast1 = wasm->asts;
    for (int i = 0; i < wasm->cur; i++) {
        ast1 = ast1->next;
    }
    return ast1;
}

void wasm_peek(Wasm *wasm) {
    wasm->cur++;
}

bool wasm_is_local_seen(Wasm *wasm, const char *name) {
    for (int i = 0; i < wasm->seen_count; i++) {
        if (strcmp(wasm->seen_locals[i], name) == 0) return true;
    }
    return false;
}

void wasm_push_local(Wasm *wasm, const char *name) {
    if (wasm->seen_count < 256) {
        wasm->seen_locals[wasm->seen_count++] = strdup(name);
    }
}

void wasm_scan_locals(Wasm *wasm, struct AST *ast) {
    if (!ast) return;
    if (ast->type == AST_LOCAL) {
        if (!wasm_is_local_seen(wasm, ast->data.text.value)) {
            wasm_write_text(wasm, "\t(local $%s %s)\n", ast->data.text.value, len_to_type(ast->typeinfo));
            wasm_push_local(wasm, ast->data.text.value);
        }
    } else if (ast->type == AST_LABEL || ast->type == AST_FUNCDEF) {
        struct AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            wasm_scan_locals(wasm, current);
            current = current->next;
        }
    }
}
void wasm_eat_expr(Wasm *wasm, AST *ast) {
    if (ast->type == AST_INT){
        wasm_write_text(wasm, "\ti32.const %s\n", ast->data.text.value);
    }else if (ast->type == AST_STRING){
        wasm_write_text(wasm, "\ti32.const 1024\n");// Temporary placeholder
    };
}

void wasm_gen_instructions(Wasm *wasm, AST *ast) {
    if (!ast) return;
    if (ast->type == AST_CALL) {
        wasm_write_text(wasm, "\tcall $%s\n", ast->data.jmpif.label);
    } else if (ast->type == AST_LABEL) {
        wasm_write_text(wasm, "\tblock $lbl.%s\n", ast->data.text.value);
        struct AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            wasm_gen_instructions(wasm, current);
            current = current->next;
        }
        wasm_write_text(wasm, "\tend\n");
    }else if (ast->type == AST_JMP) {
       wasm_write_text(wasm, "\tbr $lbl%s\n", ast->data.jmpif.label);
    }
}


void wasm_eat_body(Wasm *wasm) {
    struct AST *ast_ptr = load_ast_wasm(wasm);
    if (!ast_ptr) return;

    if (ast_ptr->type == AST_FUNCDEF) {
        wasm->seen_count = 0;
        wasm_write_text(wasm, "(func $%s\n", ast_ptr->data.funcdef.name);
        
        wasm_scan_locals(wasm, ast_ptr);
        
        struct AST *current = ast_ptr->data.funcdef.block;
        while (current != NULL) {
            wasm_gen_instructions(wasm, current);
            current = current->next;
        }

        wasm_write_text(wasm, "\ti32.const 1\n\tglobal.set $a0\n");
        wasm_write_text(wasm, "\ti32.const 1024\n\tglobal.set $a1\n");
        wasm_write_text(wasm, "\ti32.const 6\n\tglobal.set $a2\n");
        wasm_write_text(wasm, "\tcall $syscall_write\n");
        wasm_write_text(wasm, ")\n");
    } else if (ast_ptr->type == AST_LABEL) {
        wasm_gen_instructions(wasm, ast_ptr);
    }

    wasm_peek(wasm);
}

Wasm *wasm_init(Reviser *reviser, char *output_file) {
    Wasm *wasm = malloc(sizeof(Wasm));
    wasm->name = output_file;
    wasm->asts = reviser->asts;
    wasm->astlen = reviser->astlen;
    wasm->cur = 0;
    wasm->global = reviser->global;
    wasm->seen_count = 0;
    wasm->_wat = "tmp.wat"; 
    FILE *f = fopen(wasm->_wat, "w");
    if (!f) { perror("fopen"); exit(1); }
    wasm->f = f;

    wasm_write_text(wasm, "(module\n");
    wasm_write_text(wasm, "(import \"env\" \"js_write\" (func $js_write (param i32 i32 i32)))\n");
    wasm_write_text(wasm, "(memory (export \"memory\") 1)\n");
    wasm_write_text(wasm, "(data (i32.const 1024) \"Hello\\n\")\n");

    for (int i = 0; i < 14; i++) {
        if (strcmp(regs[i], "__")) {
            wasm_write_text(wasm, "(global $%s (mut i32) (i32.const 0))\n", regs[i]);
            wasm_write_text(wasm, "(export \"%s\" (global $%s))\n", regs[i], regs[i]);
        }
    }

    wasm_write_text(wasm, "(func $syscall_write\n\tglobal.get $a0\n\tglobal.get $a1\n\tglobal.get $a2\n\tcall $js_write\n)\n");
    return wasm;
}

void wasm_close(Wasm *wasm) {
    wasm_write_text(wasm, "(export \"_start\" (func $main))\n)");
    fclose(wasm->f);
    char string[256];
    snprintf(string, sizeof(string), "bat %s", wasm->_wat);
    system(string);
    snprintf(string, sizeof(string), "cat %s | pbcopy", wasm->_wat);
    system(string);
    snprintf(string, sizeof(string), "wat2wasm %s -o %s", wasm->_wat, wasm->name);
    system(string);
}

char wasm_eat(Wasm *wasm) {
    if (wasm->cur >= wasm->astlen) return -1;
    wasm_eat_body(wasm);
    return 0;
}
