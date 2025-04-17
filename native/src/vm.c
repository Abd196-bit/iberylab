#include "vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define INITIAL_HEAP_SIZE 1024
#define INITIAL_STACK_SIZE 256
#define INITIAL_SYMBOL_TABLE_SIZE 64

VM* create_vm() {
    VM* vm = (VM*)malloc(sizeof(VM));
    if (!vm) return NULL;
    
    // Initialize heap
    vm->heap = (void**)malloc(INITIAL_HEAP_SIZE * sizeof(void*));
    vm->heap_size = 0;
    vm->heap_capacity = INITIAL_HEAP_SIZE;
    
    // Initialize stack
    vm->stack = (void**)malloc(INITIAL_STACK_SIZE * sizeof(void*));
    vm->stack_size = 0;
    vm->stack_capacity = INITIAL_STACK_SIZE;
    
    // Initialize registers
    vm->pc = NULL;
    vm->sp = vm->stack;
    vm->fp = vm->stack;
    
    // Initialize symbol table
    vm->symbols.names = (char**)malloc(INITIAL_SYMBOL_TABLE_SIZE * sizeof(char*));
    vm->symbols.values = (void**)malloc(INITIAL_SYMBOL_TABLE_SIZE * sizeof(void*));
    vm->symbols.count = 0;
    vm->symbols.capacity = INITIAL_SYMBOL_TABLE_SIZE;
    
    // Initialize terminal
    init_terminal(vm);
    
    return vm;
}

void free_vm(VM* vm) {
    if (!vm) return;
    
    // Free heap
    for (int i = 0; i < vm->heap_size; i++) {
        free(vm->heap[i]);
    }
    free(vm->heap);
    
    // Free stack
    free(vm->stack);
    
    // Free symbol table
    for (int i = 0; i < vm->symbols.count; i++) {
        free(vm->symbols.names[i]);
        free(vm->symbols.values[i]);
    }
    free(vm->symbols.names);
    free(vm->symbols.values);
    
    // Clean up terminal
    free(vm->terminal.current_dir);
    
    free(vm);
}

void* vm_alloc(VM* vm, size_t size) {
    if (vm->heap_size >= vm->heap_capacity) {
        vm->heap_capacity *= 2;
        vm->heap = (void**)realloc(vm->heap, vm->heap_capacity * sizeof(void*));
    }
    
    void* ptr = malloc(size);
    vm->heap[vm->heap_size++] = ptr;
    return ptr;
}

void vm_free(VM* vm, void* ptr) {
    for (int i = 0; i < vm->heap_size; i++) {
        if (vm->heap[i] == ptr) {
            free(ptr);
            vm->heap[i] = NULL;
            return;
        }
    }
}

void push(VM* vm, Value value) {
    if (vm->stack_size >= vm->stack_capacity) {
        vm->stack_capacity *= 2;
        vm->stack = (void**)realloc(vm->stack, vm->stack_capacity * sizeof(void*));
    }
    vm->stack[vm->stack_size++] = (void*)value.as.object;
}

Value pop(VM* vm) {
    if (vm->stack_size == 0) {
        Value null = {VAL_NULL, {.object = NULL}};
        return null;
    }
    Value value = {VAL_OBJECT, {.object = vm->stack[--vm->stack_size]}};
    return value;
}

Value peek(VM* vm, int offset) {
    if (offset >= vm->stack_size) {
        Value null = {VAL_NULL, {.object = NULL}};
        return null;
    }
    Value value = {VAL_OBJECT, {.object = vm->stack[vm->stack_size - 1 - offset]}};
    return value;
}

void define_symbol(VM* vm, const char* name, Value value) {
    if (vm->symbols.count >= vm->symbols.capacity) {
        vm->symbols.capacity *= 2;
        vm->symbols.names = (char**)realloc(vm->symbols.names, vm->symbols.capacity * sizeof(char*));
        vm->symbols.values = (void**)realloc(vm->symbols.values, vm->symbols.capacity * sizeof(void*));
    }
    
    vm->symbols.names[vm->symbols.count] = strdup(name);
    vm->symbols.values[vm->symbols.count] = value.as.object;
    vm->symbols.count++;
}

