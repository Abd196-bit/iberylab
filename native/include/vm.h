#ifndef VM_H
#define VM_H

#include "parser.h"
#include <stdbool.h>
#include <stdint.h>

#define STACK_MAX 256

// Forward declarations
typedef struct Obj Obj;
typedef struct Table Table;
typedef struct ValueArray ValueArray;

// Table structure
typedef struct {
    int count;
    int capacity;
    char** keys;
    Value* values;
} Table;

// Value array structure
typedef struct ValueArray {
    int capacity;
    int count;
    Value* values;
} ValueArray;

// Animation structure
typedef struct {
    char* emoji;
    char* action;
    int distance;
    int repeat;
    int speed;
} Animation;

// Value types
typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOLEAN,
    VAL_NULL,
    VAL_FUNCTION,
    VAL_CLASS,
    VAL_INSTANCE,
    VAL_LIST,
    VAL_MAP,
    VAL_COMMAND,
    VAL_INPUT,
    VAL_ANIMATION,
    VAL_OBJECT
} ValueType;

// Value representation
typedef struct {
    ValueType type;
    union {
        double number;
        char* string;
        bool boolean;
        void* object;
        void* function;
        struct {
            char* cmd;
            char** args;
            int arg_count;
        } command;
        Animation animation;
    } as;
} Value;

// Bytecode instructions
typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_RETURN
} OpCode;

// Code fix types
typedef enum {
    FIX_MISSING_SEMICOLON,
    FIX_MISSING_BRACE,
    FIX_MISSING_PAREN,
    FIX_UNDEFINED_VARIABLE,
    FIX_TYPE_MISMATCH,
    FIX_UNUSED_VARIABLE,
    FIX_DUPLICATE_DEFINITION,
    FIX_INVALID_OPERATOR,
    FIX_MISSING_RETURN,
    FIX_INVALID_SYNTAX
} FixType;

// Code fix structure
typedef struct {
    FixType type;
    int line;
    int column;
    char* message;
    char* fix;
} CodeFix;

// Interpretation result
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// Chunk of bytecode
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

// Virtual Machine
typedef struct {
    // Memory
    void** heap;
    int heap_size;
    int heap_capacity;
    
    // Symbol table for variables
    struct {
        char** names;
        void** values;
        int count;
        int capacity;
    } symbols;

    // Terminal state
    struct {
        char* current_dir;
        bool is_running;
        int exit_code;
    } terminal;

    // Code fixer state
    struct {
        CodeFix* fixes;
        int fix_count;
        int fix_capacity;
    } fixer;

    // Runtime state
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    Obj* objects;
} VM;

// Function declarations
VM* create_vm();
void free_vm(VM* vm);
int execute_program(VM* vm, ASTNode* program);
Value evaluate_expression(VM* vm, ASTNode* expr);
void execute_statement(VM* vm, ASTNode* stmt);

// Memory management
void* vm_alloc(VM* vm, size_t size);
void vm_free(VM* vm, void* ptr);

// Stack operations
void push(VM* vm, Value value);
Value pop(VM* vm);
Value peek(VM* vm, int offset);

// Symbol table operations
void define_symbol(VM* vm, const char* name, Value value);
Value get_symbol(VM* vm, const char* name);
bool has_symbol(VM* vm, const char* name);

// Terminal operations
void init_terminal(VM* vm);
void execute_terminal_command(VM* vm, const char* cmd);
void change_directory(VM* vm, const char* path);
void list_directory(VM* vm);
void print_working_directory(VM* vm);
void execute_system_command(VM* vm, const char* cmd);

// Input/output operations
Value execute_input_command(VM* vm, const char* prompt);
Value parse_number(const char* str);
Value convert_to_number(Value value);

// Game engine operations
void init_game_engine(VM* vm);
void execute_animation(VM* vm, Value animation);
void render_animation(VM* vm, Value animation);

// VM lifecycle
void initVM(VM* vm);
void freeVM(VM* vm);
InterpretResult interpret(VM* vm, const char* source);
void compile(VM* vm, const char* source);

// Code fixer operations
void init_fixer(VM* vm);
void free_fixer(VM* vm);
void analyze_code(VM* vm, const char* source);
void apply_fixes(VM* vm, const char* source, char** fixed_source);
void print_fixes(VM* vm);

// Table operations
void initTable(Table* table);
void freeTable(Table* table);
bool tableSet(Table* table, const char* key, Value value);
bool tableGet(Table* table, const char* key, Value* value);
bool tableDelete(Table* table, const char* key);

// Object operations
void freeObjects(VM* vm);

#endif // VM_H 