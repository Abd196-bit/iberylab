#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENTIFIER_LENGTH 256
#define MAX_LINE_LENGTH 1024

// Forward declarations of helper functions
static Token make_token(TokenType type, const char* value, Lexer* lexer);
static Token identifier_or_keyword(Lexer* lexer);
static Token number(Lexer* lexer);
static Token string(Lexer* lexer);

typedef struct {
    const char* keyword;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    {"function", TOKEN_FUNCTION},
    {"class", TOKEN_CLASS},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"input", TOKEN_INPUT},
    {"text", TOKEN_TEXT},
    {"num", TOKEN_NUM}
};

static struct {
    char op;
    TokenType type;
} operators[] = {
    {'+', TOKEN_PLUS},
    {'-', TOKEN_MINUS},
    {'*', TOKEN_MULTIPLY},
    {'/', TOKEN_DIVIDE},
    {'=', TOKEN_ASSIGN},
    {'<', TOKEN_LT},
    {'>', TOKEN_GT},
    {';', TOKEN_SEMICOLON},
    {',', TOKEN_COMMA},
    {'.', TOKEN_DOT},
    {'(', TOKEN_LPAREN},
    {')', TOKEN_RPAREN},
    {'{', TOKEN_LBRACE},
    {'}', TOKEN_RBRACE}
};

static TokenType check_keyword(const char* identifier) {
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(identifier, keywords[i].keyword) == 0) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}

// Helper function to create a token
static Token make_token(TokenType type, const char* value, Lexer* lexer) {
    Token token;
    token.type = type;
    if (value) {
        token.value.string_val = strdup(value);
    }
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

// Helper function to advance the lexer
static void advance(Lexer* lexer) {
    if (lexer->position < strlen(lexer->source)) {
        lexer->position++;
        lexer->column++;
    }
}

// Helper function to get current character
static char current_char(Lexer* lexer) {
    if (lexer->position >= strlen(lexer->source)) {
        return '\0';
    }
    return lexer->source[lexer->position];
}

// Helper function to peek next character
static char peek_char(Lexer* lexer) {
    if (lexer->position + 1 >= strlen(lexer->source)) {
        return '\0';
    }
    return lexer->source[lexer->position + 1];
}

// Create a new lexer
Lexer* create_lexer(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

// Free lexer memory
void free_lexer(Lexer* lexer) {
    free(lexer);
}

// Get next token from source
Token get_next_token(Lexer* lexer) {
    while (isspace(current_char(lexer))) {
        advance(lexer);
    }
    
    // Check for EOF
    if (current_char(lexer) == '\0') {
        return make_token(TOKEN_EOF, "", lexer);
    }
    
    // Identifiers and keywords
    if (isalpha(current_char(lexer))) {
        return identifier_or_keyword(lexer);
    }
    
    // Numbers
    if (isdigit(current_char(lexer))) {
        return number(lexer);
    }
    
    // Strings
    if (current_char(lexer) == '"') {
        return string(lexer);
    }
    
    // Single-character tokens
    switch (current_char(lexer)) {
        case '{': advance(lexer); return make_token(TOKEN_LBRACE, "{", lexer);
        case '}': advance(lexer); return make_token(TOKEN_RBRACE, "}", lexer);
        case '+': advance(lexer); return make_token(TOKEN_PLUS, "+", lexer);
        case '-': advance(lexer); return make_token(TOKEN_MINUS, "-", lexer);
        case '*': advance(lexer); return make_token(TOKEN_MULTIPLY, "*", lexer);
        case '/': advance(lexer); return make_token(TOKEN_DIVIDE, "/", lexer);
        case '(': advance(lexer); return make_token(TOKEN_LPAREN, "(", lexer);
        case ')': advance(lexer); return make_token(TOKEN_RPAREN, ")", lexer);
        case ';': advance(lexer); return make_token(TOKEN_SEMICOLON, ";", lexer);
        case '.': advance(lexer); return make_token(TOKEN_DOT, ".", lexer);
        case ',': advance(lexer); return make_token(TOKEN_COMMA, ",", lexer);
    }
    
    // If we get here, we have an invalid character
    char invalid[2] = {current_char(lexer), '\0'};
    advance(lexer);
    return make_token(TOKEN_ERROR, invalid, lexer);
}

static Token identifier_or_keyword(Lexer* lexer) {
    char buffer[256];
    int i = 0;
    
    while (isalnum(current_char(lexer)) || current_char(lexer) == '_') {
        buffer[i++] = current_char(lexer);
        advance(lexer);
    }
    buffer[i] = '\0';
    
    // Check for keywords
    if (strcmp(buffer, "function") == 0) return make_token(TOKEN_FUNCTION, buffer, lexer);
    if (strcmp(buffer, "class") == 0) return make_token(TOKEN_CLASS, buffer, lexer);
    if (strcmp(buffer, "if") == 0) return make_token(TOKEN_IF, buffer, lexer);
    if (strcmp(buffer, "else") == 0) return make_token(TOKEN_ELSE, buffer, lexer);
    if (strcmp(buffer, "while") == 0) return make_token(TOKEN_WHILE, buffer, lexer);
    if (strcmp(buffer, "return") == 0) return make_token(TOKEN_RETURN, buffer, lexer);
    if (strcmp(buffer, "true") == 0) return make_token(TOKEN_TRUE, buffer, lexer);
    if (strcmp(buffer, "false") == 0) return make_token(TOKEN_FALSE, buffer, lexer);
    
    return make_token(TOKEN_IDENTIFIER, buffer, lexer);
}

static Token number(Lexer* lexer) {
    char buffer[256];
    int i = 0;
    
    while (isdigit(current_char(lexer)) || current_char(lexer) == '.') {
        buffer[i++] = current_char(lexer);
        advance(lexer);
    }
    buffer[i] = '\0';
    
    return make_token(TOKEN_NUMBER, buffer, lexer);
}

static Token string(Lexer* lexer) {
    char buffer[256];
    int i = 0;
    
    advance(lexer); // Skip opening quote
    while (current_char(lexer) != '"' && current_char(lexer) != '\0') {
        buffer[i++] = current_char(lexer);
        advance(lexer);
    }
    buffer[i] = '\0';
    
    if (current_char(lexer) == '"') {
        advance(lexer); // Skip closing quote
        return make_token(TOKEN_STRING, buffer, lexer);
    }
    
    return make_token(TOKEN_ERROR, "Unterminated string", lexer);
} 