Value get_symbol(VM* vm, const char* name) {
    for (int i = 0; i < vm->symbols.count; i++) {
        if (strcmp(vm->symbols.names[i], name) == 0) {
            Value value = {VAL_OBJECT, {.object = vm->symbols.values[i]}};
            return value;
        }
    }
    Value null = {VAL_NULL, {.object = NULL}};
    return null;
}

bool has_symbol(VM* vm, const char* name) {
    for (int i = 0; i < vm->symbols.count; i++) {
        if (strcmp(vm->symbols.names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

void init_terminal(VM* vm) {
    vm->terminal.current_dir = getcwd(NULL, 0);
    vm->terminal.is_running = true;
    vm->terminal.exit_code = 0;
}

void execute_terminal_command(VM* vm, const char* cmd) {
    if (strncmp(cmd, "cd ", 3) == 0) {
        change_directory(vm, cmd + 3);
    } else if (strcmp(cmd, "ls") == 0) {
        list_directory(vm);
    } else if (strcmp(cmd, "pwd") == 0) {
        print_working_directory(vm);
    } else {
        execute_system_command(vm, cmd);
    }
}

void change_directory(VM* vm, const char* path) {
    if (chdir(path) == 0) {
        free(vm->terminal.current_dir);
        vm->terminal.current_dir = getcwd(NULL, 0);
    } else {
        printf("Error: Could not change directory to %s\n", path);
    }
}

void list_directory(VM* vm) {
    DIR* dir = opendir(vm->terminal.current_dir);
    if (!dir) {
        printf("Error: Could not open directory\n");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {  // Skip hidden files
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);
}

void print_working_directory(VM* vm) {
    printf("%s\n", vm->terminal.current_dir);
}

void execute_system_command(VM* vm, const char* cmd) {
    int result = system(cmd);
    if (result != 0) {
        printf("Command failed with exit code %d\n", result);
    }
}

Value execute_input_command(VM* vm, const char* prompt) {
    char input[256];
    printf("%s", prompt);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0; // Remove newline
    
    Value result;
    result.type = VAL_STRING;
    result.as.string = strdup(input);
    return result;
}

Value parse_number(const char* str) {
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = atof(str);
    return result;
}

Value convert_to_number(Value value) {
    if (value.type == VAL_NUMBER) {
        return value;
    } else if (value.type == VAL_STRING) {
        return parse_number(value.as.string);
    }
    
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = 0;
    return result;
}

void init_game_engine(VM* vm) {
    // Initialize any game engine state here
    printf("\033[2J\033[H"); // Clear screen
}

void execute_animation(VM* vm, Value animation) {
    if (animation.type != VAL_ANIMATION) return;
    
    Animation anim = animation.as.animation;
    for (int i = 0; i < anim.repeat; i++) {
        // Move cursor to start position
        printf("\033[%d;%dH", 10, 10);
        
        // Print emoji
        printf("%s", anim.emoji);
        fflush(stdout);
        
        // Animate
        for (int j = 0; j < anim.distance; j++) {
            printf("\033[%dC", 1); // Move right
            fflush(stdout);
            usleep(1000000 / anim.speed); // Delay based on speed
        }
        
        // Clear line
        printf("\033[K");
    }
}

void render_animation(VM* vm, Value animation) {
    execute_animation(vm, animation);
}

Value evaluate_expression(VM* vm, ASTNode* node) {
    if (!node) {
        Value null = {VAL_NULL, {.object = NULL}};
        return null;
    }

    switch (node->type) {
        case NODE_NUMBER: {
            Value value = {VAL_NUMBER, {.number = node->data.number.value}};
            return value;
        }
        case NODE_STRING_LITERAL: {
            Value value = {VAL_STRING, {.string = strdup(node->data.string_literal.value)}};
            return value;
        }
        case NODE_IDENTIFIER: {
            return get_symbol(vm, node->data.identifier.name);
        }
        case NODE_TEXT: {
            Value value = {VAL_STRING, {.string = strdup(node->data.text.content)}};
            return value;
        }
        case NODE_INPUT: {
            return execute_input_command(vm, node->data.input.prompt);
        }
        case NODE_NUMBER_CONVERSION: {
            Value input = evaluate_expression(vm, node->data.number_conversion.expr);
            return convert_to_number(input);
        }
        case NODE_ANIMATION: {
            Value anim;
            anim.type = VAL_ANIMATION;
            anim.as.animation.emoji = strdup(node->data.animation.emoji);
            anim.as.animation.action = strdup(node->data.animation.action);
            anim.as.animation.distance = node->data.animation.distance;
            anim.as.animation.repeat = node->data.animation.repeat;
            anim.as.animation.speed = node->data.animation.speed;
            return anim;
        }
        default: {
            fprintf(stderr, "Unknown expression type: %d\n", node->type);
            Value null = {VAL_NULL, {.object = NULL}};
            return null;
        }
    }
}

void execute_statement(VM* vm, ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM: {
            for (int i = 0; i < node->data.program.statement_count; i++) {
                execute_statement(vm, node->data.program.statements[i]);
            }
            break;
        }
        case NODE_FUNCTION_DEFINITION: {
            // Store function definition
            Value func = {VAL_FUNCTION, {.function = node}};
            define_symbol(vm, node->data.function_definition.name, func);
            break;
        }
        case NODE_TEXT: {
            Value text = evaluate_expression(vm, node->data.text.expr);
            if (text.type == VAL_STRING) {
                printf("%s\n", text.as.string);
            }
            break;
        }
        case NODE_IDENTIFIER: {
            // Look up function and execute it
            Value func = get_symbol(vm, node->data.identifier.name);
            if (func.type == VAL_FUNCTION) {
                ASTNode* func_def = (ASTNode*)func.as.function;
                execute_statement(vm, func_def->data.function_definition.body);
            }
            break;
        }
        case NODE_GAME_ENGINE: {
            init_game_engine(vm);
            for (int i = 0; i < node->data.game_engine.animation_count; i++) {
                Value anim = evaluate_expression(vm, node->data.game_engine.animations[i]);
                render_animation(vm, anim);
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown statement type: %d\n", node->type);
            break;
    }
}

int execute_program(VM* vm, ASTNode* program) {
    if (!program || program->type != NODE_PROGRAM) {
        fprintf(stderr, "Invalid program node\n");
        return 1;
    }

    for (int i = 0; i < program->data.program.statement_count; i++) {
        execute_statement(vm, program->data.program.statements[i]);
    }
    
    return 0;
}

void initVM(VM* vm) {
    vm->stackTop = vm->stack;
    vm->globals = NULL;
    vm->strings = NULL;
    vm->objects = NULL;
}

void freeVM(VM* vm) {
    freeTable(&vm->globals);
    freeTable(&vm->strings);
    freeObjects(vm);
}

static InterpretResult run(VM* vm) {
    #define READ_BYTE() (*vm->ip++)
    #define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while (false)

    for (;;) {
        #ifdef DEBUG_TRACE_EXECUTION
            printf("          ");
            for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");
            disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NULL: push(NULL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm->stack[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm->stack[slot] = peek(0);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm->globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm->globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm->globals, name, peek(0))) {
                    tableDelete(&vm->globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD:      BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) vm->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}

InterpretResult interpret(VM* vm, const char* source) {
    compile(vm, source);
    if (vm->parser->hadError) return INTERPRET_COMPILE_ERROR;

    vm->ip = vm->chunk->code;
    return run(vm);
}

void init_fixer(VM* vm) {
    vm->fixer.fixes = NULL;
    vm->fixer.fix_count = 0;
    vm->fixer.fix_capacity = 0;
}

void free_fixer(VM* vm) {
    for (int i = 0; i < vm->fixer.fix_count; i++) {
        free(vm->fixer.fixes[i].message);
        free(vm->fixer.fixes[i].fix);
    }
    free(vm->fixer.fixes);
}

void add_fix(VM* vm, FixType type, int line, int column, const char* message, const char* fix) {
    if (vm->fixer.fix_count >= vm->fixer.fix_capacity) {
        int new_capacity = vm->fixer.fix_capacity < 8 ? 8 : vm->fixer.fix_capacity * 2;
        vm->fixer.fixes = realloc(vm->fixer.fixes, sizeof(CodeFix) * new_capacity);
        vm->fixer.fix_capacity = new_capacity;
    }

    CodeFix* code_fix = &vm->fixer.fixes[vm->fixer.fix_count++];
    code_fix->type = type;
    code_fix->line = line;
    code_fix->column = column;
    code_fix->message = strdup(message);
    code_fix->fix = strdup(fix);
}

void analyze_code(VM* vm, const char* source) {
    // Reset fixer state
    free_fixer(vm);
    init_fixer(vm);

    // Split source into lines
    char* source_copy = strdup(source);
    char* line = strtok(source_copy, "\n");
    int line_number = 1;

    while (line != NULL) {
        // Check for missing semicolons
        if (strlen(line) > 0 && line[strlen(line) - 1] != ';' && 
            strstr(line, "if") == NULL && strstr(line, "while") == NULL) {
            add_fix(vm, FIX_MISSING_SEMICOLON, line_number, strlen(line),
                   "Missing semicolon at end of line",
                   strcat(strdup(line), ";"));
        }

        // Check for missing braces
        if (strstr(line, "if") != NULL && strstr(line, "{") == NULL) {
            add_fix(vm, FIX_MISSING_BRACE, line_number, strlen(line),
                   "Missing opening brace after if statement",
                   strcat(strdup(line), " {"));
        }

        // Check for undefined variables
        char* var = strtok(line, " =;");
        while (var != NULL) {
            if (isalpha(var[0]) && !has_symbol(vm, var)) {
                add_fix(vm, FIX_UNDEFINED_VARIABLE, line_number, var - line,
                       "Undefined variable used",
                       strcat(strdup("var "), var));
            }
            var = strtok(NULL, " =;");
        }

        line = strtok(NULL, "\n");
        line_number++;
    }

    free(source_copy);
}

void apply_fixes(VM* vm, const char* source, char** fixed_source) {
    // Create a copy of the source
    char* result = strdup(source);
    int offset = 0;

    // Apply fixes in reverse order to maintain line numbers
    for (int i = vm->fixer.fix_count - 1; i >= 0; i--) {
        CodeFix* fix = &vm->fixer.fixes[i];
        
        // Calculate position in source
        int pos = 0;
        int line = 1;
        while (line < fix->line && result[pos] != '\0') {
            if (result[pos] == '\n') line++;
            pos++;
        }
        pos += fix->column;

        // Apply fix
        int old_len = strlen(result);
        int fix_len = strlen(fix->fix);
        result = realloc(result, old_len + fix_len + 1);
        memmove(result + pos + fix_len, result + pos, old_len - pos + 1);
        memcpy(result + pos, fix->fix, fix_len);
    }

    *fixed_source = result;
}

void print_fixes(VM* vm) {
    printf("\nFound %d issues:\n", vm->fixer.fix_count);
    for (int i = 0; i < vm->fixer.fix_count; i++) {
        CodeFix* fix = &vm->fixer.fixes[i];
        printf("Line %d, Column %d: %s\n", fix->line, fix->column, fix->message);
        printf("Fix: %s\n\n", fix->fix);
    }
}

void compile(VM* vm, const char* source) {
    // First analyze the code
    analyze_code(vm, source);

    // If there are fixes, apply them
    if (vm->fixer.fix_count > 0) {
        printf("Found issues in your code. Applying fixes...\n");
        print_fixes(vm);
        
        char* fixed_source;
        apply_fixes(vm, source, &fixed_source);
        
        // Compile the fixed source
        Parser parser;
        initParser(&parser, fixed_source);
        Compiler compiler;
        initCompiler(&compiler, &parser);

        parser.hadError = false;
        parser.panicMode = false;

        while (!match(&parser, TOKEN_EOF)) {
            declaration(&compiler);
        }

        endCompiler(&compiler);
        free(fixed_source);
        return !parser.hadError;
    }

    // If no fixes needed, compile normally
    Parser parser;
    initParser(&parser, source);
    Compiler compiler;
    initCompiler(&compiler, &parser);

    parser.hadError = false;
    parser.panicMode = false;

    while (!match(&parser, TOKEN_EOF)) {
        declaration(&compiler);
    }

    endCompiler(&compiler);
    return !parser.hadError;
} 