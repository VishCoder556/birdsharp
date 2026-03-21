typedef struct {
    void (*init)(void *generator);
    void (*generate_ast)(void *generator, AST ast);
    void (*close)(void *generator);
}Generator_Functions;

typedef struct {
    FILE *file;
    char filename[100];
}Generator_Output;

typedef struct {
    char *name;
    AST *asts;
    int astlen;
    int cur;
    AST *curr;

    Generator_Functions functions;
    Generator_Output *output;
}Generator;

Generator *_generator_init(Typechecker *typechecker, char *file, Generator_Functions functions){
    Generator *generator = malloc(sizeof(Generator));
    generator->name = typechecker->name;
    generator->cur = 0;
    generator->asts = typechecker->asts;
    generator->curr = generator->asts;
    generator->astlen = typechecker->astlen;
    generator->functions = functions;
    generator->output = malloc(sizeof(Generator_Output));
    strcpy(generator->output->filename, file);
    generator->functions.init(generator);
    return generator;
};

int generator_eat_ast(Generator *generator){
    if (generator->astlen == generator->cur){
        generator->functions.close(generator);
        return -1;
    };
    AST *cur = generator->curr;
    if (!cur){
        generator->functions.close(generator);
        return -1;
    }
    generator->functions.generate_ast(generator, *cur);
    generator->curr = generator->curr->next;
    generator->cur++;
    return 0;
};

void _generator_new(Generator *generator, Generator_Functions functions){
    generator->functions = functions;
};

#define generator_new(generator, prefix) _generator_new(generator, (Generator_Functions){.init=prefix##_init, .generate_ast=prefix##_generate_ast, .close=prefix##_close})

void generator_write_text(Generator *generator, char *text){
    fwrite(text, sizeof(char), strlen(text), generator->output->file);
}

void generator_write_char(Generator *generator, char c){
    fwrite(&c, sizeof(char), 1, generator->output->file);
}

void generator_write_binary(Generator *generator, int *data, int count){
    fwrite(data, sizeof(int), count, generator->output->file);
}

void generator_open_text(Generator *generator){
    generator->output->file = fopen(generator->output->filename, "w");
    if (generator->output->file == NULL){
        error_generate("FileError", "Errors when opening file");
    }
}

void generator_open_binary(Generator *generator){
    generator->output->file = fopen(generator->output->filename, "wb");
    if (generator->output->file == NULL){
        error_generate("FileError", "Errors when opening file");
    }
}

void generator_open_text_file(Generator *generator, char *file){
    generator->output->file = fopen(file, "w");
    if (generator->output->file == NULL){
        error_generate("FileError", "Errors when opening file");
    }
}

void generator_open_binary_file(Generator *generator, char *file){
    generator->output->file = fopen(file, "wb");
    if (generator->output->file == NULL){
        error_generate("FileError", "Errors when opening file");
    }
}

void generator_close(Generator *generator){
    fclose(generator->output->file);
}

Generator *_generator_make(Typechecker *typechecker, Generator_Functions functions){
    char *_ir = find_ir();
    Generator *generator = _generator_init(typechecker, _ir, functions);
    while (generator_eat_ast(generator) != -1){};
    return generator;
};

#define generator_make(typechecker, prefix) _generator_make(typechecker, (Generator_Functions){.init=prefix##_init, .generate_ast=prefix##_generate_ast, .close=prefix##_close})
#define generator_clean(generator) remove(generator->output->filename)
