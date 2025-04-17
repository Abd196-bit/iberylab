#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    // Basic tokens
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_NUMBER_LITERAL,
    TOKEN_STRING_LITERAL,
    
    // Keywords
    TOKEN_FUNCTION,
    TOKEN_CLASS,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_INPUT,
    TOKEN_TEXT,
    TOKEN_NUM,
    
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LTE,
    TOKEN_GTE,
    
    // Delimiters
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_LANGLE,  // <
    TOKEN_RANGLE,  // >
    TOKEN_SLASH,   // /
    TOKEN_HASH,    // #
    
    // Special
    TOKEN_ERROR,
    
    // Game engine and animation tokens
    TOKEN_GAME_ENGINE,
    TOKEN_ANIMATE,
    TOKEN_FLY,
    TOKEN_DOWN,
    TOKEN_REPEAT,
    TOKEN_SPEED,
    TOKEN_PX
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    union {
        char* string_val;    // For identifiers and string literals
        double number_val;   // For numeric literals
    } value;
    int line;
    int column;
} Token;

typedef struct {
    const char* source;
    int position;
    int line;
    int column;
    Token current;
} Lexer;

// Function declarations
Lexer* create_lexer(const char* source);
Token get_next_token(Lexer* lexer);
void free_lexer(Lexer* lexer);

// Helper function declarations
static Token make_token(TokenType type, const char* value, Lexer* lexer);
static void advance(Lexer* lexer);
static char current_char(Lexer* lexer);
static char peek_char(Lexer* lexer);

#endif // LEXER_H 