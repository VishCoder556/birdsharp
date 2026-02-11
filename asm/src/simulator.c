static void *sim_internal_access(Simulator *sim, Location loc, int size) {
    uint8_t *base = NULL;
    int limit = 0;

    if (loc.type == LOC_STACK) {
        base = (uint8_t *)sim->stack;
        limit = 1024 * 64;
    } else if (loc.type == LOC_REG) {
        base = (uint8_t *)sim->regs;
        limit = 128;
    } else {
        exit(1);
    }

    if (loc.offset < 0 || (loc.offset + size) > limit) {
        exit(1);
    }

    return (void *)(base + loc.offset);
}

Simulator *simulator_init(Reviser *reviser, char *file) {
    Simulator *simulator = malloc(sizeof(Simulator));
    simulator->name = file;
    simulator->asts = reviser->asts;
    simulator->astlen = reviser->astlen;
    simulator->cur = 0;
    simulator->stack = calloc(1, 1024 * 64);
    simulator->regs = calloc(1, 128);
    simulator->sp = 0;
    return simulator;
}

AST *load_ast_simulator(Simulator *simulator) {
    AST *ast1 = simulator->asts;
    for (int i = 0; i < simulator->cur; i++) {
        if (ast1->next == NULL) break;
        ast1 = ast1->next;
    }
    return ast1;
}

uintptr_t simulator_eat_expr(Simulator *simulator, AST *ast) {
    if (!ast) return 0;

    switch (ast->type) {
        case AST_VAR: {
            Location l = { .type = LOC_STACK, .offset = ast->data.var.offset };
            void *ptr = sim_internal_access(simulator, l, ast->typeinfo);
            if (ast->typeinfo == 1) return (uintptr_t)(*(uint8_t*)ptr);
            if (ast->typeinfo == 2) return (uintptr_t)(*(uint16_t*)ptr);
            if (ast->typeinfo == 4) return (uintptr_t)(*(uint32_t*)ptr);
            return *(uintptr_t*)ptr;
        }
        case AST_INT:
            return (uintptr_t)atoll(ast->data.text.value);
        case AST_REG: {
            int reg_off = reg_to_num(ast->data.text.value).reg * 8;
            Location l = { .type = LOC_REG, .offset = reg_off };
            void *ptr = sim_internal_access(simulator, l, 8);
            return *(uintptr_t*)ptr;
        }
        default:
            return 0;
    }
}

Location simulator_eat_lhs(Simulator *simulator, AST *ast) {
    Location loc = { .type = LOC_UNKNOWN, .offset = 0 };
    if (!ast) return loc;

    switch (ast->type) {
        case AST_VAR:
            loc.type = LOC_STACK;
            loc.offset = ast->data.var.offset;
            break;
        case AST_REG:
            loc.type = LOC_REG;
            loc.offset = reg_to_num(ast->data.text.value).reg * 8;
            break;
        case AST_DEREF: {
            uintptr_t val = simulator_eat_expr(simulator, ast->data.expr.left);
            loc.type = LOC_STACK;
            loc.offset = (int)val;
            break;
        }
        default:
            break;
    }
    return loc;
}

void simulator_eat_ast(Simulator *simulator, AST *ast) {
    if (!ast) return;

    if (ast->type == AST_LOCAL) {
        return;
    } else if (ast->type == AST_MOV) {
        Location dest_loc = simulator_eat_lhs(simulator, ast->data.opexpr.left);
        void *dest_ptr = sim_internal_access(simulator, dest_loc, ast->typeinfo);
        uintptr_t val = simulator_eat_expr(simulator, ast->data.opexpr.right);

        if (ast->typeinfo == 1)      *(uint8_t*)dest_ptr = (uint8_t)val;
        else if (ast->typeinfo == 2) *(uint16_t*)dest_ptr = (uint16_t)val;
        else if (ast->typeinfo == 4) *(uint32_t*)dest_ptr = (uint32_t)val;
        else                         *(uintptr_t*)dest_ptr = val;
    }else if (ast->type == AST_LOCAL){
        AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            simulator_eat_ast(simulator, current);
            current = current->next;
        }
    }
}

void simulator_eat_body(Simulator *simulator) {
    AST *ast = load_ast_simulator(simulator);
    if (ast->type == AST_FUNCDEF) {
        int old_sp = simulator->sp;
        AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            simulator_eat_ast(simulator, current);
            current = current->next;
        }
        simulator->sp = old_sp;
    }else if (ast->type == AST_LOCAL){
        AST *current = ast->data.funcdef.block;
        while (current != NULL) {
            simulator_eat_ast(simulator, current);
            current = current->next;
        }
    }

    simulator->cur++;
}

char simulator_eat(Simulator *simulator) {
    if (simulator->cur >= simulator->astlen)
        return -1;
    simulator_eat_body(simulator);
    return 0;
}
