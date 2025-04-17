#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DEFINITION,
    NODE_TEXT,
    NODE_STRING_LITERAL,
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_INPUT,
    NODE_NUMBER_CONVERSION,
    NODE_GAME_ENGINE,
    NODE_EXPRESSION,
    NODE_ANIMATION,
    NODE_ANIMATION_LIST
} NodeType;

// AST node structures
typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            struct ASTNode** statements;
            int statement_count;
        } program;
        struct {
            char* name;
            struct ASTNode* body;
        } function_definition;
        struct {
            char* content;
            struct ASTNode* expr;
        } text;
        struct {
            char* value;
        } string_literal;
        struct {
            double value;
        } number;
        struct {
            char* name;
        } identifier;
        struct {
            char* prompt;
        } input;
        struct {
            struct ASTNode* expr;
        } number_conversion;
        struct {
            struct ASTNode** animations;
            int animation_count;
            struct ASTNode* expr;
        } game_engine;
        struct {
            char* emoji;
            char* action;
            int distance;
            int repeat;
            int speed;
        } animation;
    } data;
} ASTNode;

// Parser structure
typedef struct {
    Lexer* lexer;
    Token current;
    int had_error;
} Parser;

// Function declarations
Parser* create_parser(Lexer* lexer);
void free_parser(Parser* parser);
ASTNode* parse_program(Parser* parser);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int depth);

// Helper function declarations
static void parser_advance(Parser* parser);
static void expect(Parser* parser, TokenType type);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_animation(Parser* parser);

#endif // PARSER_H 